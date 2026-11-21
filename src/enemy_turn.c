/*
 * enemy_turn.c - turning the current object towards a target angle,
 * testing a coordinate against the overlap of its two zones and picking
 * the point to head for.
 * JP (ABHJ) 0x020BA670 - 0x020BAA80. Not matching yet: Enemy_TurnToTarget
 * (26/34), Enemy_InZoneOverlap (75/97) and Enemy_UpdateTarget (125/129)
 * differ only in register choice.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "game.h"
#include "scd_data.h"
#include "enemy.h"

/* Not yet identified. JP addresses. */
extern BOOL func_02073cd8(s16 x, s16 z);
extern s16 func_0201de04(s32 x, s32 z);  /* angle from the object to (x, z) */
extern BOOL func_02073d14(u8 zone);
extern s16 func_02074004(s32 x, s32 z, s8 zoneB, s8 zoneA);
extern u8 func_020748d4(s32 x, s32 z, s16 *destX, s16 *destZ);

extern s32 data_027e0018;                 /* DTCM */
extern s32 data_027e0020;                 /* DTCM */

/*
 * Aim at the player. If the player is in another zone, aim at the point
 * where the line to the player crosses into it, or at the zones' doorway.
 */
void Enemy_UpdateTarget(void)
{
    BOOL axis;
    s32 x;
    s32 z;

    ENEMY_WORK(data_027e000c)->zoneB = func_020748d4((s16)PLAYER_X, (s16)PLAYER_Z,
                                         &data_027e000c->destX, &data_027e000c->destZ);
    if (ENEMY_WORK(data_027e000c)->zoneB & 0x10) {
        ENEMY_WORK(data_027e000c)->targetRotY = func_0201de04(PLAYER_X, PLAYER_Z);
        return;
    }
    axis = func_02073d14(ENEMY_WORK(data_027e000c)->zoneA);
    if (axis) {
        z = data_027e0020;
        x = (PLAYER_X - data_027e000c->posX) * (z - data_027e000c->posZ) / (PLAYER_Z - data_027e000c->posZ);
        x += data_027e000c->posX;
    } else {
        x = data_027e0018;
        z = (PLAYER_Z - data_027e000c->posZ) * (x - data_027e000c->posX) / (PLAYER_X - data_027e000c->posX);
        z += data_027e000c->posZ;
    }
    if (Enemy_InZoneOverlap(axis, (s16)x, (s16)z) && !func_02073cd8(x, z)) {
        ENEMY_WORK(data_027e000c)->targetRotY = func_0201de04(x, z);
        data_027e000c->destX = x;
        data_027e000c->destZ = z;
    } else {
        ENEMY_WORK(data_027e000c)->targetRotY = func_02074004((s16)data_027e000c->posX, (s16)data_027e000c->posZ,
                                                  (s8)ENEMY_WORK(data_027e000c)->zoneB, (s8)ENEMY_WORK(data_027e000c)->zoneA);
        data_027e000c->destX = data_027e0018;
        data_027e000c->destZ = data_027e0020;
    }
}


/*
 * Is the coordinate (x if axis, else z) inside the overlap of the object's
 * two zones, shrunk at the ends that func_02073cd8 accepts?
 */
u8 Enemy_InZoneOverlap(s32 axis, s16 x, s16 z)
{
    RoomRect *b;
    RoomRect *a;
    u16 lo;
    u16 hi;
    u16 v;
    u16 margin;

    a = (RoomRect *)((u16 *)data_0214b640.room->zones + ENEMY_WORK(data_027e000c)->zoneA * 6 + 1);
    b = (RoomRect *)((u16 *)data_0214b640.room->zones + ENEMY_WORK(data_027e000c)->zoneB * 6 + 1);
    if (axis) {
        v = x;
        lo = a->x0;
        if (lo <= b->x0) {
            lo = b->x0;
        }
        hi = a->x1;
        if (hi >= b->x1) {
            hi = b->x1;
        }
    } else {
        v = z;
        lo = a->z0;
        if (lo <= b->z0) {
            lo = b->z0;
        }
        hi = a->z1;
        if (hi >= b->z1) {
            hi = b->z1;
        }
    }
    margin = ((s32)(hi - lo) >> 3) + 0x280;
    if (axis) {
        if (func_02073cd8(lo, z)) {
            lo += margin;
        }
        if (func_02073cd8(hi, z)) {
            hi -= margin;
        }
    } else {
        if (func_02073cd8(x, lo)) {
            lo += margin;
        }
        if (func_02073cd8(x, hi)) {
            hi -= margin;
        }
    }
    if (lo > hi) {
        return FALSE;
    }
    return (lo <= v) & (v < hi);
}

/* Turn towards targetRotY by up to step (more for odd kinds). */
void Enemy_TurnToTarget(s32 step)
{
    EnemyObj *e = data_027e000c;
    s16 target = ENEMY_WORK(e)->targetRotY;
    s16 s = step + (e->kind & 1) * 8;
    s16 rot = e->rotY;

    if ((u32)(target + s - rot) <= (u32)(s * 2)) {
        e->rotY = target;
    } else {
        if ((target - rot) & 0x800) {
            e->rotY = rot - s;
        } else {
            e->rotY = rot + s;
        }
        data_027e000c->rotY &= 0xFFF;
    }
}
