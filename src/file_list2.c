/*
 * file_list2.c - the files screen: the list of files by row, picking a
 * file and showing its name.
 * JP (ABHJ) 0x020BF2B0 - 0x020BF96C.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "mi.h"
#include "pad.h"
#include "scd_data.h"

/* A file that can be read. 0xC bytes (see file_page.c). */
typedef struct FileEntry {
    /* 0x00 */ u8 item;
    /* 0x01 */ u8 pages;
    /* 0x02 */ u16 msgBase;
    /* 0x04 */ void *gfx;
    /* 0x08 */ u16 nameMsg;
    /* 0x0A */ u8 unk0A[2];
} FileEntry;

/* Scene work area (data_0213b214.work). */
typedef struct FileWork {
    /* 0x000 */ s8 grid[3][6];   /* file index per cell, -1: empty */
    /* 0x012 */ s8 row;
    /* 0x013 */ s8 sel;
    /* 0x014 */ u8 page;
    /* 0x015 */ s8 col;
    /* 0x016 */ s8 scrollDir;    /* 0: back, 1: forward, -1: none */
    /* 0x017 */ s8 buttons[2];
    /* 0x019 */ u8 unk019[0x81C - 0x19];
    /* 0x81C */ u8 unk81C[0x18];
} FileWork;

/* A text buffer drawn to a screen (DTCM). */
typedef struct TextBuf {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u16 pos;
    /* 0x06 */ u16 size;
    /* 0x08 */ u8 dirty;
    /* 0x09 */ u8 unk09[3];
    /* 0x0C */ u8 *buf;
} TextBuf;

typedef struct UnkDtcm_027e01ac {
    /* 0x00 */ u8 unk00[0x20];
    /* 0x20 */ s16 scroll;
} UnkDtcm_027e01ac;

#define FILE_WORK ((FileWork *)data_0213b214.work)

/* Not yet identified. JP addresses. */
extern void func_020038b8(void *a);
extern s32 func_0200e980(s32 x, s32 lo, s32 hi);  /* clamp */
extern s32 func_02013328(u32 a);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern void func_02091a88(u32 msg, void *a, u32 b, u32 c);
extern BOOL func_020bfc08(void *a);
extern void func_020bfc9c(void *a, u32 b, u32 c, u32 d, u32 e);
extern void func_020c4258(u32 a);
extern void func_02100224(void *reg, u32 a, u32 b, u32 c, u32 d);
extern void File_ClearBgs(void);
extern void File_FirstCol(void);
extern void File_HideButtons(u8 page, u8 first, u8 last);
extern u8 File_NextCol(u8 col);
extern u8 File_PrevCol(u8 col);
extern BOOL File_ScrollLeft(s32 limit, s16 step);
extern BOOL File_ScrollRight(s32 limit, s16 step);
extern void File_SetState(u8 state);
extern void Snd_Play0(u32 id);

extern void *data_02124504[];             /* JP 0x02124504: graphics by row */
extern u8 data_02124644[];                /* JP 0x02124644 */
extern const FileEntry data_021246bc[];   /* JP 0x021246BC */
#ifdef VERSION_EU
/* EU: per-language file names, 0x10 bytes per file. */
typedef struct FileNameEu {
    /* 0x00 */ u16 nameMsg;
    /* 0x02 */ u8 unk02[0xE];
} FileNameEu;

extern const FileNameEu data_e_021250d0[];  /* EU 0x021250D0 */
#endif
extern Pad data_0213a6c0;                 /* JP 0x0213A6C0 */
extern u8 data_0213a708;                  /* JP 0x0213A708: nonzero while fading */
extern u8 data_0213ec68[];                /* JP 0x0213EC68: main screen text */
extern u8 data_0213f168[];                /* JP 0x0213F168 */
extern u8 data_0213f468[];                /* JP 0x0213F468: sub screen text */
extern UnkDtcm_027e01ac data_027e01ac;    /* DTCM */
extern TextBuf data_027e01bc;             /* DTCM */
extern TextBuf data_027e01fc;             /* DTCM */

void FileList_Init(void)
{
    if (data_0213a708 != 0) {
        return;
    }
    File_ClearBgs();
    func_020038b8(data_02124644);
    func_020bfc9c(FILE_WORK->unk81C, 2, 0, 1, 1);
    data_0213b214.unk24 = 0;
    func_02100224((void *)0x04000050, 0x10, 4, 8, 8);
    File_SetState(1);
    func_0206fb04(0, 2, 0, 0x100);
}

void FileList_ShowName(u8 row, u8 col);

/* Slide to another row of files. */
void FileList_ChangeRow(void)
{
    FileWork *w;

    switch (data_0213b214.unk22) {
    case 0:
        w = FILE_WORK;
        if (w->scrollDir == 1) {
            data_027e01ac.scroll = -0xD6;
        } else {
            data_027e01ac.scroll = 0xE4;
        }
        func_020038b8(data_02124504[(u8)w->row]);
        data_0213b214.unk1E = 1;
        data_0213b214.unk22 = 1;
        break;
    case 1:
        switch (FILE_WORK->scrollDir) {
        case 1:
            if (File_ScrollRight(0, 0x32) == TRUE) {
                data_0213b214.unk22 = 2;
            }
            break;
        case 0:
            if (File_ScrollLeft(0, 0x32) == TRUE) {
                data_0213b214.unk22 = 2;
            }
            break;
        }
        break;
    case 2:
        w = FILE_WORK;
        if (w->col != -1) {
            FileList_ShowName(w->row, w->col);
        }
        File_SetState(2);
        break;
    }
}

