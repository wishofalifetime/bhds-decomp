/*
 * g3d_mat.c - 3D material setup (texture, palette, polygon attributes and
 * colors written to the geometry engine) and related helpers.
 * JP (ABHJ) 0x020B8324 - 0x020B8778.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "hw_regs.h"
#include "g3.h"

/* VRAM slot of a loaded texture or palette. 0x14 bytes. */
typedef struct G3dVramEntry {
    /* 0x00 */ u32 addr;
    /* 0x04 */ u8 unk04[0x10];
} G3dVramEntry;

typedef struct G3dTexRef {
    /* 0x00 */ u8 tex;          /* index into data_0213bf20 */
    /* 0x01 */ u8 pltt;         /* index into data_0213ba20 */
    /* 0x02 */ u8 pad02[2];
    /* 0x04 */ u32 sizeS;
    /* 0x08 */ u32 sizeT;
} G3dTexRef;

/* A flat material: color, alpha and texture. */
typedef struct G3dMaterial {
    /* 0x00 */ u8 unk00[0x14];
    /* 0x14 */ u16 color;
    /* 0x16 */ u8 unk16[0x24 - 0x16];
    /* 0x24 */ u8 polyId;
    /* 0x25 */ u8 palette;
    /* 0x26 */ u8 format;
    /* 0x27 */ u8 alpha;
    /* 0x28 */ G3dTexRef *tex;
    /* 0x2C */ u32 texAddr;
    /* 0x30 */ u32 plttAddr;
    /* 0x34 */ u32 sizeS;
    /* 0x38 */ u32 sizeT;
} G3dMaterial;

/* A model with its own texture slots. */
typedef struct G3dModel {
    /* 0x00 */ u8 visible;
    /* 0x01 */ u8 unk01[0xE - 0x1];
    /* 0x0E */ u16 flags;       /* bit 14: use data_0212377c[bits 12-13] as polygon id */
    /* 0x10 */ u8 unk10[0x26 - 0x10];
    /* 0x26 */ u8 kind;
    /* 0x27 */ u8 variant;
    /* 0x28 */ u8 unk28[0xAC - 0x28];
    /* 0xAC */ u8 tex;
    /* 0xAD */ u8 pltt;
    /* 0xAE */ u8 padAE[2];
    /* 0xB0 */ u32 sizeS;
    /* 0xB4 */ u32 sizeT;
} G3dModel;

typedef struct UnkDtcm_027e01ac {
    /* 0x00 */ u8 unk00[0x22];
    /* 0x22 */ s16 unk22;
} UnkDtcm_027e01ac;

typedef struct UnkDtcm_027e01bc {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u16 unk04;
    /* 0x06 */ u16 size;
    /* 0x08 */ u8 dirty;
    /* 0x09 */ u8 unk09[3];
    /* 0x0C */ void *src;
} UnkDtcm_027e01bc;

/* Not yet identified. JP addresses. */
extern void func_01ffcab4(G3dModel *model);  /* ITCM */
extern u8 func_02076974(u32 a);
extern u8 func_02076bc8(u32 a, u8 *out);
extern void func_02105aac(u16 value, void *dst, u32 size);  /* probably MI_CpuFill16 */

extern G3dVramEntry data_0213bf20[];         /* JP 0x0213BF20: textures */
extern G3dVramEntry data_0213ba20[];         /* JP 0x0213BA20: palettes */
extern u32 data_0212377c[];                  /* JP 0x0212377C */
extern u8 data_0213ec68[];                   /* JP 0x0213EC68: 2 x 0x800 bytes */
extern u32 data_0213a67c;                    /* JP 0x0213A67C */
extern UnkDtcm_027e01ac data_027e01ac;       /* DTCM */
extern UnkDtcm_027e01bc data_027e01bc;       /* DTCM */
extern UnkDtcm_027e01bc data_027e01fc;       /* DTCM */

u32 G3d_GetPolyAlpha(u32 kind, u32 variant);


