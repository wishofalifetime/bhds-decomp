#ifndef BHDS_CHOICE_H
#define BHDS_CHOICE_H

#include "types.h"
#include "scd_data.h"

/* One side of the screen. 0x1C bytes. */
typedef struct ChoiceSide {
    /* 0x00 */ s32 x;            /* 20.12 fixed point */
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 vx;
    /* 0x0C */ s32 vy;
    /* 0x10 */ u8 unk10[8];
    /* 0x18 */ u8 count;
    /* 0x19 */ u8 kind;
    /* 0x1A */ u8 unk1A[2];
} ChoiceSide;

/* Scene work area (data_0213b214.work). */
typedef struct ChoiceWork {
    /* 0x00 */ u8 cur;
    /* 0x01 */ u8 unk01[8];
    /* 0x09 */ u8 blink[2];      /* 1: the button blinks */
    /* 0x0B */ u8 unk0B;
    /* 0x0C */ ChoiceSide side[2];
    /* 0x44 */ u8 unk44;
} ChoiceWork;

/* A sprite passed to func_020062ac. */
typedef struct ChoiceSprite {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u8 frame;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u16 unk04;
    /* 0x06 */ s16 x;
    /* 0x08 */ s16 y;
    /* 0x0A */ u16 unk0A;
} ChoiceSprite;

typedef struct ChoicePos {
    /* 0x00 */ u8 x;
    /* 0x01 */ u8 y;
    /* 0x02 */ u8 unk02[2];
} ChoicePos;

#define CHOICE_WORK ((ChoiceWork *)data_0213b214.work)

void Choice_UpdateCursor(void);
BOOL Choice_Blink(void);

#endif /* BHDS_CHOICE_H */
