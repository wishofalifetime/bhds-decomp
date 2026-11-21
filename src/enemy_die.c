/*
 * enemy_die.c - an enemy's death sequences, a lying-dead loop, the
 * dispatchers by kind and range, and the setup for kind 0x29.
 * JP (ABHJ) 0x020BB620 - 0x020BBE28.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "enemy.h"

/* Effect parameters passed to the effect spawners. */
typedef struct EnemyFx {
    /* 0x00 */ s32 unk00;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 unk0C;
} EnemyFx;

/* The object's position block (posX..0x3F), saved around a call. */
typedef struct EnemyPos {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
    /* 0x0C */ s32 unk0C;
} EnemyPos;

/* Not yet identified. JP addresses. */
extern void func_01ff8078(EnemyVec *a, EnemyVec *b, EnemyVec *out);
extern s32 func_01ffbe38(s32 *pos, u16 value);
extern void func_01ffd504(u32 a);
extern void func_02018188(u32 a, u32 b, u32 c, EnemyVec *pos, EnemyFx *fx, u32 f);
extern void func_02018320(EnemyObj *obj, u32 b, u32 c, u32 d, EnemyFx *fx, u32 f);
extern void func_02055434(EnemyObj *obj);
extern void func_02072bcc(EnemyModel *model, EnemyGfx *gfx);
extern void func_02091220(void *dst, s32 a, s32 b);
extern void func_02091250(void *dst, s32 a, s32 b);
extern void func_020912cc(void *dst, float a, float b);
extern void func_0209132c(void *dst, s32 a, s32 b);
extern void func_0209f86c(EnemyObj *obj, u32 joint, EnemyVec *a, EnemyVec *b);
extern void Bgm_PlayJingle(u16 track);
extern void Snd_Play6(u32 id);
extern void Snd_PlayAtObject3(u32 id, s8 d);

extern VoidFunc data_021242c0[];          /* JP 0x021242C0: by range */
extern VoidFunc data_02124334[];          /* JP 0x02124334: by kind - 0x20 */
extern u16 data_02124248[];               /* JP 0x02124248 */
extern s32 data_027e0018;                 /* DTCM */
extern u32 data_027e0024;                 /* DTCM */
extern s16 data_027e0028[3];              /* DTCM */
extern EnemyVec data_027e003c;            /* DTCM */
extern EnemyFx data_027e0054;             /* DTCM */
extern EnemyVec data_027e007c;            /* DTCM */
extern EnemyFx data_027e0090;             /* DTCM */
extern EnemyVec data_027e009c;            /* DTCM */
extern EnemyFx data_027e00b0;             /* DTCM */
extern EnemyVec data_027e0148;            /* DTCM */

void Enemy_InitKind29(void)
{
    Enemy_Init(0x29);
    data_027e0028[0] = 0;
    data_027e0028[1] = 0;
    data_027e0028[2] = 0;
    data_027e0024 = 0x606060;
    data_027e000c->state = 0;
    data_027e000c->subState = 0;
    data_027e000c->unk0A3 = 0;
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk080 = 0xC;
    data_027e000c->unk00C = data_02124248;
}

void Enemy_RunRange(void)
{
    data_021242c0[data_027e000c->range]();
}

void Enemy_RunKind(void)
{
    data_02124334[data_027e000c->kind - 0x20]();
}

void Enemy_Nop(void)
{
}

/* Lie still, looping the animation. */
void Enemy_Act4(void)
{
    EnemyObj *e = data_027e000c;

    switch (e->unk093) {
    case 0:
        e->unk093 = 1;
        data_027e000c->state = 0;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk098 = 0;
    case 1:
        Enemy_Animate(1, 0x400);
        if (data_027e000c->kind == 0x26) {
            data_027e000c->model->anim->frame = 0;
        }
        break;
    }
}

