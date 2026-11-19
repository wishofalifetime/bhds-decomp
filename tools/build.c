/*
 * build.c - build bhds-decomp with the original toolchain.
 *
 * Usage (run from the repository root):
 *   build                         compile every C file in src/ for JP
 *   build <j|u|e>                 compile every C file in src/ for a region
 *   build build/<r>/src/X.o       compile one object (objdiff calls it this way)
 *   build rom [<j|u|e>]           compile, link, build the ROM and verify its
 *                                 SHA-1 against baseroms.sha1 (default JP)
 *
 * The ROM build links our compiled object for every unit marked "complete"
 * in config/<r>/arm9/delinks.txt, and the delinked original for the rest.
 *
 * Environment:
 *   MWCC_DIR      folder with mwccarm.exe and mwldarm.exe
 *                 (default C:\mwccarm\1.2\sp2p3)
 *   DSD           path to the ds-decomp CLI
 *                 (default C:\ds-decomp\dsd-windows-x86_64.exe)
 *   DS_ARM7_BIOS  path to your own DS ARM7 BIOS dump. Needed for the ROM's
 *                 secure-area checksum; never put it in the repository.
 *
 * Build this tool: clang -O2 -o build/build.exe tools/build.c
 */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define MKDIR(p) _mkdir(p)
#else
#include <dirent.h>
#include <sys/stat.h>
#define MKDIR(p) mkdir((p), 0777)
#endif

#define DEFAULT_MWCC_DIR "C:\\mwccarm\\1.2\\sp2p3"
#define DEFAULT_DSD      "C:\\ds-decomp\\dsd-windows-x86_64.exe"

/* Confirmed against the JP ROM: NitroMain and system.c match with these. */
static const char *const CFLAGS =
    "-c -nolink -interworking -proc arm946e -enum int -char signed "
    "-lang c99 -Cpp_exceptions off -nostdinc -i include -O4,s";

static const char *const LDFLAGS =
    "-proc arm946e -nostdlib -interworking -nodead -m _start -map closure,unused";

/* ------------------------------------------------------------------ */
/* helpers                                                            */

static const char *env_or(const char *name, const char *def)
{
    const char *v = getenv(name);
    return (v != NULL && v[0] != '\0') ? v : def;
}

static void make_parents(const char *path)
{
    char buf[1024];
    size_t i, n = strlen(path);
    if (n >= sizeof(buf)) {
        fprintf(stderr, "path too long: %s\n", path);
        exit(1);
    }
    memcpy(buf, path, n + 1);
    for (i = 1; i < n; i++) {
        if (buf[i] == '/' || buf[i] == '\\') {
            char c = buf[i];
            buf[i] = '\0';
            if (MKDIR(buf) != 0 && errno != EEXIST) {
                perror(buf);
                exit(1);
            }
            buf[i] = c;
        }
    }
}

/* Run a command line built with printf-style formatting. Returns TRUE on success. */
static int run(const char *fmt, ...)
{
    char line[4096], cmd[4200];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
#ifdef _WIN32
    /* cmd.exe strips one pair of outer quotes */
    snprintf(cmd, sizeof(cmd), "\"%s\"", line);
#else
    snprintf(cmd, sizeof(cmd), "%s", line);
#endif
    fflush(stdout);
    return system(cmd) == 0;
}

static int valid_region(const char *r)
{
    return strcmp(r, "j") == 0 || strcmp(r, "u") == 0 || strcmp(r, "e") == 0;
}