BOOL FileList_HasCol(u8 col);
void FileList_Select(int row, int col);

/* Change rows with the D-pad/L/R, move along the row, pick a file with A. */
void FileList_Input(void)
{
    FileWork *w;
    s8 row;
    s32 col;
    s32 r;

    if (func_020bfc08(FILE_WORK->unk81C) == TRUE) {
        func_020c4258(3);
        return;
    }
    FILE_WORK->scrollDir = -1;
    w = FILE_WORK;
    row = w->row;
    col = w->col;
    w->buttons[0] = 0;
    FILE_WORK->buttons[1] = 0;
    File_HideButtons(row, 0, 2);
    r = func_02013328(1);
    if (r < 0) {
        switch (data_0213a6c0.trg) {
        case 2:
        case 0x100:
            FILE_WORK->scrollDir = 0;
            break;
        case 1:
        case 0x200:
            FILE_WORK->scrollDir = 1;
            break;
        case 4:
            if (col != -1) {
                col = (s8)File_PrevCol(col);
            }
            break;
        case 8:
            if (col != -1) {
                col = (s8)File_NextCol(col);
            }
            break;
        }
    } else if (r == 0) {
        FILE_WORK->scrollDir = 0;
    } else if (r == 1) {
        FILE_WORK->scrollDir = 1;
    }
    if (FILE_WORK->scrollDir == 0) {
        row--;
    } else if (FILE_WORK->scrollDir == 1) {
        row++;
    }
    row = func_0200e980(row, 0, 2);
    if ((u8)FILE_WORK->row != row) {
        data_0213b214.blink = 8;
        FILE_WORK->row = row;
        File_SetState(3);
        data_0213b214.unk1E = 0;
        Snd_Play0(1);
    }
    r = func_02013328(0);
    if (r < 0) {
        if ((data_0213a6c0.trg & 0x10) && FileList_HasCol(col) == TRUE) {
            FileList_Select(row, col);
            return;
        }
    } else if (FileList_HasCol(r) == TRUE) {
        if (col == r) {
            FileList_Select(row, col);
            return;
        }
        col = r;
    }
    if (FILE_WORK->col != col) {
        FileList_ShowName(row, col);
        FILE_WORK->col = col;
        Snd_Play0(1);
    }
}

void TextBuf_Clear(u32 which);

void FileList_Scroll(void)
{
    switch (data_0213b214.unk22) {
    case 0:
        TextBuf_Clear(0);
        FILE_WORK->col = -1;
        File_FirstCol();
        data_0213b214.unk22 = 1;
        break;
    case 1:
        switch (FILE_WORK->scrollDir) {
        case 1:
            if (File_ScrollRight(0xE4, 0x32) == TRUE) {
                File_SetState(1);
            }
            FILE_WORK->buttons[1] = 1;
            break;
        case 0:
            if (File_ScrollLeft(-0xD6, 0x32) == TRUE) {
                File_SetState(1);
            }
            FILE_WORK->buttons[0] = 1;
            break;
        }
        break;
    }
}

void FileList_ShowName(u8 row, u8 col)
{
    TextBuf_Clear(0);
#ifdef VERSION_EU
    func_02091a88(data_e_021250d0[(u8)FILE_WORK->grid[row][col]].nameMsg | 0xE900, data_0213f168, 0x40,
                  0x01003100);
#else
    func_02091a88(data_021246bc[(u8)FILE_WORK->grid[row][col]].nameMsg | 0xE900, data_0213f168, 0x40,
                  0x01003100);
#endif
}

/* Clear the text buffer of the main (0) or sub screen. */
void TextBuf_Clear(u32 which)
{
    u8 *buf;
    TextBuf *t;

    if (which == 0) {
        buf = data_0213ec68;
        t = &data_027e01bc;
    } else {
        buf = data_0213f468;
        t = &data_027e01fc;
    }
    MI_CpuClear16(buf, 0x600);
    t->pos = 0;
    t->buf = buf;
    t->size = 0x600;
    t->dirty = 1;
}

void FileList_Select(int row, int col)
{
    Snd_Play0(3);
    TextBuf_Clear(0);
    FILE_WORK->sel = FILE_WORK->grid[row][col];
    func_020c4258(2);
    func_0206fb04(0, 1, 1, 0x100);
}

/* Is there a file in this column of the current row? */
BOOL FileList_HasCol(u8 col)
{
    s8 *row = FILE_WORK->grid[(u8)FILE_WORK->row];
    u8 i;

    for (i = 0; i < 6; i++) {
        if (row[i] != -1 && i == col) {
            return TRUE;
        }
    }
    return FALSE;
}
