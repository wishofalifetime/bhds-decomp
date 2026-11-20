#ifndef BHDS_OBJ_H
#define BHDS_OBJ_H

#include "types.h"

/*
 * Scene object created by room script opcode 0x1F.
 * Only the fields written so far are known; names are placeholders.
 */
typedef struct ObjModel {
    /* 0x00 */ u8 unk00[0x0C];
    /* 0x0C */ void *unk0C;
} ObjModel;

typedef struct Obj {
    /* 0x00 */ ObjModel *model;
    /* 0x04 */ u8 unk04[3];
    /* 0x07 */ u8 unk07;
    /* 0x08 */ u8 unk08;
    /* 0x09 */ u8 kind;          /* script id & 0x7F */
    /* 0x0A */ u8 unk0A;
    /* 0x0B */ u8 unk0B;
    /* 0x0C */ u16 *unk0C;       /* points at unk94 */
    /* 0x10 */ u8 unk10[4];
    /* 0x14 */ u32 flags;
    /* 0x18 */ u8 unk18[0x30 - 0x18];
    /* 0x30 */ s32 posX;
    /* 0x34 */ s32 posY;
    /* 0x38 */ s32 posZ;
    /* 0x3C */ s16 initX;
    /* 0x3E */ s16 initY;
    /* 0x40 */ s16 initZ;
    /* 0x42 */ s16 unk42;
    /* 0x44 */ s16 unk44;
    /* 0x46 */ s16 unk46;
    /* 0x48 */ u8 unk48[0x4E - 0x48];
    /* 0x4E */ s16 unk4E;
    /* 0x50 */ s16 unk50;
    /* 0x52 */ u8 unk52;         /* 0-31, set/adjusted by opcode 0x34 */
    /* 0x53 */ u8 unk53[0x92 - 0x53];
    /* 0x92 */ u16 unk92;        /* compared by opcode 0x36 */
    /* 0x94 */ u16 unk94;
    /* 0x96 */ u16 unk96;
    /* 0x98 */ u16 unk98;
    /* 0x9A */ u16 unk9A;
    /* 0x9C */ u16 unk9C;
    /* 0x9E */ u16 unk9E;
    /* 0xA0 */ s16 unkA0;
    /* 0xA2 */ u8 unkA2[2];
    /* 0xA4 */ s16 unkA4;
    /* 0xA6 */ u8 unkA6[2];
    /* 0xA8 */ s16 unkA8;
    /* 0xAA */ u8 unkAA[2];
    /* 0xAC */ s16 unkAC;
    /* 0xAE */ u8 unkAE[2];
    /* 0xB0 */ void *parent;
    /* 0xB4 */ u8 unkB4[0xEC - 0xB4];
    /* 0xEC */ u16 unkEC;        /* bits 2-3 cleared by opcode 0x21 */
} Obj;

#endif /* BHDS_OBJ_H */
