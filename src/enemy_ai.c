/*
 * enemy_ai.c - an enemy's chase behaviour: heading for the player through
 * the room's zones, switching range by distance and the action handlers.
 * JP (ABHJ) 0x020BAA80 - 0x020BAEB4.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "game.h"
#include "enemy.h"

/* Distances at which the range index changes (0: never). */
typedef struct EnemyRange {
    /* 0x00 */ u16 near;
    /* 0x02 */ u16 far;
} EnemyRange;

typedef struct EnemyRangeNext {
    /* 0x00 */ u8 near;
    /* 0x01 */ u8 far;
} EnemyRangeNext;

/* Not yet identified. JP addresses. */
extern u32 Sqrt(u32 x);  /* BIOS call stub, Thumb */
extern s16 func_0201de04(s32 x, s32 z);  /* angle from the object to (x, z) */
extern void func_0205516c(u32 id);
extern void func_020bb2e8(u32 anim);

extern EnemyRange data_02124228[];        /* JP 0x02124228 */
extern EnemyRangeNext data_02124220[];    /* JP 0x02124220 */


void Enemy_Act2(void)
{
    Enemy_UpdateTarget();
    if (data_027e000c->unk093 == 0) {
        data_027e000c->unk093 = 1;
        if (data_027e000c->state != 7) {
            data_027e000c->state = 7;
            data_027e000c->subState = 0;
            data_027e000c->unk0A3 = 0;
            data_027e000c->unk098 = 7;
            func_020bb2e8(0x8002);
        }
    }
    if ((u32)(ENEMY_WORK(data_027e000c)->targetRotY + 0x180 - data_027e000c->rotY) <= 0x300) {
        Enemy_TurnToTarget(0x30);
        func_0205516c(0x5D);
        Enemy_MoveForward(data_027e000c->speed);
    } else {
        Enemy_TurnToTarget(0x28);
    }
    Enemy_Wander(0);
}

void Enemy_Act1(void)
{
    EnemyObj *e;

    Enemy_UpdateTarget();
    e = data_027e000c;
    if ((u32)(ENEMY_WORK(e)->targetRotY + 0x180 - e->rotY) <= 0x300) {
        if (e->state != 8) {
            e->state = 8;
            data_027e000c->subState = 0;
            data_027e000c->unk0A3 = 0;
            data_027e000c->unk098 = 7;
            func_020bb2e8(0x8005);
        }
        Enemy_TurnToTarget(0x60);
        Enemy_MoveForward(0xD2);
    } else {
        if (e->state != 7) {
            e->state = 7;
            data_027e000c->subState = 0;
            data_027e000c->unk0A3 = 0;
            data_027e000c->unk098 = 7;
            func_020bb2e8(0x8002);
        }
        Enemy_TurnToTarget(0x30);
        if ((u32)(ENEMY_WORK(data_027e000c)->targetRotY + 0x200 - data_027e000c->rotY) <= 0x400) {
            func_0205516c(0x5D);
            Enemy_MoveForward(data_027e000c->speed);
        }
    }
    Enemy_StartWander(data_027e000c->destX, data_027e000c->destZ);
}

void Enemy_Act0(void)
{
    EnemyObj *e;

    ENEMY_WORK(data_027e000c)->targetRotY = func_0201de04(PLAYER_X, PLAYER_Z);
    e = data_027e000c;
    if ((u32)(ENEMY_WORK(e)->targetRotY + 0x200 - e->rotY) <= 0x400) {
        if (e->state != 3) {
            e->state = 3;
            data_027e000c->subState = 0;
            data_027e000c->unk0A3 = 0;
            data_027e000c->unk098 = 7;
            func_020bb2e8(0x8003);
        }
        Enemy_TurnToTarget(0x30);
        Enemy_MoveForward(-0x3C);
        Enemy_StartWander((s16)PLAYER_X, (s16)PLAYER_Z);
    } else {
        if (e->state != 7) {
            e->state = 7;
            data_027e000c->subState = 0;
            data_027e000c->unk0A3 = 0;
            data_027e000c->unk098 = 7;
            func_020bb2e8(0x8002);
        }
        ENEMY_WORK(data_027e000c)->targetRotY += 0x800;
        ENEMY_WORK(data_027e000c)->targetRotY &= 0xFFF;
        Enemy_TurnToTarget(0x30);
        func_0205516c(0x5D);
        Enemy_MoveForward(data_027e000c->speed);
        Enemy_StartIdle();
    }
}

void Enemy_Reset(void)
{
    data_027e000c->unk091 = 1;
    Enemy_StartIdle();
}

/* Move to the next range when the player comes nearer or goes farther. */
void Enemy_UpdateRange(void)
{
    s32 dz = data_027e000c->posZ - PLAYER_Z;
    s32 dx = data_027e000c->posX - PLAYER_X;
    u32 dist = Sqrt(dx * dx + dz * dz);
    EnemyObj *e = data_027e000c;
    u8 range = e->range;

    if (data_02124228[range].near != 0 && dist < data_02124228[range].near) {
        *(u16 *)&e->range = data_02124220[range].near;
    } else if (data_02124228[range].far != 0 && data_02124228[range].far <= dist) {
        *(u16 *)&e->range = data_02124220[range].far;
    }
}
