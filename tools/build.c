/*
 * build.c - compile the game's C sources with the original compiler.
 *
 * Usage:
 *   build                        build every C file in src/ for JP
 *   build <region>               build every C file in src/ for j, u or e
 *   build build/<region>/src/X.o build one object (objdiff calls it this way)
 *
 * Environment:
 *   MWCC_DIR  folder with mwccarm.exe (default C:\mwccarm\1.2\sp2p3)
 *
 * Build this tool: clang -O2 -o build/build.exe tools/build.c
 * Run from the repository root.
 */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* Confirmed against the JP ROM: NitroMain and system.c match with these. */
static const char *const CFLAGS =
    "-c -nolink -interworking -proc arm946e -enum int -char signed "
    "-lang c99 -nostdinc -i include -O4,s";

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

static int valid_region(const char *r)
{
    return strcmp(r, "j") == 0 || strcmp(r, "u") == 0 || strcmp(r, "e") == 0;
}

/* Compile src/<name>.c to build/<region>/src/<name>.o */
static int compile(const char *region, const char *name)
{
    const char *dir = getenv("MWCC_DIR");
    char out[512], cmd[2048];
    int rc;

    if (dir == NULL || dir[0] == '\0') {
        dir = DEFAULT_MWCC_DIR;
    }
    snprintf(out, sizeof(out), "build/%s/src/%s.o", region, name);
    make_parents(out);
    snprintf(cmd, sizeof(cmd), "\"\"%s\\mwccarm.exe\" %s -o \"%s\" \"src/%s.c\"\"",
             dir, CFLAGS, out, name);
#ifndef _WIN32
    /* Outer quotes are only needed by cmd.exe */
    memmove(cmd, cmd + 1, strlen(cmd));
    cmd[strlen(cmd) - 1] = '\0';
#endif
    printf("CC  src/%s.c -> %s\n", name, out);
    fflush(stdout);
    rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "error: compiling src/%s.c failed\n", name);
    }
    return rc == 0;
}

static int build_all(const char *region)
{
    int ok = 1, count = 0;
#ifdef _WIN32
    struct _finddata_t fd;
    intptr_t h = _findfirst("src/*.c", &fd);
    if (h == -1) {
        fprintf(stderr, "error: no src/*.c (run from the repository root)\n");
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
    printf("%d file(s), %s\n", count, ok ? "ok" : "FAILED");
    return ok;
}

/* Parse build/<region>/src/<name>.o */
static int build_object(const char *path)
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

int main(int argc, char **argv)
{
    int i, ok = 1;

    if (argc < 2) {
        return build_all("j") ? 0 : 1;
    }
    for (i = 1; i < argc; i++) {
        if (valid_region(argv[i])) {
            ok &= build_all(argv[i]);
        } else {
            ok &= build_object(argv[i]);
        }
    }
    return ok ? 0 : 1;
}
