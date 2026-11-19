/*
 * pad.c - button input.
 * JP (ABHJ): Pad_Update at 0x02073B98.
 * Status: NOT matching (20/45). Same instructions and order, but the lid
 * flag lands in r1 and the key mask in r0 in the original (swapped here).
 * Tried without success: declaration order, BOOL/int/u8/u16/s32 types,
 * macro vs inline PAD helpers, open-coded fold test, xy temp, ternary,
 * pointer local, no prev temp, store order, keys=0 + if(!folded).
 */
#include "types.h"
#include "sdk.h"
#include "pad.h"

/* Not yet identified. JP addresses. */
extern void func_02073940(void);
extern void func_02073b44(void);

void Pad_Update(void)
{
    u16 keys;
    u16 prev;
    BOOL folded = PAD_DetectFold();

    if (folded) {
        keys = 0;
    } else {
        keys = PAD_Read();
        /* Drop impossible opposite-direction combinations */
        if ((keys & (PAD_KEY_RIGHT | PAD_KEY_LEFT)) == (PAD_KEY_RIGHT | PAD_KEY_LEFT)) {
            keys ^= PAD_KEY_RIGHT | PAD_KEY_LEFT;
        }
        if ((keys & (PAD_KEY_UP | PAD_KEY_DOWN)) == (PAD_KEY_UP | PAD_KEY_DOWN)) {
            keys ^= PAD_KEY_UP | PAD_KEY_DOWN;
        }
    }

    prev = gPad.cont;
    gPad.lidClosed = folded;
    gPad.prev = prev;
    gPad.cont = keys;
    gPad.trg = keys & ~gPad.prev;

    func_02073b44();
    func_02073940();
}
