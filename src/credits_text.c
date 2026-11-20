/*
 * credits_text.c - scrolling text screen: BG scroll and text output. Not
 * matching yet: Credits_WriteText and Credits_SetScroll differ in register
 * allocation (SetScroll also reloads data_0213b214.work after each store).
 * JP (ABHJ) 0x020B8778 - 0x020B88A4.
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

/* Not yet identified. JP addresses. */
extern void func_020059ac(void);
extern BOOL func_02005be4(void);
extern void func_02005c34(void);
extern void func_02091a88(const void *src, void *dst, u16 len, u32 attr);
extern void func_020c365c(void *a);
extern void func_020c4174(u32 a);
extern u16 data_0213a708;                    /* JP 0x0213A708 */
extern u8 data_0213b264[];                   /* JP 0x0213B264 */
extern u8 data_0213ec68[];                   /* JP 0x0213EC68: 2 x 0x800 bytes */
extern UnkDtcm_BgScroll data_027e01ac;       /* DTCM */
extern UnkDtcm_BgScroll data_027e01ec;       /* DTCM */
extern UnkDtcm_027e01bc data_027e01bc;       /* DTCM */
extern UnkDtcm_027e01bc data_027e01fc;       /* DTCM */

void Credits_SetScroll(s32 a, s32 b)
{
    data_027e01ac.unk12 = a >> 8;
    data_027e01ac.unk32 = CREDITS->fade2 >> 8;
    data_027e01ec.unk12 = b >> 8;
    data_027e01ec.unk22 = CREDITS->fade0 >> 8;
    data_027e01ec.unk32 = CREDITS->fade1 >> 8;
}

void Credits_Exit(void)
{
    func_02005c34();
    if (!func_02005be4() && data_0213a708 == 0) {
        func_020059ac();
        func_020c365c(data_0213b264);
        func_020c4174(0);
    }
}

/* Write text into BG map which at (x, y) and mark the map for upload. */
void Credits_WriteText(u32 which, u8 x, u8 y, const void *src, u16 len, u8 palette)
{
    UnkDtcm_027e01bc *t;
    u8 prio;
    u8 *base = data_0213ec68 + which * 0x800;

    if (which == 0) {
        prio = 1;
        t = &data_027e01bc;
    } else {
        prio = 5;
        t = &data_027e01fc;
    }
    func_02091a88(src, base + (x + y * 32) * 2, len, ((palette << 3) & 0xFFF) | 0x1000 | (prio << 24));
    t->unk04 = 0;
    t->src = base;
    t->size = 0x800;
    t->dirty = 1;
}
