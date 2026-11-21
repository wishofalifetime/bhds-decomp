#ifndef BHDS_ENEMY_H
#define BHDS_ENEMY_H

#include "types.h"
#include "game.h"

/* Movement work area of an enemy (EnemyObj.work). */
typedef struct EnemyWork {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u8 count;
    /* 0x03 */ u8 timer;
    /* 0x04 */ u8 unk04;
    /* 0x05 */ u8 unk05;
    /* 0x06 */ u8 unk06;
    /* 0x07 */ u8 unk07;
    /* 0x08 */ u8 zoneA;
    /* 0x09 */ u8 zoneB;
    /* 0x0A */ s16 targetRotY;
} EnemyWork;

/* Animation state reached through the object's model. */
typedef struct EnemyAnimData {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u16 frameCount;
} EnemyAnimData;

typedef struct EnemyAnim {
    /* 0x00 */ s32 frame;        /* 20.12 fixed point */
    /* 0x04 */ u32 unk04;
    /* 0x08 */ EnemyAnimData *data;
} EnemyAnim;

/* Points at an entry of data_0213fc68. */
typedef struct EnemySlotRef {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 index;
} EnemySlotRef;

typedef struct EnemyModel {
    /* 0x00 */ u8 unk00[0x10];
    /* 0x10 */ EnemyAnim *anim;
    /* 0x14 */ EnemySlotRef *slot;
} EnemyModel;

typedef struct EnemyVec {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
} EnemyVec;

/* A 0x58-byte entry of the table at data_0213fc68. */
typedef struct EnemySlot {
    /* 0x00 */ u32 flags;
    /* 0x04 */ EnemyVec vec;
    /* 0x10 */ u8 unk10[0x58 - 0x10];
} EnemySlot;

typedef struct EnemyRes {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ void *data;
} EnemyRes;

/* Graphics of an enemy (EnemyObj.gfx). */
typedef struct EnemyGfx {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ EnemyRes *res;
    /* 0x08 */ u8 unk08[4];
    /* 0x0C */ void *tex;
    /* 0x10 */ u8 unk10[4];
    /* 0x14 */ EnemySlotRef *slot;
    /* 0x18 */ u8 unk18[0x20 - 0x18];
    /* 0x20 */ u8 unk20[0x54 - 0x20];
    /* 0x54 */ EnemySlot *slotPtr;
} EnemyGfx;

/* The object fields used here. Overlaps Obj (obj.h), which is still partial. */
typedef struct EnemyObj {
    /* 0x000 */ EnemyModel *model;
    /* 0x004 */ u16 animId;
    /* 0x006 */ u8 unk006;
    /* 0x007 */ s8 unk007;
    /* 0x008 */ u8 unk008;
    /* 0x009 */ u8 kind;
    /* 0x00A */ u8 variant;
    /* 0x00B */ u8 unk00B;
    /* 0x00C */ u16 *unk00C;
    /* 0x010 */ u8 unk010[0x30 - 0x10];
    /* 0x030 */ s32 posX;
    /* 0x034 */ s32 posY;
    /* 0x038 */ s32 posZ;
    /* 0x03C */ u8 unk03C[0x44 - 0x3C];
    /* 0x044 */ s16 rotY;
    /* 0x046 */ u8 unk046[0x54 - 0x46];
    /* 0x054 */ u8 unk054[0x80 - 0x54];
    /* 0x080 */ s16 unk080;
    /* 0x082 */ u8 unk082;
    /* 0x083 */ u8 unk083;
    /* 0x084 */ u8 unk084;
    /* 0x085 */ u8 unk085[0x91 - 0x85];
    /* 0x091 */ u8 unk091;
    /* 0x092 */ u8 range;        /* index into the range tables */
    /* 0x093 */ u8 unk093;
    /* 0x094 */ s16 unk094;
    /* 0x096 */ u8 unk096;
    /* 0x097 */ u8 unk097;
    /* 0x098 */ u8 unk098;
    /* 0x099 */ u8 unk099[0xA0 - 0x99];
    /* 0x0A0 */ u8 unk0A0;
    /* 0x0A1 */ u8 state;
    /* 0x0A2 */ u8 subState;
    /* 0x0A3 */ u8 unk0A3;
    /* 0x0A4 */ u8 unk0A4[2];
    /* 0x0A6 */ s16 speed;
    /* 0x0A8 */ s16 timer;
    /* 0x0AA */ u8 unk0AA[0xB0 - 0xAA];
    /* 0x0B0 */ s32 unkB0;
    /* 0x0B4 */ s32 unkB4;
    /* 0x0B8 */ s32 unkB8;
    /* 0x0BC */ u8 anim;
    /* 0x0BD */ u8 unkBD;
    /* 0x0BE */ u8 unkBE;
    /* 0x0BF */ u8 unk0BF[0xC1 - 0xBF];
    /* 0x0C1 */ s8 unk0C1;
    /* 0x0C2 */ u8 unk0C2[0xC6 - 0xC2];
    /* 0x0C6 */ s16 unk0C6;
    /* 0x0C8 */ u8 unk0C8[0x168 - 0xC8];
    /* 0x168 */ u8 unk168;
    /* 0x169 */ u8 unk169[0x176 - 0x169];
    /* 0x176 */ s16 destX;
    /* 0x178 */ s16 destZ;
    /* 0x17A */ u8 unk17A[2];
    /* 0x17C */ u8 work[0xC];  /* EnemyWork, see ENEMY_WORK */
    /* 0x188 */ EnemyGfx *gfx;
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
void Enemy_CheckState(void);
void Enemy_UpdateRange(void);
void Enemy_Reset(void);
void Enemy_SetAnim(u32 anim);
void Enemy_Init(u32 arg);
u8 Enemy_Animate(u8 mode, s16 speed);

#endif /* BHDS_ENEMY_H */
