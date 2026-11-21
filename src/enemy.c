/*
 * enemy.c - movement of the current object (data_027e000c): random
 * wandering and moving forward.
 * JP (ABHJ) 0x020BA1B0 - 0x020BA670.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"
#include "mathrand.h"
#include "enemy.h"

/* Not yet identified. JP addresses. */
extern void func_020a1d9c(s32 angle, s16 *in, s16 *out);  /* rotate a vector */
extern void func_020abdec(u32 kind);

extern s16 data_027e0028[3];              /* DTCM */

/* Move forward along rotY. */
void Enemy_MoveForward(s16 speed)
{
    data_027e0028[1] = 0;
    data_027e0028[2] = 0;
    data_027e0028[0] = speed;
    func_020a1d9c(data_027e000c->rotY, data_027e0028, data_027e0028);
    data_027e000c->posX += data_027e0028[0];
    data_027e000c->posZ += data_027e0028[2];
}

void Enemy_StartIdle(void)
{
    ENEMY_WORK(data_027e000c)->timer = ((MATH_Rand32(&data_027e0064, 0) & 0x18) + 0x30) >> 1;
    ENEMY_WORK(data_027e000c)->unk04 = 0;
    ENEMY_WORK(data_027e000c)->unk05 = 0;
    ENEMY_WORK(data_027e000c)->unk06 = 0;
    ENEMY_WORK(data_027e000c)->unk07 = 0;
    data_027e000c->anim = 0x10;
    data_027e000c->unkB0 = 0;
    data_027e000c->unkB4 = 0;
    data_027e000c->unkB8 = 0;
    data_027e000c->unkBD = 0xC0;
    data_027e000c->unkBE = 0x40;
}

/* Pick new random wander parameters whenever the timer runs out. */
void Enemy_Wander(u32 arg)
{
    u8 mask;
    u16 r;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    EnemyObj *e;

    data_027e000c->anim = 0x33;
    mask = (((arg & 1) ^ 1) + 1) * 4 - 1;
    ENEMY_WORK(data_027e000c)->timer--;
    if (ENEMY_WORK(data_027e000c)->timer != 0) {
        return;
    }
    ENEMY_WORK(data_027e000c)->timer = (MATH_Rand32(&data_027e0064, 0) & 0x18) + 0x30;
    r = MATH_Rand32(&data_027e0064, 0);
    a = r & mask;
    b = (r >> 4) & 7;
    c = (r >> 8) & 0x18;
    d = (r >> 12) & 0x18;

    e = data_027e000c;
    if (ENEMY_WORK(e)->unk04 == a || ((ENEMY_WORK(e)->unk04 | a) & 3) == 0) {
        a = (a + 1) & mask;
    }
    ENEMY_WORK(e)->unk04 = a;
    data_027e000c->unkB4 = (a & 3) * 0x60;
    if (!(a & 4)) {
        data_027e000c->unkB4 = -data_027e000c->unkB4;
    }
    data_027e000c->unkB4 &= 0xFFF;

    e = data_027e000c;
    if (ENEMY_WORK(e)->unk05 == b || ((ENEMY_WORK(e)->unk05 | b) & 3) == 0) {
        b = (b + 1) & 7;
    }
    ENEMY_WORK(e)->unk05 = b;
    data_027e000c->unkB0 = (b & 3) * 0xA0;
    if (b & 4) {
        data_027e000c->unkB0 = -data_027e000c->unkB0;
    }
    data_027e000c->unkB0 &= 0xFFF;

    if (ENEMY_WORK(data_027e000c)->unk06 == c) {
        c = (c + 8) & 0x18;
    }
    ENEMY_WORK(data_027e000c)->unk06 = c;
    data_027e000c->unkBE = c + 0x14;
    if (ENEMY_WORK(data_027e000c)->unk07 == d) {
        d = (d + 8) & 0x18;
    }
    ENEMY_WORK(data_027e000c)->unk07 = d;
    data_027e000c->unkBD = d + 0x28;
}

void Enemy_StartWander(s32 x, s32 z)
{
    ENEMY_WORK(data_027e000c)->timer = ((MATH_Rand32(&data_027e0064, 0) & 0x18) + 0x30) >> 1;
    ENEMY_WORK(data_027e000c)->unk04 = 0;
    ENEMY_WORK(data_027e000c)->unk05 = 0;
    ENEMY_WORK(data_027e000c)->unk06 = 0;
    ENEMY_WORK(data_027e000c)->unk07 = 0;
    data_027e000c->anim = 0x11;
    data_027e000c->unkB0 = x;
    data_027e000c->unkB4 = 0;
    data_027e000c->unkB8 = z;
    data_027e000c->unkBD = 0xC0;
    data_027e000c->unkBE = 0x40;
}

void Enemy_CheckState(void)
{
    EnemyObj *e = data_027e000c;

    switch (e->state) {
    case 3:
    case 7:
        if (e->subState == 7 || e->subState == 0x15) {
            func_020abdec(0);
        }
        break;
    case 8:
        if (e->subState == 7 || e->subState == 0x11) {
            func_020abdec(1);
        }
        break;
    }
}
