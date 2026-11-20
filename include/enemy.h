#ifndef BHDS_ENEMY_H
#define BHDS_ENEMY_H

#include "types.h"

/* The object fields used here. Overlaps Obj (obj.h), which is still partial. */
typedef struct EnemyObj {
    /* 0x000 */ u8 unk000[9];
    /* 0x009 */ u8 kind;
    /* 0x00A */ u8 unk00A[0x30 - 0xA];
    /* 0x030 */ s32 posX;
    /* 0x034 */ s32 posY;
    /* 0x038 */ s32 posZ;
    /* 0x03C */ u8 unk03C[0x44 - 0x3C];
    /* 0x044 */ s16 rotY;
    /* 0x046 */ u8 unk046[0xA1 - 0x46];
    /* 0x0A1 */ u8 state;
    /* 0x0A2 */ u8 subState;
    /* 0x0A3 */ u8 unk0A3[0xB0 - 0xA3];
    /* 0x0B0 */ s32 unkB0;
    /* 0x0B4 */ s32 unkB4;
    /* 0x0B8 */ s32 unkB8;
    /* 0x0BC */ u8 anim;
    /* 0x0BD */ u8 unkBD;
    /* 0x0BE */ u8 unkBE;
    /* 0x0BF */ u8 unk0BF[0x17C - 0xBF];
    /* 0x17C */ u8 unk17C[3];
    /* 0x17F */ u8 timer;
    /* 0x180 */ u8 unk180;
    /* 0x181 */ u8 unk181;
    /* 0x182 */ u8 unk182;
    /* 0x183 */ u8 unk183;
    /* 0x184 */ u8 zoneA;
    /* 0x185 */ u8 zoneB;
    /* 0x186 */ s16 targetRotY;
} EnemyObj;

extern EnemyObj *data_027e000c;           /* DTCM: current object */

#endif /* BHDS_ENEMY_H */
