/*
 * system.c - per-frame system handling: lid-close sleep and soft reset.
 * JP (ABHJ): Main_LidSleep 0x0207812C, Main_CheckSoftReset 0x020781E0.
 * Status: decompiled, NOT yet verified to match (compiler not set up).
 */
#include "types.h"
#include "sdk.h"
#include "pad.h"
#include "game.h"

/* Not yet identified. JP addresses. */
extern void func_020b7ce0(void);
extern void func_020d8acc(void);
extern BOOL func_0210a7d0(u32 on);                      /* probably PM_SetLCDPower */
extern void func_0210a8e0(u32 trigger, u32 logic, u16 keys); /* probably PM_GoSleepMode */

#define SOFT_RESET_KEYS (PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START | PAD_BUTTON_SELECT)

/* TRUE when L+R+Start+Select is held and one of them was just pressed. */
BOOL Main_CheckSoftReset(void)
{
    if (gGame.unk09 == 0 && (gPad.trg & SOFT_RESET_KEYS) != 0 && gPad.cont == SOFT_RESET_KEYS) {
        return TRUE;
    }
    return FALSE;
}

/* Put the system to sleep while the lid is closed, then restore the screens. */
void Main_LidSleep(void)
{
    if (gPad.lidClosed) {
        func_020b7ce0();
        func_020d8acc();
        Main_EndFrame();
        GX_DispOff();
        GXS_DispOff();
        func_0210a7d0(0);
        func_0210a8e0(0xC, 0, 0);
        Main_EndFrame();
        while (!func_0210a7d0(1)) {
        }
        Main_EndFrame();
        GX_SetPower(POWCNT1_ALL_ENGINES);
        GX_DispOn();
        GXS_DispOn();
    }
}
