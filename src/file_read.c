/*
 * file_read.c - the files screen: reading a file page by page, and moving
 * between the files of a row.
 * JP (ABHJ) 0x020BEDD8 - 0x020BF1F8, and the File_Update table template
 * in .data (JP 0x0212EF2C).
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "pad.h"
#include "scd_data.h"

/* Scene work area (data_0213b214.work). */
typedef struct FileWork {
    /* 0x000 */ s8 grid[3][6];   /* file index per cell, -1: empty */
    /* 0x012 */ u8 row;
    /* 0x013 */ u8 sel;
    /* 0x014 */ u8 page;
    /* 0x015 */ s8 col;
    /* 0x016 */ s8 scrollDir;    /* 0: back, 1: forward, -1: none */
    /* 0x017 */ s8 buttons[2];
    /* 0x019 */ u8 unk019[0x81C - 0x19];
    /* 0x81C */ u8 unk81C[0x18];
} FileWork;

typedef struct UnkDtcm_027e01ac {
    /* 0x00 */ u8 unk00[0x20];
    /* 0x20 */ s16 scroll;
} UnkDtcm_027e01ac;

#define FILE_WORK ((FileWork *)data_0213b214.work)

/* Not yet identified. JP addresses. */
extern s32 func_0200e980(s32 x, s32 lo, s32 hi);  /* clamp */
extern s32 func_02013328(u32 a);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern BOOL func_020bfc08(void *a);
extern void func_020bfc9c(void *a, u32 b, u32 c, u32 d, u32 e);
extern void File_ClearBgs(void);
extern void File_Exit(void);
extern void File_HideButtons(u8 page, u8 first, u8 last);
extern void File_LoadGfx(void);
extern u8 File_PageCount(void);
extern void File_Scroll(void);
extern BOOL File_ScrollLeft(s32 limit, s16 step);
extern BOOL File_ScrollRight(s32 limit, s16 step);
extern void File_SetState(u8 state);
extern void File_ShowPage(void);
extern void Snd_Play0(u32 id);

extern Pad data_0213a6c0;                 /* JP 0x0213A6C0 */
extern u8 data_0213a708;                  /* JP 0x0213A708: nonzero while fading */
extern UnkDtcm_027e01ac data_027e01ac;    /* DTCM */

#define REG_BLDCNT (*(vu16 *)0x04000050)

/* Put the cursor on the first file of the row. */
void File_FirstCol(void)
{
    u8 i;
    s8 *row;
    FileWork *w;

    w = FILE_WORK;
    row = w->grid[w->row];

    for (i = 0; i < 6; i++) {
        if (row[i] != -1) {
            w->col = i;
            break;
        }
    }
}

void File_ReadInit(void);
void File_PageTurn(void);
void File_ReadInput(void);

void File_Update(void)
{
    VoidFunc funcs[] = { File_ReadInit, File_PageTurn, File_ReadInput, File_Scroll, File_Exit };

    funcs[data_0213b214.unk21]();
}

void File_ReadInit(void)
{
    if (data_0213a708 != 0) {
        return;
    }
    data_027e01ac.scroll = 0xE4;
    FILE_WORK->scrollDir = 1;
    File_ClearBgs();
    File_LoadGfx();
    FILE_WORK->buttons[0] = -1;
    FILE_WORK->buttons[1] = 0;
    data_0213b214.unk1D = 0;
    data_0213b214.unk1E = 0;
    func_020bfc9c(FILE_WORK->unk81C, 2, 0, 1, 1);
    REG_BLDCNT = 0;
    File_SetState(1);
    func_0206fb04(0, 2, 0, 0x100);
}

/* Slide the old page out and the new one in. */
void File_PageTurn(void)
{
    switch (data_0213b214.unk22) {
    case 0:
        if (FILE_WORK->scrollDir == 1) {
            data_027e01ac.scroll = -0xD6;
        } else {
            data_027e01ac.scroll = 0xE4;
        }
        File_ShowPage();
        data_0213b214.unk22 = 1;
        break;
    case 1:
        switch (FILE_WORK->scrollDir) {
        case 1:
            if (File_ScrollRight(0, 0x32) == TRUE) {
                data_0213b214.unk1D = FILE_WORK->page;
                File_SetState(2);
            }
            break;
        case 0:
            if (File_ScrollLeft(0, 0x32) == TRUE) {
                data_0213b214.unk1D = FILE_WORK->page;
                File_SetState(2);
            }
            break;
        }
        break;
    }
}

/* Turn pages with the D-pad, L/R or the touch screen. */
void File_ReadInput(void)
{
    u32 page;
    u32 last;
    s32 r;

    if (func_020bfc08(FILE_WORK->unk81C) == TRUE) {
        File_SetState(4);
        return;
    }
    FILE_WORK->scrollDir = -1;
    page = FILE_WORK->page;
    last = File_PageCount() - 1;
    FILE_WORK->buttons[0] = 0;
    FILE_WORK->buttons[1] = 0;
    if (page == last) {
        data_0213b214.unk24 = 1;
    }
    File_HideButtons(page, 0, last);
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
        }
    } else if (r == 0) {
        FILE_WORK->scrollDir = 0;
    } else if (r == 1) {
        FILE_WORK->scrollDir = 1;
    }
    if (FILE_WORK->scrollDir == 0) {
        page--;
    } else if (FILE_WORK->scrollDir == 1) {
        page++;
        if (page > last) {
            data_0213b214.blink = 8;
            FILE_WORK->buttons[1] = 1;
            File_SetState(4);
            Snd_Play0(2);
            return;
        }
    }
    page = func_0200e980(page, 0, last);
    if (FILE_WORK->page != page) {
        FILE_WORK->page = page;
        data_0213b214.blink = 8;
        File_SetState(3);
        Snd_Play0(5);
    }
}