/* Clear count 0x40-byte rows of map buffer which (wrapping at 32), then mark it for upload. */
void G3d_ClearRows(u32 which, u8 row, u8 count)
{
    UnkDtcm_027e01bc *t;
    u8 *base;

    if (which == 0) {
        t = &data_027e01bc;
    } else {
        t = &data_027e01fc;
    }
    base = data_0213ec68 + which * 0x800;
    while (count != 0) {
        volatile u16 zero = 0;

        func_02105aac(zero, base + row * 0x40, 0x40);
        row = (row + 1) & 0x1F;
        count--;
    }
    t->unk04 = 0;
    t->src = base;
    t->size = 0x800;
    t->dirty = 1;
}

void G3d_GetPair(u32 a, u32 b, u8 *out)
{
    out[0] = func_02076bc8(a, out);
    out[1] = func_02076974(b);
}

/* Send a model's texture, palette, polygon attributes and colors. */
void G3d_SetModelMaterial(G3dModel *m)
{
    u8 polyId;

    if (m->visible) {
        REG_TEXIMAGE_PARAM = (data_0213bf20[m->tex].addr & 0xFFFF) | 0x4C000000 |
                             (m->sizeS << 20) | (m->sizeT << 23) | 0x20000000;
        G3_TexPlttBase(((data_0213ba20[m->pltt].addr & 0xFFFF) << 3) + (m->variant >> 3) * 0x20, 3);
        if (m->flags & 0x4000) {
            polyId = data_0212377c[(m->flags >> 12) & 3];
        } else {
            polyId = 0x1F;
        }
        G3_PolygonAttr(0, 0, 3, G3d_GetPolyAlpha(m->kind, m->variant), polyId, 0x800);
        G3_MaterialColorDiffAmb(0x7FFF, 0x7FFF, TRUE);
        G3_MaterialColorSpecEmi(0, 0x7FFF, TRUE);
        func_01ffcab4(m);
    }
}

u32 G3d_GetPolyAlpha(u32 kind, u32 variant)
{
    if (kind == 0x1B || kind == 0x13) {
        return 2;
    }
    if (kind == 9 && variant == 3) {
        return 3;
    }
    return 1;
}

/* Bit number of the single set bit in v (0 if none below 16). */
u8 G3d_BitIndex(u32 v)
{
    u8 i;

    for (i = 0; i < 0x10; i++) {
        if (1 << i == v) {
            return i;
        }
    }
    return 0;
}

/* Material whose texture and palette come from the VRAM slot tables. */
void G3d_SetMaterialSlots(G3dMaterial *m)
{
    if (m->tex != NULL) {
        REG_TEXIMAGE_PARAM = (m->format << 26) | (data_0213bf20[m->tex->tex].addr & 0xFFFF) | 0x40000000 |
                             (m->tex->sizeS << 20) | (m->tex->sizeT << 23) | 0x20000000;
        G3_TexPlttBase(((data_0213ba20[m->tex->pltt].addr & 0xFFFF) << 3) + m->palette * 0x20, m->format);
    } else {
        REG_TEXIMAGE_PARAM = 0x20000000;
        REG_PLTT_BASE = 0;
    }
    G3_PolygonAttr(0, 0, 3, m->alpha, m->polyId, 0x800);
    G3_MaterialColorDiffAmb(m->color, m->color, TRUE);
}

void G3d_SetUnk22(s32 v)
{
    if (v <= 0) {
        data_027e01ac.unk22 = v;
        data_0213a67c = v << 5;
    } else {
        data_027e01ac.unk22 = v;
        data_0213a67c = (v - 1) << 5;
    }
}

/* Material with a direct texture address. */
void G3d_SetMaterial(G3dMaterial *m)
{
    REG_TEXIMAGE_PARAM = (m->format << 26) | (m->texAddr & 0xFFFF) | 0x40000000 |
                         (m->sizeS << 20) | (m->sizeT << 23) | 0x20000000;
    G3_TexPlttBase((m->plttAddr & 0xFFFF) << 3, m->format);
    G3_PolygonAttr(0, 0, 2, m->alpha, m->polyId, 0);
    G3_MaterialColorDiffAmb(m->color, m->color, TRUE);
}
