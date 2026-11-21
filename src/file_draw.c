/*
 * file_draw.c - the files screen: drawing the page counter, the grid of
 * files and the buttons.
 * JP (ABHJ) 0x020BE45C - 0x020BE714.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "mi.h"
#include "scd_data.h"

/* A 2D sprite passed to func_020062ac (see choice.h). */
typedef struct FileSprite {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u8 frame;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u16 unk04;
    /* 0x06 */ u16 x;
    /* 0x08 */ u16 y;
    /* 0x0A */ u16 unk0A;
} FileSprite;

typedef struct FilePos {
    /* 0x00 */ u8 x;
    /* 0x01 */ u8 y;
    /* 0x02 */ u8 unk02[2];
} FilePos;

/* Scene work area (data_0213b214.work). */
typedef struct FileWork {
    /* 0x000 */ s8 grid[3][6];   /* file index per cell, -1: empty */
    /* 0x012 */ u8 row;
    /* 0x013 */ u8 sel;
    /* 0x014 */ u8 page;
    /* 0x015 */ s8 col;
    /* 0x016 */ u8 unk016;
    /* 0x017 */ s8 buttons[2];   /* 1: blinking, -1: hidden */
    /* 0x019 */ u8 unk019[0x81C - 0x19];
    /* 0x81C */ u8 unk81C[0x18];
} FileWork;

typedef struct UnkDtcm_027e01ac {
    /* 0x00 */ u8 unk00[0x10];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 unk12[0x20 - 0x12];
    /* 0x20 */ s16 unk20;
} UnkDtcm_027e01ac;

#define FILE_WORK ((FileWork *)data_0213b214.work)

/* Not yet identified. JP addresses. */
extern void func_02003a0c(u32 a);
extern void func_020062ac(u32 layer, FileSprite *spr);
extern void func_020bfbd0(void *a);
extern s32 func_0210315c(char *dst, const char *fmt, ...);  /* sprintf */
extern BOOL File_Blink(void);
extern u8 File_PageCount(void);

extern u8 data_021244dc[];                /* JP 0x021244DC: button sprite ids */
extern FilePos data_021244f8[];           /* JP 0x021244F8: button positions */
extern FilePos data_0212465c[];           /* JP 0x0212465C: cell positions */
extern char data_02124794[];              /* JP 0x02124794: "%d/%d" */
extern char data_0213ab38[];              /* JP 0x0213AB38 */
extern UnkDtcm_027e01ac data_027e01ac;    /* DTCM */

void File_ClearBgs(void)
{
    func_02003a0c(0);
    func_02003a0c(1);
    func_02003a0c(2);
    func_02003a0c(3);
}

void File_Draw(void)
{
    FileSprite spr;
    u8 *s;
    u8 c;
    u8 i;
    u8 row;

    data_027e01ac.unk10 = data_027e01ac.unk20;
    MI_CpuClear16(&spr, sizeof(spr));
    if (data_0213b214.state == 2 && data_0213b214.unk21 != 0) {
        u8 page;

        spr.unk03 = 2;
        spr.x = 8;
        spr.y = 8;
        page = data_0213b214.unk1D + 1;
        s = (u8 *)data_0213ab38;
        func_0210315c(data_0213ab38, data_02124794, page, File_PageCount());
        while ((c = *s++) != 0) {
            if (c != ' ') {
                spr.id = c - 0x2B;
                if (data_0213b214.unk25 != 1) {
                    func_020062ac(9, &spr);
                }
            }
            spr.x += 8;
        }
    }
    MI_CpuClear16(&spr, sizeof(spr));
    if (data_0213b214.unk1E == 1) {
        row = FILE_WORK->row;
        for (i = 0; i < 6; i++) {
            spr.id = 1;
            if (FILE_WORK->grid[row][i] != -1) {
                if (i == FILE_WORK->col) {
                    spr.id--;
                }
                spr.x = data_0212465c[i].x - data_027e01ac.unk20;
                spr.y = data_0212465c[i].y;
                func_020062ac(0, &spr);
            }
        }
    }
    for (i = 0; i < 2; i++) {
        spr.x = data_021244f8[i].x;
        spr.y = data_021244f8[i].y;
        spr.id = data_021244dc[i];
        if (i == 1 && data_0213b214.unk24 == 1) {
            spr.id = 0x19;
        }
        if (FILE_WORK->buttons[i] == 1) {
            spr.id -= File_Blink();
        }
        if (FILE_WORK->buttons[i] != -1 && data_0213b214.unk25 != 1) {
            func_020062ac(1, &spr);
        }
    }
    if (data_0213b214.unk25 != 1) {
        func_020bfbd0(FILE_WORK->unk81C);
    }
}
