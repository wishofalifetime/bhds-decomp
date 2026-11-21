/*
 * file_scroll.c - the files screen: scrolling the row of files, the page
 * buttons and leaving the screen.
 * JP (ABHJ) 0x020BEB18 - 0x020BEDD8. The US and EU versions also refresh
 * the page text after scrolling and reset it when leaving.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"

/* Scene work area (data_0213b214.work). */
typedef struct FileWork {
    /* 0x000 */ s8 grid[3][6];
    /* 0x012 */ u8 row;
    /* 0x013 */ u8 sel;
    /* 0x014 */ u8 page;
    /* 0x015 */ s8 col;
    /* 0x016 */ s8 scrollDir;    /* 0: left, 1: right */
    /* 0x017 */ s8 buttons[2];   /* 1: blinking, -1: hidden */
    /* 0x019 */ u8 unk019[0x834 - 0x19];
    /* 0x834 */ u8 unk834;
} FileWork;

typedef struct UnkDtcm_027e01ac {
    /* 0x00 */ u8 unk00[0x12];
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 unk14[0x20 - 0x14];
    /* 0x20 */ s16 scroll;
} UnkDtcm_027e01ac;

#define FILE_WORK ((FileWork *)data_0213b214.work)

/* Not yet identified. JP addresses. */
extern void func_02003b00(u32 a);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern void func_02092b10(u32 msg, u32 a);
extern void func_020c4174(u32 a);
extern void func_020c4258(u32 a);
extern void Bgm_ClearSlot(u32 i);
extern void File_ClearBgs(void);
extern u8 File_PageCount(void);
#ifndef VERSION_JP
extern void File_Refresh(void);  /* US/EU only */
#endif

extern UnkDtcm_027e01ac data_027e01ac;    /* DTCM */

/* Hide the left button on the first page and the right one on the last. */
void File_HideButtons(s32 page, s32 first, s32 last)
{
    if (first == page) {
        FILE_WORK->buttons[0] = -1;
        return;
    }
    if (last == page && data_0213b214.state != 2) {
        FILE_WORK->buttons[1] = -1;
    }
}

BOOL File_ScrollRight(s32 limit, s16 step);
BOOL File_ScrollLeft(s32 limit, s16 step);
void File_SetState(u8 state);

void File_Scroll(void)
{
    File_PageCount();
    switch (FILE_WORK->scrollDir) {
    case 1:
        if (File_ScrollRight(0xE4, 0x32) == TRUE) {
            File_SetState(1);
#ifndef VERSION_JP
            File_Refresh();
#endif
        }
        FILE_WORK->buttons[1] = 1;
        break;
    case 0:
        if (File_ScrollLeft(-0xD6, 0x32) == TRUE) {
            File_SetState(1);
#ifndef VERSION_JP
            File_Refresh();
#endif
        }
        FILE_WORK->buttons[0] = 1;
        data_0213b214.unk24 = 0;
        break;
    }
}

/* Leave the screen, or go back to the file list. */
void File_Exit(void)
{
    switch (data_0213b214.unk22) {
    case 0:
        data_0213b214.unk22 = 1;
        data_0213b214.speed = 10;
        break;
    case 1:
    default:
        if (data_0213b214.speed != 0) {
            data_0213b214.speed--;
        } else {
            File_ClearBgs();
#ifdef VERSION_EU
            data_0213b214.unk1C = 0;
            data_027e01ac.unk12 = 0;
#endif
            if (data_0213b750.flags & 0x100) {
                func_02092b10(0xEA06, 0);
                data_0213b214.unk25 = 1;
                data_0213b214.unk22 = 2;
            } else {
                func_0206fb04(0, 1, 1, 0x100);
                func_02003b00(2);
                data_027e01ac.scroll = 0xE4;
#ifdef VERSION_US
                data_0213b214.unk1C = 0;
                data_027e01ac.unk12 = 0;
#endif
                FILE_WORK->scrollDir = 1;
                FILE_WORK->buttons[0] = -1;
                FILE_WORK->buttons[1] = -1;
                FILE_WORK->page = 0;
                func_020c4258(1);
                data_0213b214.unk21 = 0;
            }
        }
        break;
    case 2:
        if (!(data_0213a71c.unk05 & 0x80)) {
            if (FILE_WORK->unk834 != 0) {
                Bgm_ClearSlot(0);
            }
            func_020c4174(0);
        }
        break;
    }
}

BOOL File_ScrollRight(s32 limit, s16 step)
{
    data_027e01ac.scroll += step;
    if (limit < data_027e01ac.scroll) {
        data_027e01ac.scroll = limit;
        data_0213b214.blink = 0;
        return TRUE;
    }
    return FALSE;
}

BOOL File_ScrollLeft(s32 limit, s16 step)
{
    data_027e01ac.scroll -= step;
    if (limit > data_027e01ac.scroll) {
        data_027e01ac.scroll = limit;
        data_0213b214.blink = 0;
        return TRUE;
    }
    return FALSE;
}

void File_SetState(u8 state)
{
    data_0213b214.unk21 = state;
    data_0213b214.unk22 = 0;
}
