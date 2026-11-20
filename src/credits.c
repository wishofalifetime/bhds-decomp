/*
 * credits.c - scrolling text screen: two columns of text lines drawn into
 * BG maps and scrolled up, then faded out.
 * JP (ABHJ) 0x020B88A4 - 0x020B8D6C.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "hw_regs.h"
#include "game.h"
#include "pad.h"
#include "scd_data.h"
#include "credits.h"

/* NitroSDK GX_SetVisiblePlane / GXS_SetVisiblePlane */
#define GX_SetVisiblePlane(p)  (REG_DISPCNT = (REG_DISPCNT & ~0x1F00) | ((p) << 8))
#define GXS_SetVisiblePlane(p) (REG_DB_DISPCNT = (REG_DB_DISPCNT & ~0x1F00) | ((p) << 8))

/* Not yet identified. JP addresses. */
extern void func_0200582c(u32 a);
extern void func_02005738(void);
extern void func_020059f0(u32 a);
extern u32 func_02005ad8(void);
extern void func_02005c14(u32 a, s32 b);
extern void func_02005c34(void);
extern void func_020035e4(u32 a, void *buf, u32 b, u32 size);
extern void func_020038b8(const void *a);
extern void func_0200f660(u32 a);
extern void func_0200f6e8(u32 a);
extern void func_0200f770(u32 a);
extern void func_0200f804(void);
extern void func_0200fa18(void);
extern void func_0200fa68(void);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern void func_020c3690(void *a, u32 b);
extern void func_020c36cc(void *a, u32 b, u32 c, u32 d);
extern void func_020fec40(u32 a, u32 b, u32 c);
extern void func_02105c14(void *dst, u32 value, u32 size);  /* probably MI_CpuFill8 */
extern void G3d_ClearRows(u32 which, u8 row, u8 count);
extern s8 *data_0212eb2c[];                  /* JP 0x0212EB2C: text lines, NULL-terminated */
extern const u8 data_021237c4[];             /* JP 0x021237C4 */
extern u8 data_0213b230[];                   /* JP 0x0213B230 */
extern u8 data_0213b264[];                   /* JP 0x0213B264 */

void Credits_Init(void)
{
    func_020c36cc(data_0213b230, 0, 0, 0x28);
    func_020c3690(data_0213b264, func_02005ad8());
    func_020059f0(data_0213b214.unk80);
    REG_POWCNT1 |= 0x8000;
    func_020fec40(1, 0, 0);
    func_0200fa68();
    func_0200fa18();
    func_0200f804();
    func_0200f770(6);
    func_0200f6e8(0x14);
    func_0200f660(8);
    func_02105c14(gGame.workBuf, 0, 0x8000);
    func_020035e4(1, gGame.workBuf, 0, 0x8000);
    func_020035e4(5, gGame.workBuf, 0, 0x8000);
    func_020038b8(data_021237c4);
    REG_BG3CNT = (REG_BG3CNT & 0x43) | 0x9E10;
    REG_DB_BG2CNT = (REG_DB_BG2CNT & 0x43) | 0x9C10;
    REG_DB_BG3CNT = (REG_DB_BG3CNT & 0x43) | 0x9E10;
    GX_SetVisiblePlane(2);
    GXS_SetVisiblePlane(2);
    data_0213b214.speed = 0x150;
    func_02105c14(data_0213b214.work, 0, 8);
    CREDITS->lines[0].pos = 0;
    CREDITS->lines[1].pos = 0xC000;
    CREDITS->fade0 = -0x10000;
    CREDITS->fade1 = -0x10000;
    CREDITS->fade2 = -0x10000;
    Credits_SetScroll((s16)CREDITS->lines[0].pos, (s16)CREDITS->lines[1].pos);
    func_0200582c(0x19);
    func_02005738();
    func_0206fb04(4, 2, 0, 0x100);
    data_0213b214.state = 1;
}

void Credits_Update(void)
{
    BOOL done;

    data_0213b214.speed = 0x150;
    Credits_SetScroll((s16)CREDITS->lines[0].pos, (s16)CREDITS->lines[1].pos);
    done = Credits_ScrollLine(1);
    if (done) {
        done = FALSE;
        GXS_SetVisiblePlane(0xA);
        if (CREDITS->fade1 < 0x10000) {
            CREDITS->fade1 += data_0213b214.speed;
        }
        if (CREDITS->fade1 > 0) {
            GXS_SetVisiblePlane(0xE);
            if (CREDITS->fade0 < 0) {
                CREDITS->fade0 += data_0213b214.speed;
                if (CREDITS->fade0 > 0) {
                    CREDITS->fade0 = 0;
                }
            } else {
                done = TRUE;
            }
            GX_SetVisiblePlane(0xA);
            if (CREDITS->fade2 < 0) {
                CREDITS->fade2 += data_0213b214.speed;
                if (CREDITS->fade2 > 0) {
                    CREDITS->fade2 = 0;
                }
            }
        }
    }
    if (CREDITS->lines[1].pos > 0xE000) {
        Credits_ScrollLine(0);
    }
    func_02005c34();
    if (Credits_CheckSkip(done)) {
        func_02005c14(0, -0x18);
        func_0206fb04(4, 2, 1, 0x100);
        data_0213b214.state = 2;
    }
}

/* Leave when done and any button was pressed, or on START. */
u32 Credits_CheckSkip(BOOL done)
{
    u32 ret = 0;

    if (done) {
        ret |= gPad.trg != 0;
    }
    return ret | (gPad.trg & 8);
}

/* Scroll column which; draw the next text line when it reaches a new row. */
BOOL Credits_ScrollLine(u32 which)
{
    CreditsLine *l = &CREDITS->lines[which];
    u32 row;
    BOOL end = FALSE;

    if (data_0212eb2c[l->index] != NULL) {
        u32 y = l->pos >> 8;

        if (y > 0xC0) {
            row = ((y + 0xC0) >> 4) & 0xF;

            if (row != l->lastRow) {
                G3d_ClearRows(which, row * 2, 4);
                if (data_0212eb2c[l->index][0] != 0) {
                    Credits_WriteText(which, 1, row * 2, data_0212eb2c[l->index], row * 0x40 + 1, 0x1E);
                }
                l->index++;
                l->lastRow = row;
            }
        }
        l->pos += data_0213b214.speed;
    } else {
        end = TRUE;
    }
    return end;
}
