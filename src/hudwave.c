/*
 * hudwave.c - the status screen's animated wave: columns of small
 * sprites whose height and brightness follow a curve over time.
 * JP (ABHJ) 0x020B8DF4 - 0x020B8F9C. Not matching yet (83/106): the loop-invariant
 * constants land in different registers.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"

/* A 2D sprite drawn through the 3D engine (see g3d_mat.c). 0x44 bytes. */
typedef struct HudSprite {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 width;
    /* 0x06 */ s16 unk06;
    /* 0x08 */ s16 unk08;
    /* 0x0A */ s16 unk0A;
    /* 0x0C */ s16 unk0C;
    /* 0x0E */ s16 unk0E;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 unk12[2];
    /* 0x14 */ u16 color;
    /* 0x16 */ u8 unk16[2];
    /* 0x18 */ s32 scaleX;
    /* 0x1C */ s32 scaleY;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ u8 alpha;
    /* 0x25 */ u8 palette;
    /* 0x26 */ u8 format;
    /* 0x27 */ u8 polyId;
    /* 0x28 */ void *tex;
    /* 0x2C */ u8 unk2C[0x44 - 0x2C];
} HudSprite;

typedef struct UnkStruct_0213a680 {
    /* 0x00 */ u8 unk00[0x18];
    /* 0x18 */ u16 timer;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 colorIndex;
    /* 0x1D */ u8 x;
    /* 0x1E */ u8 y;
} UnkStruct_0213a680;

/* Not yet identified. JP addresses. */
extern void func_02105c14(void *dst, u32 value, u32 size);  /* probably MI_CpuFill8 */
extern void Hud_DrawSprite(HudSprite *sprite);

extern UnkStruct_0213a680 data_0213a680;  /* JP 0x0213A680 */
extern u16 data_02123fa0[];               /* JP 0x02123FA0: colors */

/* Draw the wave: up to 33 columns, skipping the first four, brightest at the left. */
void Hud_DrawWave(void)
{
    s16 n;
    s16 i;
    u8 alpha;
    u16 skip = 4;
    HudSprite spr;

    n = (u16)(4.0 * data_0213a680.timer) % 0x51;
    for (i = 0; n != 0 && i < 0x21; n--, i++) {
        if (n >= 0x30) {
            if (skip != 0) {
                skip--;
            }
            continue;
        }
        if (skip != 0) {
            skip--;
            alpha = 0x1D;
        } else {
            alpha = 0x1D - (u8)(29.0 * (i - 0.2 * 24) / (1.2 * 24));
        }
        func_02105c14(&spr, 0, sizeof(spr));
        spr.alpha = alpha;
        spr.format = 3;
        spr.width = 0x10;
        spr.y = data_0213a680.y + 0x18 - n;
        spr.unk0E = 0x40;
        spr.unk10 = 1;
        spr.scaleX = 0x1000;
        spr.scaleY = 0x1000;
        spr.x = data_0213a680.x;
        spr.color = data_02123fa0[data_0213a680.colorIndex];
        Hud_DrawSprite(&spr);
    }
}
