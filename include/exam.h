#ifndef BHDS_EXAM_H
#define BHDS_EXAM_H

#include "types.h"
#include "scd_data.h"

/* A 2D sprite drawn through the 3D engine (see hud.c). 0x44 bytes. */
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

/* A texture reference loaded by func_02076bc8. */
typedef struct TexRef {
    /* 0x00 */ u8 slot;
    /* 0x01 */ u8 pal;
    /* 0x02 */ u8 unk02[6];
    /* 0x08 */ u32 unk08;
} TexRef;

/* A sprite with its own texture. 0x50 bytes. */
typedef struct ItemSprite {
    /* 0x00 */ HudSprite spr;
    /* 0x44 */ TexRef tex;
} ItemSprite;

/* Icon layout per item. */
typedef struct ItemIcon {
    /* 0x00 */ u16 color;
    /* 0x02 */ u16 unk02;
    /* 0x04 */ u8 unk04;
    /* 0x05 */ u8 x;
    /* 0x06 */ u8 y;
    /* 0x07 */ u8 unk07;
} ItemIcon;

/* A 2D sprite passed to func_020062ac (see choice.h). */
typedef struct ExamSprite {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u8 frame;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u16 unk04;
    /* 0x06 */ s16 x;
    /* 0x08 */ s16 y;
    /* 0x0A */ u16 unk0A;
} ExamSprite;

typedef struct ExamPos {
    /* 0x00 */ u8 x;
    /* 0x01 */ u8 y;
    /* 0x02 */ u8 unk02[2];
} ExamPos;

/* Scene work area (data_0213b214.work). */
typedef struct ExamWork {
    /* 0x00 */ u8 analog;        /* last input came from the touch screen */
    /* 0x01 */ u8 keys;          /* turn/zoom directions this frame */
    /* 0x02 */ u8 hideButtons;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u16 msg;
    /* 0x06 */ u16 shownMsg;
    /* 0x08 */ u32 msgArg;
    /* 0x0C */ u8 unk0C[4];
    /* 0x10 */ ItemSprite icon;
} ExamWork;

/* The model's rotation (inside data_0213b214). */
typedef struct UnkStruct_0213b414 {
    /* 0x00 */ u8 unk00[0x6E];
    /* 0x6E */ s16 rotX;
    /* 0x70 */ s16 rotY;
} UnkStruct_0213b414;

typedef struct ItemStackInfo {
    /* 0x00 */ u8 maxCount;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ s8 combineGroup;
    /* 0x03 */ u8 examine;       /* index into the examine tables */
} ItemStackInfo;

#define EXAM_WORK ((ExamWork *)data_0213b214.work)
#define INV_SLOTS ((u8 (*)[2])data_02147640.unk538)

#endif /* BHDS_EXAM_H */
