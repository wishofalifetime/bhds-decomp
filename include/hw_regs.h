#ifndef BHDS_HW_REGS_H
#define BHDS_HW_REGS_H

#include "types.h"

/*
 * DS ARM9 I/O registers used by the game. Names and bit meanings follow
 * GBATEK (problemkaputt.de/gbatek-nds-reference.htm). Engine B registers
 * mirror engine A at +0x1000 and use the DB_ prefix.
 */

/* --- 2D engine A --- */
#define REG_DISPCNT         (*(vu32 *)0x04000000)
#define REG_BG1HOFS         (*(vu16 *)0x04000014)
#define REG_BLDCNT          (*(vu16 *)0x04000050)
#define REG_MASTER_BRIGHT   (*(vu16 *)0x0400006C)

/* --- 2D engine B --- */
#define REG_DB_DISPCNT        (*(vu32 *)0x04001000)
#define REG_DB_BG1HOFS        (*(vu16 *)0x04001014)
#define REG_DB_BLDCNT         (*(vu16 *)0x04001050)
#define REG_DB_MASTER_BRIGHT  (*(vu16 *)0x0400106C)

/* DISPCNT bits 16-17: display mode. 0 = off (white), 1 = graphics,
 * 2 = VRAM, 3 = main memory (modes 2 and 3 are engine A only). */
#define DISPCNT_MODE_SHIFT     16
#define DISPCNT_MODE_MASK      0x00030000
#define DISPCNT_MODE_GRAPHICS  0x00010000

/* MASTER_BRIGHT: bits 0-4 factor (0-16), bits 14-15 mode (1 up, 2 down) */
#define MASTER_BRIGHT_FACTOR_MASK  0x001F
#define MASTER_BRIGHT_MODE_UP      0x4000
#define MASTER_BRIGHT_MODE_DOWN    0x8000

/* --- 3D display --- */
#define REG_DISP3DCNT       (*(vu16 *)0x04000060)
#define DISP3DCNT_RDLINES_UNDERFLOW  0x1000  /* bit 12, write 1 to acknowledge */
#define DISP3DCNT_RAM_OVERFLOW       0x2000  /* bit 13, write 1 to acknowledge */

/* --- Interrupts --- */
#define REG_IME             (*(vu16 *)0x04000208)  /* bit 0: master enable */

/* --- Keypad --- */
/* KEYINPUT: 0 = pressed. Bits: A, B, Select, Start, Right, Left, Up, Down, R, L */
#define REG_KEYINPUT        (*(vu16 *)0x04000130)

/* --- Power --- */
#define REG_POWCNT1         (*(vu16 *)0x04000304)
#define POWCNT1_2D_A        0x0002
#define POWCNT1_3D_RENDER   0x0004
#define POWCNT1_3D_GEOMETRY 0x0008
#define POWCNT1_2D_B        0x0200
#define POWCNT1_ALL_ENGINES (POWCNT1_2D_A | POWCNT1_3D_RENDER | POWCNT1_3D_GEOMETRY | POWCNT1_2D_B)

/* --- 3D geometry command ports --- */
#define REG_POLYGON_ATTR    (*(vu32 *)0x040004A4)  /* cmd 0x29 */
#define REG_TEXIMAGE_PARAM  (*(vu32 *)0x040004A8)  /* cmd 0x2A */
#define REG_PLTT_BASE       (*(vu32 *)0x040004AC)  /* cmd 0x2B */
#define REG_DIF_AMB         (*(vu32 *)0x040004C0)  /* cmd 0x30 */
#define REG_SPE_EMI         (*(vu32 *)0x040004C4)  /* cmd 0x31 */
#define REG_SWAP_BUFFERS    (*(vu32 *)0x04000540)  /* cmd 0x50 */
#define SWAP_BUFFERS_MANUAL_SORT  0x1  /* bit 0: translucent Y-sort manual */
#define SWAP_BUFFERS_W_DEPTH      0x2  /* bit 1: W-buffering instead of Z */

#define REG_GXSTAT          (*(vu32 *)0x04000600)
#define GXSTAT_MTX_STACK_ERROR  0x00008000  /* bit 15, write 1 to clear */
#define GXSTAT_GEOMETRY_BUSY    0x08000000  /* bit 27 */

#endif /* BHDS_HW_REGS_H */