static int file_exists(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f != NULL) {
        fclose(f);
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* compiling                                                          */

/* Compile src/<name>.c to build/<region>/src/<name>.o */
static int compile(const char *region, const char *name)
{
    char out[512];
    snprintf(out, sizeof(out), "build/%s/src/%s.o", region, name);
    make_parents(out);
    printf("CC    src/%s.c -> %s\n", name, out);
    if (!run("\"%s\\mwccarm.exe\" %s -o \"%s\" \"src/%s.c\"",
             env_or("MWCC_DIR", DEFAULT_MWCC_DIR), CFLAGS, out, name)) {
        fprintf(stderr, "error: compiling src/%s.c failed\n", name);
        return 0;
    }
    return 1;
}

static int compile_all(const char *region)
{
    int ok = 1, count = 0;
#ifdef _WIN32
    struct _finddata_t fd;
    intptr_t h = _findfirst("src/*.c", &fd);
    if (h == -1) {
        fprintf(stderr, "error: no C files in src/ (run from the repository root)\n");
        return 0;
    }
    do {
        char name[260];
        size_t n = strlen(fd.name);
        memcpy(name, fd.name, n - 2);
        name[n - 2] = '\0';
        ok &= compile(region, name);
        count++;
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir("src");
    struct dirent *e;
    if (d == NULL) {
        fprintf(stderr, "error: no src/ (run from the repository root)\n");
        return 0;
    }
    while ((e = readdir(d)) != NULL) {
        size_t n = strlen(e->d_name);
        if (n > 2 && strcmp(e->d_name + n - 2, ".c") == 0) {
            char name[260];
            memcpy(name, e->d_name, n - 2);
            name[n - 2] = '\0';
            ok &= compile(region, name);
            count++;
        }
    }
    closedir(d);
#endif
    printf("%d file(s) compiled, %s\n", count, ok ? "ok" : "FAILED");
    return ok;
}

/* Parse build/<region>/src/<name>.o and compile it */
static int compile_object(const char *path)
{
    char buf[512], *p, *region, *name, *ext;
    size_t i;

    if (strlen(path) >= sizeof(buf)) {
        return 0;
    }
    strcpy(buf, path);
    for (i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == '\\') buf[i] = '/';
    }
    p = strstr(buf, "build/");
    if (p == NULL) goto bad;
    region = p + 6;
    p = strchr(region, '/');
    if (p == NULL || strncmp(p, "/src/", 5) != 0) goto bad;
    *p = '\0';
    name = p + 5;
    ext = strrchr(name, '.');
    if (ext == NULL || strcmp(ext, ".o") != 0) goto bad;
    *ext = '\0';
    if (!valid_region(region)) goto bad;
    return compile(region, name);
bad:
    fprintf(stderr, "error: expected build/<j|u|e>/src/<name>.o, got %s\n", path);
    return 0;
}

/* ------------------------------------------------------------------ */
/* link objects                                                       */

/*
 * dsd lcf writes objects.txt with our compiled object for every unit
 * marked "complete" in delinks.txt and the delinked original for the
 * rest. Report how many of each the link uses.
 */
static int count_link_objects(const char *path)
{
    FILE *f = fopen(path, "r");
    char line[1024];
    int ours = 0, orig = 0;
    if (f == NULL) {
        perror(path);
        return 0;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strstr(line, "delinks") != NULL) {
            orig++;
        } else if (strstr(line, ".o") != NULL) {
            ours++;
        }
    }
    fclose(f);
    printf("LINK  %d object(s) from our C, %d original\n", ours, orig);
    return 1;
}

/* ------------------------------------------------------------------ */
/* SHA-1 (FIPS 180-1)                                                 */

typedef struct {
    unsigned long h[5];
    unsigned long long len;
    unsigned char buf[64];
    size_t used;
} Sha1;

#define ROL32(x, n) ((((x) << (n)) | ((x) >> (32 - (n)))) & 0xFFFFFFFFUL)

static void sha1_block(Sha1 *s, const unsigned char *p)
{
    unsigned long w[80], a, b, c, d, e, t;
    int i;
    for (i = 0; i < 16; i++) {
        w[i] = ((unsigned long)p[i * 4] << 24) | ((unsigned long)p[i * 4 + 1] << 16) |
               ((unsigned long)p[i * 4 + 2] << 8) | (unsigned long)p[i * 4 + 3];
    }
    for (i = 16; i < 80; i++) {
        w[i] = ROL32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }
    a = s->h[0]; b = s->h[1]; c = s->h[2]; d = s->h[3]; e = s->h[4];
    for (i = 0; i < 80; i++) {
        unsigned long f, k;
        if (i < 20)      { f = (b & c) | (~b & d);          k = 0x5A827999UL; }
        else if (i < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1UL; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDCUL; }
        else             { f = b ^ c ^ d;                   k = 0xCA62C1D6UL; }
        t = (ROL32(a, 5) + (f & 0xFFFFFFFFUL) + e + k + w[i]) & 0xFFFFFFFFUL;
        e = d; d = c; c = ROL32(b, 30); b = a; a = t;
    }
    s->h[0] = (s->h[0] + a) & 0xFFFFFFFFUL;
    s->h[1] = (s->h[1] + b) & 0xFFFFFFFFUL;
    s->h[2] = (s->h[2] + c) & 0xFFFFFFFFUL;
    s->h[3] = (s->h[3] + d) & 0xFFFFFFFFUL;
    s->h[4] = (s->h[4] + e) & 0xFFFFFFFFUL;
}

static void sha1_init(Sha1 *s)
{
    s->h[0] = 0x67452301UL; s->h[1] = 0xEFCDAB89UL; s->h[2] = 0x98BADCFEUL;
    s->h[3] = 0x10325476UL; s->h[4] = 0xC3D2E1F0UL;
    s->len = 0;
    s->used = 0;
}

static void sha1_update(Sha1 *s, const unsigned char *p, size_t n)
{
    s->len += n;
    while (n > 0) {
        size_t take = 64 - s->used;
        if (take > n) take = n;
        memcpy(s->buf + s->used, p, take);
        s->used += take;
        p += take;
        n -= take;
        if (s->used == 64) {
            sha1_block(s, s->buf);
            s->used = 0;
        }
    }
}

static void sha1_hex(Sha1 *s, char out[41])
{
    unsigned long long bits = s->len * 8;
    unsigned char pad = 0x80, zero = 0, lenb[8];
    int i;
    sha1_update(s, &pad, 1);
    while (s->used != 56) sha1_update(s, &zero, 1);
    for (i = 0; i < 8; i++) lenb[i] = (unsigned char)(bits >> (56 - 8 * i));
    sha1_update(s, lenb, 8);
    for (i = 0; i < 5; i++) sprintf(out + i * 8, "%08lx", s->h[i]);
}

