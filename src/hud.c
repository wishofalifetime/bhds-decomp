/*
 * hud.c - status screen display: frame sprites, mode and wave parameters.
 * JP (ABHJ) 0x020B9534 - 0x020B9988.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "game.h"
#include "scd_data.h"
#include "mathrand.h"

/* A 2D sprite drawn through the 3D engine (see hudwave.c). 0x44 bytes. */
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

typedef struct HudState {
    /* 0x00 */ u8 tex[0xC];          /* texture reference for the frame */
    /* 0x0C */ u8 tex2[0xC];
    /* 0x18 */ u16 timer;
    /* 0x1A */ u8 mode;
    /* 0x1B */ s8 unk1B;
    /* 0x1C */ u8 colorIndex;
    /* 0x1D */ u8 x;
    /* 0x1E */ u8 y;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ u8 unk20;
    /* 0x21 */ s8 unk21;
} HudState;

/* Wave parameters passed to Hud_SetWave. */
typedef struct HudWaveParam {
    /* 0x00 */ u8 x;
    /* 0x01 */ u8 y;
    /* 0x02 */ u8 unk02;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ s8 unk04;
} HudWaveParam;

/* Not yet identified. JP addresses. */
extern void func_02105c14(void *dst, u32 value, u32 size);  /* probably MI_CpuFill8 */
extern void func_020767b0(u8 *slot);
extern void func_020769b4(u8 *slot);
extern u8 func_02076974(u32 id);
extern u8 func_02076bc8(u32 id, void *ref);
extern void Hud_DrawSprite(HudSprite *sprite);

extern HudState data_0213a680;            /* JP 0x0213A680 */
extern VoidFunc data_02123f8c[];          /* JP 0x02123F8C: by mode */
extern u8 data_02123f94[];                /* JP 0x02123F94 */
extern u8 data_02123f7c[];                /* JP 0x02123F7C: frame texture by data_0213a6cc.unk03 */

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[3];
    /* 0x03 */ u8 unk03;
} UnkStruct_0213a6cc;

extern UnkStruct_0213a6cc data_0213a6cc;  /* JP 0x0213A6CC */

void Hud_SetWave(HudWaveParam *p);
u32 Hud_GetCondition(void);
void Hud_Reset(void);

/* Load the HUD textures. */
void Hud_Init(void)
{
    u8 id = data_02123f7c[data_0213a6cc.unk03];

    data_02143640.unk0E = 1;
    data_0213a680.tex2[0] = func_02076bc8(id, data_0213a680.tex2);
    data_0213a680.tex2[1] = func_02076974(id);
    data_0213a680.tex[0] = func_02076bc8(0x2B, data_0213a680.tex);
    data_0213a680.tex[1] = func_02076974(0x2B);
    data_02143640.unk0E = 0;
    Hud_Reset();
}

void Hud_Reset(void)
{
    data_0213a680.unk1F = 0;
    data_0213a680.unk20 = 0;
    data_0213a680.timer = 0;
    data_0213a680.mode = 0;
}

void Hud_FreeTextures(void)
{
    func_020769b4(&data_0213a680.tex[0]);
    func_020767b0(&data_0213a680.tex[1]);
    func_020769b4(&data_0213a680.tex2[0]);
    func_020767b0(&data_0213a680.tex2[1]);
}

/* Condition from 0 (fine) to 3 (danger) by remaining health; bit 7: poisoned. */
u32 Hud_GetCondition(void)
{
    s32 c = 3 - data_02144540.unkD8 / (data_02144640.maxHealth / 4);
    u8 cond;

    if (c < 0) {
        c = 0;
    }
    if (c >= 4) {
        c = 3;
    }
    cond = c;
    if (data_02144640.unk04 & 0x22) {
        cond |= 0x80;
    }
    return cond;
}

/* Pick new wave parameters from the current condition, with some randomness. */
void Hud_UpdateWave(void)
{
    s32 cond = Hud_GetCondition();
    s32 kind = cond & 3;
    HudWaveParam p;

    p.unk02 = kind;
    p.unk03 = kind % 6;
    p.unk04 = data_02123f94[kind];
    p.x = 0xE0;
    p.y = 0x3A;
    if ((u8)(cond / 0x80)) {
        if (MATH_Rand32(&data_027e0064, 0) & 2) {
            p.unk02 = (MATH_Rand32(&data_027e0064, 0) & 1) + 5;
        }
        p.unk04 = 3;
    }
    Hud_SetWave(&p);
}

void Hud_SetWave(HudWaveParam *p)
{
    if ((u16)(2.6 * data_0213a680.timer) % 0x73 < 3) {
        data_0213a680.unk1F = p->unk02;
        data_0213a680.unk20 = p->unk03;
    }
    data_0213a680.unk21 = p->unk04;
    data_0213a680.unk1B = p->unk04;
    data_0213a680.x = p->x;
    data_0213a680.y = p->y;
    data_0213a680.timer++;
    if (data_0213a680.mode == 1 && data_0213a680.timer == 0x14) {
        Hud_Reset();
    }
}

void Hud_SetMode(u32 kind)
{
    data_0213a680.mode = 1;
    data_0213a680.timer = 0;
    switch (kind) {
    case 1:
        data_0213a680.colorIndex = 0;
        break;
    case 2:
        data_0213a680.colorIndex = 5;
        break;
    case 3:
        data_0213a680.colorIndex = 6;
        break;
    }
}

/* Draw the frame and its background, then run the mode handler. */
void Hud_Draw(void)
{
    HudSprite spr;

    func_02105c14(&spr, 0, sizeof(spr));
    spr.x = data_0213a680.x;
    spr.y = data_0213a680.y;
    spr.width = 0x20;
    spr.unk0A = 8;
    spr.unk0E = 0x40;
    spr.unk0C = 0x30;
    spr.unk10 = 0x30;
    spr.color = 0x7FFF;
    spr.scaleX = 0x1000;
    spr.scaleY = 0x1000;
    spr.alpha = 0x1F;
    spr.format = 3;
    spr.tex = &data_0213a680;
    Hud_DrawSprite(&spr);

    func_02105c14(&spr, 0, sizeof(spr));
    spr.x = data_0213a680.x;
    spr.y = data_0213a680.y;
    spr.width = 0x20;
    spr.unk0E = 0x44;
    spr.unk10 = 0x34;
    spr.color = 0x6318;
    spr.scaleX = 0x1000;
    spr.scaleY = 0x1000;
    spr.alpha = 0x1F;
    spr.format = 3;
    spr.tex = NULL;
    Hud_DrawSprite(&spr);

    data_02123f8c[data_0213a680.mode]();
}
