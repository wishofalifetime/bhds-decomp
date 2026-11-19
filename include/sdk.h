#ifndef BHDS_SDK_H
#define BHDS_SDK_H

/*
 * NitroSDK 2.2 functions and inline helpers the game uses.
 * Declarations are written for this project, not copied from the SDK.
 * Addresses are JP (ABHJ).
 */
#include "types.h"
#include "hw_regs.h"

/* --- Shared main RAM written by the ARM7 --- */
/* X/Y, debug and lid state forwarded from the ARM7. The bit layout is
 * NitroSDK's own (not documented in GBATEK); inferred from game code. */
#define HW_BUTTON_XY_BUF        (*(vu16 *)0x027FFFA8)
#define HW_RESET_PARAMETER_BUF  (*(vu32 *)0x027FFC20)

/* --- Buttons (combined KEYINPUT + ARM7 X/Y) --- */
#define PAD_BUTTON_A      0x0001
#define PAD_BUTTON_B      0x0002
#define PAD_BUTTON_SELECT 0x0004
#define PAD_BUTTON_START  0x0008
#define PAD_KEY_RIGHT     0x0010
#define PAD_KEY_LEFT      0x0020
#define PAD_KEY_UP        0x0040
#define PAD_KEY_DOWN      0x0080
#define PAD_BUTTON_R      0x0100
#define PAD_BUTTON_L      0x0200
#define PAD_BUTTON_X      0x0400
#define PAD_BUTTON_Y      0x0800
#define PAD_BUTTON_DEBUG  0x2000
#define PAD_ALL_MASK      0x2FFF
#define PAD_FOLD_MASK     0x8000

/* Pressed buttons, 1 = pressed */
static inline u16 PAD_Read(void)
{
    return (u16)(((REG_KEYINPUT | HW_BUTTON_XY_BUF) ^ PAD_ALL_MASK) & PAD_ALL_MASK);
}

/* TRUE when the lid is closed */
static inline BOOL PAD_DetectFold(void)
{
    return (BOOL)((HW_BUTTON_XY_BUF & PAD_FOLD_MASK) >> 15);
}

/* --- BIOS call stubs (Thumb) --- */
void VBlankIntrWait(void);  /* 0x02000662, SVC 0x05 */

/* --- GX --- */
void GX_DispOn(void);   /* 0x020FECB4 */
void GX_DispOff(void);  /* 0x020FED00 */
#define GXS_DispOn()  (REG_DB_DISPCNT |= DISPCNT_MODE_GRAPHICS)
#define GXS_DispOff() (REG_DB_DISPCNT &= ~DISPCNT_MODE_GRAPHICS)
#define GX_SetPower(power) \
    (REG_POWCNT1 = (u16)((REG_POWCNT1 & ~POWCNT1_ALL_ENGINES) | (power)))

/* --- G3X --- */
void G3X_Reset(void);   /* 0x021007BC */

/* --- OS --- */
void OS_ResetSystem(u32 parameter);  /* 0x02104F78 */

#endif /* BHDS_SDK_H */