/* Slide back and fall; when the body lands, play the jingle. */
void Enemy_Die2(void)
{
    EnemyObj *e = data_027e000c;
    EnemyPos pos;

    switch (e->unk093) {
    case 0:
        e->unk093 = 1;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk098 = 0;
        data_027e000c->speed = 1000;
        Enemy_SetAnim(0x8055);
    case 1:
        e = data_027e000c;
        e->speed -= e->subState * 15;
        Enemy_Animate(1, 0x400);
        func_01ffd504(0x800);
        pos = *(EnemyPos *)&data_027e000c->posX;
        data_027e0018 = func_01ffbe38(&data_027e000c->posX, data_027e000c->unk00C[5]);
        e = data_027e000c;
        *(EnemyPos *)&e->posX = pos;
        if (data_027e0018 != 0) {
            e->unk093 = 2;
            Bgm_PlayJingle(0x26B);
            Snd_PlayAtObject3(0x28, 0);
        }
        break;
    case 2:
        e->unk093 = 3;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk098 = 3;
        data_027e000c->timer = 1;
        Enemy_SetAnim(0x8E);
    case 3:
        if (data_027e000c->timer == 0) {
            if (Enemy_Animate(0, 0x400)) {
                data_027e000c->unk093 = 4;
                data_027e000c->unk008 |= 6;
            }
            if (data_027e000c->subState == 0x15) {
                Snd_Play6(0x9F);
            }
            data_027e000c->timer = 1;
        } else {
            data_027e000c->timer--;
        }
        break;
    }
}

/* Collapse with a burst of effects, then sink and shrink away. */
void Enemy_Die1(void)
{
    EnemyObj *e = data_027e000c;

    switch (e->unk093) {
    case 0:
        e->unk093 = 1;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk0A0 = 0xB4;
        data_027e000c->unk098 = 3;
        Enemy_SetAnim(0x58);
        func_02072bcc(data_027e000c->model, data_027e000c->gfx);
        data_027e0054 = data_027e0090;
        func_0209f86c(data_027e000c, 8, &data_027e0148, &data_027e003c);
        func_01ff8078(&data_027e0148, &data_027e003c, &data_027e009c);
        func_02018188(0, 3, 0, &data_027e009c, &data_027e0054, 0);
        func_02018188(0, 3, 0, &data_027e007c, &data_027e00b0, 0);
    case 1:
        e = data_027e000c;
        if (e->subState < 10) {
            data_027e0054 = data_027e0090;
            data_027e0054.unk04 = -0x898;
            func_02018320(e, 0, 0, 0, &data_027e0054, 0);
        }
        if (data_027e000c->subState == 0x2A) {
            Snd_PlayAtObject3(9, 0);
        }
        if (Enemy_Animate(0, 0x400)) {
            data_027e000c->unk093 = 2;
        }
        break;
    case 2:
        e->unk007 = 0;
        func_02055434(data_027e000c);
        func_0209132c(data_027e000c->unk054, -500, 0);
        func_020912cc(data_027e000c->unk054, 1.8f, 1.8f);
        data_027e000c->unk093 = 3;
        data_027e000c->timer = 30;
        break;
    case 3:
        func_02091220(e->unk054, 0x10, 0x10);
        data_027e000c->timer--;
        if (data_027e000c->timer == 0) {
            data_027e000c->unk093 = 4;
        }
        break;
    case 4:
        break;
    }
}

/* Burn: effects from a joint while the animation plays, then fade. */
void Enemy_Die0(void)
{
    EnemyObj *e = data_027e000c;

    switch (e->unk093) {
    case 0:
        e->unk093 = 1;
        data_027e000c->subState = 0;
        data_027e000c->unk0A3 = 0;
        data_027e000c->unk098 = 0;
        data_027e000c->timer = 0xFA;
        data_027e000c->unk096 = 0x80;
        data_027e000c->unk008 |= 6;
        data_027e000c->rotY = 0xC83;
        Enemy_SetAnim(0x67);
    case 1:
        func_0209f86c(data_027e000c, 0, &data_027e0148, &data_027e003c);
        func_01ff8078(&data_027e0148, &data_027e003c, &data_027e009c);
        e = data_027e000c;
        if (e->subState >= 2 && e->subState < 11) {
            if (e->subState == 2) {
                func_02072bcc(e->model, e->gfx);
            }
            data_027e0054 = data_027e0090;
            data_027e0054.unk04 = -0x5DC;
            func_02018188(0, 0, 0, &data_027e009c, &data_027e0054, 0);
        }
        if (data_027e000c->subState > 0x5F && (data_027e000c->subState & 1)) {
            data_027e0054 = data_027e0090;
            func_02018188(0, 0, 0, &data_027e009c, &data_027e0054, 0);
        }
        if (Enemy_Animate(0, 0x400)) {
            data_027e000c->unk093 = 2;
            func_02091250(data_027e000c->unk054, 0x19C, 0x21C);
            func_02055434(data_027e000c);
            func_0209132c(data_027e000c->unk054, 0, 0);
        }
        break;
    case 2:
        e->unk094 = -1;
        func_02091220(data_027e000c->unk054, 6, 6);
        data_027e000c->timer--;
        if (data_027e000c->timer == 0) {
            data_027e000c->unk093 = 3;
        }
        break;
    }
}
