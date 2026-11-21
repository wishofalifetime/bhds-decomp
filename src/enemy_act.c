/*
 * enemy_act.c - an enemy's per-frame update, its first action (idle until
 * woken, then wandering), setup and animation selection.
 * JP (ABHJ) 0x020BAEB4 - 0x020BB37C.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "enemy.h"
#include "mathrand.h"

/* Not yet identified. JP addresses. */
extern void func_01ffabf8(void *list, EnemyObj *obj);
extern void func_01ffbe38(s32 *pos, u16 value);
extern void func_01ffd5dc(void);
extern s32 func_020599f8(EnemyObj *obj);
extern u32 func_02059a9c(EnemyObj *obj, u8 state);
extern u8 func_02073e30(s32 x, s32 z);
extern void func_02075c90(void *dst, s32 a, s32 b);
extern void func_020a00bc(EnemyObj *obj, u8 bank, u8 anim);
extern void func_020a2c88(EnemyObj *obj);
extern void func_020ad5ac(void);
extern void func_020d908c(void);
extern u8 func_020bb37c(u8 mode, s16 speed);

extern u16 data_02121864[];               /* JP 0x02121864: animation bank by kind */
extern u16 data_02121b4c[];               /* JP 0x02121B4C: animation by kind, 0x7FFF: none */
extern VoidFunc data_02124238[];          /* JP 0x02124238: actions by range */
extern s8 data_0211f378[];                /* JP 0x0211F378 */
extern u8 data_02144584[];                /* JP 0x02144584 */

/*
 * Pick an animation. Ids from 0x8000 are looked up per kind in
 * data_02121B4C; the animation restarts only when it changes.
 */
void Enemy_SetAnim(u32 anim)
{
    s32 kind = func_020599f8(data_027e000c);

    if (anim >= 0x8000) {
        anim = data_02121b4c[kind + (anim - 0x8000) * 2];
        if (anim == 0x7FFF) {
            return;
        }
    }
    if (anim != data_027e000c->animId) {
        data_027e000c->animId = anim;
        data_027e000c->subState = 0;
        func_020a00bc(data_027e000c, data_02121864[kind], anim);
    }
}

void Enemy_Hit(void)
{
    s32 r;

    if (data_027e000c->unk0C6 != 0 && (r = func_020599f8(data_027e000c)) != 2) {
        Enemy_SetAnim(r == 0 ? 0x95 : 0x7F);
        data_027e000c->model->anim->frame = data_027e000c->model->anim->data->frameCount << 12;
        data_027e000c->unk098 = 0;
        func_020bb37c(0, 0x1000);
    }
}

void Enemy_Init(void)
{
    u32 anim;
    u8 sub;

    func_020ad5ac();
    data_027e000c->unk080 = 0;
    data_027e000c->unk083 = 0x16;
    data_027e000c->unk082 = 1;
    data_027e000c->unk084 = 1;
    func_02075c90(data_027e000c->unk054, 0x1999, 0x1999);
    data_027e000c->unk0C1 = data_0211f378[1];
    data_027e000c->unk007 = data_0211f378[2];
    data_027e000c->unk0A3 = 0;
    data_027e000c->anim = 0;
    anim = func_02059a9c(data_027e000c, data_027e000c->state);
    sub = data_027e000c->subState;
    Enemy_SetAnim(anim);
    data_027e000c->subState = sub;
    data_027e000c->model->anim->frame = sub << 12;
    data_027e000c->state = 0xFF;
}

void Enemy_Update(void)
{
    u32 mode;

    if (data_027e000c->unk091 == 0) {
        Enemy_Reset();
    }
    func_020d908c();
    Enemy_UpdateRange();
    ENEMY_WORK(data_027e000c)->zoneA = func_02073e30((s16)data_027e000c->posX, (s16)data_027e000c->posZ);
    data_02124238[data_027e000c->range]();
    if (ENEMY_WORK(data_027e000c)->unk01 & 1) {
        mode = 3;
    } else {
        mode = 1;
    }
    ENEMY_WORK(data_027e000c)->flags = func_020bb37c(mode, 0x1000 / (data_027e000c->unk098 + 1));
    Enemy_CheckState();
    func_020a2c88(data_027e000c);
    func_01ffabf8(data_02144584, data_027e000c);
    func_01ffd5dc();
    func_01ffbe38(&data_027e000c->posX, data_027e000c->unk00C[5]);
}

/* Wait a random time, then get up and wander. */
void Enemy_Act3(void)
{
    EnemyObj *e = data_027e000c;

    switch (e->unk093) {
    case 0:
        e->unk093 = 1;
        data_027e000c->state = 0;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk098 = 7;
        Enemy_SetAnim(0x8000);
        ENEMY_WORK(data_027e000c)->count = (MATH_Rand32(&data_027e0064, 0) & 0x38) + 0x40;
        Enemy_StartIdle();
        break;
    case 1:
        ENEMY_WORK(e)->count--;
        if (ENEMY_WORK(data_027e000c)->count != 0) {
            Enemy_Wander(0);
            break;
        }
        data_027e000c->unk093 = 2;
        data_027e000c->state = 5;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk098 = 7;
        Enemy_SetAnim(0x8017);
        Enemy_StartIdle();
    case 2:
        if (!(ENEMY_WORK(data_027e000c)->flags & 1)) {
            break;
        }
        data_027e000c->unk093 = 3;
        data_027e000c->state = 6;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk098 = 7;
        Enemy_SetAnim(0x8001);
    case 3:
        Enemy_Wander((u8)((data_027e000c->kind & 1) ^ 1));
        if (ENEMY_WORK(data_027e000c)->flags != 0) {
            data_027e000c->subState = 0x14;
            data_027e000c->model->anim->frame = 0x14000;
        }
        break;
    }
}
