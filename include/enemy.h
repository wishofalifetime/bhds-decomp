#ifndef BHDS_ENEMY_H
#define BHDS_ENEMY_H

#include "types.h"
#include "game.h"

/* Movement work area of an enemy (EnemyObj.work). */
typedef struct EnemyWork {
    /* 0x00 */ u8 unk00[3];
    /* 0x03 */ u8 timer;
    /* 0x04 */ u8 unk04;
    /* 0x05 */ u8 unk05;
    /* 0x06 */ u8 unk06;
    /* 0x07 */ u8 unk07;
    /* 0x08 */ u8 zoneA;
    /* 0x09 */ u8 zoneB;
    /* 0x0A */ s16 targetRotY;
} EnemyWork;

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
    /* 0x046 */ u8 unk046[0x91 - 0x46];
    /* 0x091 */ u8 unk091;
    /* 0x092 */ u8 range;        /* index into the range tables */
    /* 0x093 */ u8 unk093;
    /* 0x094 */ u8 unk094[0x98 - 0x94];
    /* 0x098 */ u8 unk098;
    /* 0x099 */ u8 unk099[0xA1 - 0x99];
    /* 0x0A1 */ u8 state;
    /* 0x0A2 */ u8 subState;
    /* 0x0A3 */ u8 unk0A3;
    /* 0x0A4 */ u8 unk0A4[2];
    /* 0x0A6 */ s16 speed;
    /* 0x0A8 */ u8 unk0A8[0xB0 - 0xA8];
    /* 0x0B0 */ s32 unkB0;
    /* 0x0B4 */ s32 unkB4;
    /* 0x0B8 */ s32 unkB8;
    /* 0x0BC */ u8 anim;
    /* 0x0BD */ u8 unkBD;
    /* 0x0BE */ u8 unkBE;
    /* 0x0BF */ u8 unk0BF[0x176 - 0xBF];
    /* 0x176 */ s16 destX;
    /* 0x178 */ s16 destZ;
    /* 0x17A */ u8 unk17A[2];
    /* 0x17C */ u8 work[0xC];  /* EnemyWork, see ENEMY_WORK */
} EnemyObj;

#define ENEMY_WORK(e) ((EnemyWork *)(e)->work)

/* The player's position. */
#define PLAYER_X data_02143640.unkF74[0]
#define PLAYER_Z data_02143640.unkF74[2]

extern EnemyObj *data_027e000c;           /* DTCM: current object */

void Enemy_StartWander(s32 x, s32 z);
void Enemy_StartIdle(void);
void Enemy_Wander(u32 arg);
void Enemy_MoveForward(s16 speed);
void Enemy_TurnToTarget(s32 step);
u8 Enemy_InZoneOverlap(s32 axis, s16 x, s16 z);

#endif /* BHDS_ENEMY_H */
