/*
 * progress.c - report how much of each region's ARM9 code is matched.
 *
 * Usage (from the repository root):
 *   progress            report JP, US and EU
 *   progress j u e      report the listed regions
 *
 * Reads config/<r>/arm9/delinks.txt (units marked "complete" count as
 * matched) and config/<r>/arm9/symbols.txt (for function counts).
 *
 * Build: clang -O2 -o build/progress.exe tools/progress.c
 */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ARM9 .text layout per region, found from link-order call layering. */
typedef struct Area {
    const char *name;
    unsigned long start;
    unsigned long end;
} Area;

#define NUM_AREAS 6

typedef struct Region {
    const char *code;   /* j, u, e */
    const char *label;
    Area areas[NUM_AREAS];
} Region;

static const Region REGIONS[] = {
    { "j", "JP (ABHJ)", {
        { "NitroSDK startup", 0x02000000, 0x02000B88 },
        { "Game code",        0x02000B88, 0x020D96BC },
        { "CRI ADX/AHX",      0x020D96BC, 0x020F20C8 },
        { "NitroSystem",      0x020F20C8, 0x020FD5C8 },
        { "NitroSDK",         0x020FD5C8, 0x0210FBF4 },
        { "MSL / runtime",    0x0210FBF4, 0x02118918 } } },
    { "u", "US (ABHE)", {
        { "NitroSDK startup", 0x02000000, 0x02000B88 },
        { "Game code",        0x02000B88, 0x020D9C80 },
        { "CRI ADX/AHX",      0x020D9C80, 0x020F268C },
        { "NitroSystem",      0x020F268C, 0x020FDB8C },
        { "NitroSDK",         0x020FDB8C, 0x021101B8 },
        { "MSL / runtime",    0x021101B8, 0x02118EDC } } },
    { "e", "EU (ABHP)", {
        { "NitroSDK startup", 0x02000000, 0x02000B88 },
        { "Game code",        0x02000B88, 0x020D9FE8 },
        { "CRI ADX/AHX",      0x020D9FE8, 0x020F29F4 },
        { "NitroSystem",      0x020F29F4, 0x020FDEF4 },
        { "NitroSDK",         0x020FDEF4, 0x02110520 },
        { "MSL / runtime",    0x02110520, 0x02119244 } } },
};

#define MAX_RANGES 4096

typedef struct Range {
    unsigned long start;
    unsigned long end;
} Range;

static Range matched[MAX_RANGES];
static int num_matched;

/* Collect .text ranges of units marked "complete". */
static int load_matched(const char *path)
{
    FILE *f = fopen(path, "r");
    char line[1024];
    Range pending[64];
    int num_pending = 0, in_unit = 0, complete = 0, i;

    if (f == NULL) {
        perror(path);
        return 0;
    }
    num_matched = 0;
    for (;;) {
        char *got = fgets(line, sizeof(line), f);
        int header = got != NULL && line[0] != ' ' && line[0] != '\t' &&
                     line[0] != '\n' && line[0] != '\r';
        /* flush the previous unit at a new header or end of file */
        if (got == NULL || header) {
            if (in_unit && complete) {
                for (i = 0; i < num_pending && num_matched < MAX_RANGES; i++) {
                    matched[num_matched++] = pending[i];
                }
            }
            num_pending = 0;
            complete = 0;
            in_unit = 0;
            if (got == NULL) break;
            in_unit = strchr(line, ':') != NULL && strstr(line, ".c:") != NULL;
            continue;
        }
        if (!in_unit) continue;
        if (strstr(line, "complete") != NULL) {
            complete = 1;
        } else if (strstr(line, ".text") != NULL && num_pending < 64) {
            char *s = strstr(line, "start:");
            char *e = strstr(line, "end:");
            if (s != NULL && e != NULL) {
                pending[num_pending].start = strtoul(s + 6, NULL, 16);
                pending[num_pending].end = strtoul(e + 4, NULL, 16);
                num_pending++;
            }
        }
    }
    fclose(f);
    return 1;
}

static int is_matched(unsigned long addr)
{
    int i;
    for (i = 0; i < num_matched; i++) {
        if (addr >= matched[i].start && addr < matched[i].end) return 1;
    }
    return 0;
}

static unsigned long overlap(unsigned long a0, unsigned long a1, unsigned long b0, unsigned long b1)
{
    unsigned long lo = a0 > b0 ? a0 : b0;
    unsigned long hi = a1 < b1 ? a1 : b1;
    return hi > lo ? hi - lo : 0;
}

static int report(const Region *r)
{
    char path[256];
    unsigned long fn_total[NUM_AREAS] = {0}, fn_done[NUM_AREAS] = {0};
    unsigned long all_bytes = 0, all_done = 0;
    FILE *f;
    char line[1024];
    int a, i;

    snprintf(path, sizeof(path), "config/%s/arm9/delinks.txt", r->code);
    if (!load_matched(path)) return 0;

    snprintf(path, sizeof(path), "config/%s/arm9/symbols.txt", r->code);
    f = fopen(path, "r");
    if (f == NULL) {
        perror(path);
        return 0;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        char *k = strstr(line, " kind:function");
        char *ad = strstr(line, " addr:0x");
        unsigned long addr;
        if (k == NULL || ad == NULL) continue;
        addr = strtoul(ad + 8, NULL, 16);
        for (a = 0; a < NUM_AREAS; a++) {
            if (addr >= r->areas[a].start && addr < r->areas[a].end) {
                fn_total[a]++;
                if (is_matched(addr)) fn_done[a]++;
                break;
            }
        }
    }
    fclose(f);

    printf("%s\n", r->label);
    printf("  %-18s %20s %9s %16s\n", "area", "matched bytes", "", "functions");
    for (a = 0; a < NUM_AREAS; a++) {
        const Area *ar = &r->areas[a];
        unsigned long size = ar->end - ar->start, done = 0;
        for (i = 0; i < num_matched; i++) {
            done += overlap(ar->start, ar->end, matched[i].start, matched[i].end);
        }
        all_bytes += size;
        all_done += done;
        printf("  %-18s %8lu / %8lu %8.3f%% %7lu / %5lu\n", ar->name, done, size,
               100.0 * (double)done / (double)size, fn_done[a], fn_total[a]);
    }
    printf("  %-18s %8lu / %8lu %8.3f%%\n\n", "all ARM9 code", all_done, all_bytes,
           100.0 * (double)all_done / (double)all_bytes);
    return 1;
}

int main(int argc, char **argv)
{
    int i, j, ok = 1;
    const int n = (int)(sizeof(REGIONS) / sizeof(REGIONS[0]));

    if (argc < 2) {
        for (j = 0; j < n; j++) ok &= report(&REGIONS[j]);
        return ok ? 0 : 1;
    }
    for (i = 1; i < argc; i++) {
        int found = 0;
        for (j = 0; j < n; j++) {
            if (strcmp(argv[i], REGIONS[j].code) == 0) {
                ok &= report(&REGIONS[j]);
                found = 1;
            }
        }
        if (!found) {
            fprintf(stderr, "unknown region %s (use j, u or e)\n", argv[i]);
            ok = 0;
        }
    }
    return ok ? 0 : 1;
}
