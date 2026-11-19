/*
 * unpack_rom.c - split a Nintendo DS ROM into its parts for bhds-decomp.
 *
 * Usage: unpack_rom <rom.nds> <out_dir>
 *
 * Writes header.bin, arm9.bin, arm7.bin, arm9_footer.bin, overlay tables,
 * banner.bin, the NitroFS file tree under files/, and info.txt describing
 * the ARM9 module parameters (SDK version, BSS, autoloads, compression).
 *
 * Build: cc -O2 -o unpack_rom tools/unpack_rom.c
 */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir((p), 0777)
#endif

#define NITRO_CODE_LE 0x2106C0DEu
#define NITRO_CODE_BE 0xDEC00621u

static uint8_t *rom;
static size_t rom_size;
static FILE *info;

static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)(p[0] | p[1] << 8);
}

static void die(const char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

/* Print to stdout and info.txt */
static void report(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    if (info != NULL) {
        va_start(ap, fmt);
        vfprintf(info, fmt, ap);
        va_end(ap);
    }
}

/* Create every directory along path (not the final component). */
static void make_parents(const char *path) {
    char buf[1024];
    size_t i, n = strlen(path);
    if (n >= sizeof(buf)) die("path too long");
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

static void write_file(const char *out, const char *name, uint32_t off, uint32_t size) {
    char path[1024];
    FILE *f;
    if ((size_t)off + size > rom_size) die("range past end of ROM");
    snprintf(path, sizeof(path), "%s/%s", out, name);
    make_parents(path);
    f = fopen(path, "wb");
    if (f == NULL) { perror(path); exit(1); }
    if (size != 0 && fwrite(rom + off, 1, size, f) != size) die("write failed");
    fclose(f);
}

static void sdk_version_str(uint32_t v, char *buf, size_t n) {
    unsigned major = v >> 24, minor = (v >> 16) & 0xFF, relstep = v & 0xFFFF;
    char tag[32];
    if (relstep >= 30000) {
        if (relstep > 30000) snprintf(tag, sizeof(tag), "RELEASE plus%u", relstep - 30000);
        else snprintf(tag, sizeof(tag), "RELEASE");
    } else if (relstep >= 20000) {
        snprintf(tag, sizeof(tag), "RC%u", relstep - 20000);
    } else if (relstep >= 10000) {
        snprintf(tag, sizeof(tag), "PR%u", relstep - 10000);
    } else {
        snprintf(tag, sizeof(tag), "test%u", relstep);
    }
    snprintf(buf, n, "%u.%u %s (0x%08X)", major, minor, tag, (unsigned)v);
}

/* NitroFS walk */
static uint32_t fnt_off, fnt_size, fat_off, fat_size;
static unsigned file_count;
static unsigned long long file_bytes;

static void walk_dir(const char *out, uint16_t dir_id, const char *prefix, int depth) {
    const uint8_t *fnt = rom + fnt_off;
    uint32_t entry = (uint32_t)(dir_id & 0xFFF) * 8;
    uint32_t p;
    uint16_t file_id;

    if (depth > 64) die("directory tree too deep");
    if (entry + 8 > fnt_size) die("bad FNT directory entry");
    p = rd32(fnt + entry);
    file_id = rd16(fnt + entry + 4);

    for (;;) {
        uint8_t t, len;
        char name[256], child[1024];
        if (p >= fnt_size) die("FNT overrun");
        t = fnt[p++];
        if (t == 0) break;
        len = t & 0x7F;
        if (p + len > fnt_size) die("FNT name overrun");
        memcpy(name, fnt + p, len);
        name[len] = '\0';
        p += len;
        snprintf(child, sizeof(child), "%s%s", prefix, name);

        if (t & 0x80) {
            uint16_t sub;
            if (p + 2 > fnt_size) die("FNT subdir overrun");
            sub = rd16(fnt + p);
            p += 2;
            strncat(child, "/", sizeof(child) - strlen(child) - 1);
            walk_dir(out, sub, child, depth + 1);
        } else {
            uint32_t start, end;
            char rel[1100];
            if ((uint32_t)file_id * 8 + 8 > fat_size) die("bad FAT index");
            start = rd32(rom + fat_off + file_id * 8);
            end = rd32(rom + fat_off + file_id * 8 + 4);
            if (end < start) die("bad FAT entry");
            snprintf(rel, sizeof(rel), "files/%s", child);
            write_file(out, rel, start, end - start);
            file_count++;
            file_bytes += end - start;
            file_id++;
        }
    }
}

int main(int argc, char **argv) {
    const char *rom_path, *out;
    const uint8_t *h;
    uint32_t a9_off, a9_entry, a9_ram, a9_size;
    uint32_t a7_off, a7_entry, a7_ram, a7_size;
    uint32_t ov9_off, ov9_size, ov7_off, ov7_size, banner_off;
    char title[13], code[5], path[1024];
    FILE *f;
    long len;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <rom.nds> <out_dir>\n", argv[0]);
        return 1;
    }
    rom_path = argv[1];
    out = argv[2];

    f = fopen(rom_path, "rb");
    if (f == NULL) { perror(rom_path); return 1; }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len < 0x4000) die("file too small to be a DS ROM");
    rom_size = (size_t)len;
    rom = malloc(rom_size);
    if (rom == NULL) die("out of memory");
    if (fread(rom, 1, rom_size, f) != rom_size) die("read failed");
    fclose(f);

    snprintf(path, sizeof(path), "%s/info.txt", out);
    make_parents(path);
    info = fopen(path, "w");
    if (info == NULL) { perror(path); return 1; }

    h = rom;
    memcpy(title, h, 12); title[12] = '\0';
    memcpy(code, h + 12, 4); code[4] = '\0';
    report("rom: %s\n", rom_path);
    report("title=%s code=%s rev=%u\n", title, code, h[0x1E]);

    write_file(out, "header.bin", 0, 0x4000);

    a9_off = rd32(h + 0x20); a9_entry = rd32(h + 0x24); a9_ram = rd32(h + 0x28); a9_size = rd32(h + 0x2C);
    a7_off = rd32(h + 0x30); a7_entry = rd32(h + 0x34); a7_ram = rd32(h + 0x38); a7_size = rd32(h + 0x3C);
    fnt_off = rd32(h + 0x40); fnt_size = rd32(h + 0x44); fat_off = rd32(h + 0x48); fat_size = rd32(h + 0x4C);
    ov9_off = rd32(h + 0x50); ov9_size = rd32(h + 0x54); ov7_off = rd32(h + 0x58); ov7_size = rd32(h + 0x5C);
    banner_off = rd32(h + 0x68);

    write_file(out, "arm9.bin", a9_off, a9_size);
    write_file(out, "arm7.bin", a7_off, a7_size);
    report("arm9: rom=0x%X ram=0x%08X entry=0x%08X size=0x%X\n", a9_off, a9_ram, a9_entry, a9_size);
    report("arm7: rom=0x%X ram=0x%08X entry=0x%08X size=0x%X\n", a7_off, a7_ram, a7_entry, a7_size);

    /* Footer after ARM9: nitrocode, module params offset, hook offset */
    if ((size_t)a9_off + a9_size + 12 <= rom_size && rd32(rom + a9_off + a9_size) == NITRO_CODE_BE) {
        const uint8_t *arm9 = rom + a9_off;
        uint32_t mp = rd32(rom + a9_off + a9_size + 4);
        uint32_t al, al_end, al_start, bss_start, bss_end, comp_end, sdk_ver, code_be, code_le;
        char ver[64];

        write_file(out, "arm9_footer.bin", a9_off + a9_size, 12);
        if (mp + 36 > a9_size) die("module params outside ARM9");
        al = rd32(arm9 + mp);          al_end = rd32(arm9 + mp + 4);
        al_start = rd32(arm9 + mp + 8); bss_start = rd32(arm9 + mp + 12);
        bss_end = rd32(arm9 + mp + 16); comp_end = rd32(arm9 + mp + 20);
        sdk_ver = rd32(arm9 + mp + 24); code_be = rd32(arm9 + mp + 28);
        code_le = rd32(arm9 + mp + 32);
        sdk_version_str(sdk_ver, ver, sizeof(ver));

        report("arm9 footer: module params at arm9+0x%X\n", mp);
        report("  autoload list 0x%08X-0x%08X, autoload data start 0x%08X\n", al, al_end, al_start);
        report("  static bss 0x%08X-0x%08X\n", bss_start, bss_end);
        report("  compressed_static_end 0x%08X (%s)\n", comp_end, comp_end ? "COMPRESSED" : "not compressed");
        report("  sdk version %s\n", ver);
        report("  nitrocode ok: %s\n", (code_be == NITRO_CODE_BE && code_le == NITRO_CODE_LE) ? "True" : "False");

        if (comp_end == 0 && al_end >= al) {
            uint32_t i, n = (al_end - al) / 12;
            for (i = 0; i < n; i++) {
                uint32_t o = al - a9_ram + i * 12;
                if (o + 12 > a9_size) die("autoload list outside ARM9");
                report("  autoload[%u] ram=0x%08X size=0x%X bss=0x%X\n", (unsigned)i,
                       rd32(arm9 + o), rd32(arm9 + o + 4), rd32(arm9 + o + 8));
            }
        }
    } else {
        report("arm9 footer: none found\n");
    }

    write_file(out, "arm9_ovt.bin", ov9_off, ov9_size);
    write_file(out, "arm7_ovt.bin", ov7_off, ov7_size);
    report("overlays: arm9=%u arm7=%u\n", ov9_size / 32, ov7_size / 32);

    if (banner_off != 0) {
        uint16_t bver = rd16(rom + banner_off);
        uint32_t bsize = bver == 2 ? 0x940 : bver == 3 ? 0xA40 : 0x840;
        write_file(out, "banner.bin", banner_off, bsize);
    }

    if ((size_t)fnt_off + fnt_size > rom_size || (size_t)fat_off + fat_size > rom_size)
        die("FNT/FAT outside ROM");
    walk_dir(out, 0xF000, "", 0);
    report("nitrofs: %u files, %llu bytes\n", file_count, file_bytes);

    fclose(info);
    free(rom);
    return 0;
}