static int sha1_file(const char *path, char out[41])
{
    static unsigned char chunk[1 << 16];
    FILE *f = fopen(path, "rb");
    Sha1 s;
    size_t n;
    if (f == NULL) {
        perror(path);
        return 0;
    }
    sha1_init(&s);
    while ((n = fread(chunk, 1, sizeof(chunk), f)) > 0) sha1_update(&s, chunk, n);
    fclose(f);
    sha1_hex(&s, out);
    return 1;
}

/* Find the expected hash for baserom.<region>.nds in baseroms.sha1 */
static int expected_sha1(const char *region, char out[41])
{
    FILE *f = fopen("baseroms.sha1", "r");
    char line[256], want[32];
    if (f == NULL) {
        perror("baseroms.sha1");
        return 0;
    }
    snprintf(want, sizeof(want), "baserom.%s.nds", region);
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strstr(line, want) != NULL && strlen(line) >= 40) {
            memcpy(out, line, 40);
            out[40] = '\0';
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    fprintf(stderr, "error: no %s entry in baseroms.sha1\n", want);
    return 0;
}

/* ------------------------------------------------------------------ */
/* ROM build                                                          */

static int build_rom(const char *region)
{
    const char *dsd = env_or("DSD", DEFAULT_DSD);
    const char *mwcc = env_or("MWCC_DIR", DEFAULT_MWCC_DIR);
    const char *bios = getenv("DS_ARM7_BIOS");
    char cfg[256], objs[256], elf[256], lcf[256];
    char romcfg[256], rom[256], got[41], want[41];

    snprintf(cfg, sizeof(cfg), "config/%s/arm9/config.yaml", region);
    snprintf(objs, sizeof(objs), "build/%s/objects.txt", region);
    snprintf(elf, sizeof(elf), "build/%s/arm9.o", region);
    snprintf(lcf, sizeof(lcf), "build/%s/arm9.lcf", region);
    snprintf(romcfg, sizeof(romcfg), "build/%s/build/rom_config.yaml", region);
    snprintf(rom, sizeof(rom), "build/%s/bhds_%s.nds", region, region);

    if (!file_exists(cfg)) {
        fprintf(stderr, "error: %s not found; region %s has no dsd config yet\n", cfg, region);
        return 0;
    }
    if (!compile_all(region)) return 0;

    printf("DSD   delink\n");
    if (!run("\"%s\" delink --config-path \"%s\"", dsd, cfg)) return 0;
    printf("DSD   lcf\n");
    if (!run("\"%s\" lcf --config-path \"%s\"", dsd, cfg)) return 0;

    if (!count_link_objects(objs)) return 0;
    if (!run("\"%s\\mwldarm.exe\" %s -o \"%s\" \"%s\" \"@%s\"", mwcc, LDFLAGS, elf, lcf, objs)) {
        fprintf(stderr, "error: link failed\n");
        return 0;
    }

    printf("DSD   check modules\n");
    if (!run("\"%s\" rom config --elf \"%s\" --config \"%s\"", dsd, elf, cfg)) return 0;
    if (!run("\"%s\" check modules --fail --config-path \"%s\"", dsd, cfg)) {
        fprintf(stderr, "error: built modules do not match the original\n");
        return 0;
    }

    printf("DSD   rom build -> %s\n", rom);
    if (bios == NULL || bios[0] == '\0') {
        printf("warning: DS_ARM7_BIOS not set; the secure-area checksum will be wrong\n");
        if (!run("\"%s\" rom build --config \"%s\" --rom \"%s\"", dsd, romcfg, rom)) return 0;
    } else {
        if (!run("\"%s\" rom build --config \"%s\" --arm7-bios \"%s\" --rom \"%s\"",
                 dsd, romcfg, bios, rom)) return 0;
    }

    if (!sha1_file(rom, got) || !expected_sha1(region, want)) return 0;
    printf("SHA1  built    %s\n      expected %s\n", got, want);
    if (strcmp(got, want) != 0) {
        printf("ROM DOES NOT MATCH\n");
        return 0;
    }
    printf("ROM MATCHES (%s)\n", region);
    return 1;
}

/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    int i, ok = 1;

    if (argc < 2) {
        return compile_all("j") ? 0 : 1;
    }
    if (strcmp(argv[1], "rom") == 0) {
        const char *region = argc > 2 ? argv[2] : "j";
        if (!valid_region(region)) {
            fprintf(stderr, "error: region must be j, u or e\n");
            return 1;
        }
        return build_rom(region) ? 0 : 1;
    }
    for (i = 1; i < argc; i++) {
        if (valid_region(argv[i])) {
            ok &= compile_all(argv[i]);
        } else {
            ok &= compile_object(argv[i]);
        }
    }
    return ok ? 0 : 1;
}
