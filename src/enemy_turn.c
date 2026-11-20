/*
 * enemy_turn.c - turning the current object towards a target angle and
 * testing a coordinate against the overlap of its two zones.
 * JP (ABHJ) 0x020BA670 - 0x020BA87C. Not matching yet: Enemy_TurnToTarget
 * (26/34) and Enemy_InZoneOverlap (75/97) differ only in register choice.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"
#include "enemy.h"

/* Not yet identified. JP addresses. */
extern BOOL func_02073cd8(s32 x, s32 z);

/*
 * Is the coordinate (x if axis, else z) inside the overlap of the object's
 * two zones, shrunk at the ends that func_02073cd8 accepts?
 */
u8 Enemy_InZoneOverlap(s32 axis, s32 x, s32 z)
{
    RoomRect *b;
    RoomRect *a;
    u16 lo;
    u16 hi;
    u16 v;
    u16 margin;

    a = (RoomRect *)((u16 *)data_0214b640.room->zones + data_027e000c->zoneA * 6 + 1);
    b = (RoomRect *)((u16 *)data_0214b640.room->zones + data_027e000c->zoneB * 6 + 1);
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
        if (func_02073cd8((s16)lo, z)) {
            lo += margin;
        }
        if (func_02073cd8((s16)hi, z)) {
            hi -= margin;
        }
    } else {
        if (func_02073cd8(x, (s16)lo)) {
            lo += margin;
        }
        if (func_02073cd8(x, (s16)hi)) {
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
    s16 target = e->targetRotY;
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
