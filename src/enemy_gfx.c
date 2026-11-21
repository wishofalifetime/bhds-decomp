/*
 * enemy_gfx.c - an enemy's animation step and graphics setup.
 * JP (ABHJ) 0x020BB37C - 0x020BB620.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "enemy.h"

/* Not yet identified. JP addresses. */
extern EnemySlot *func_01ff9190(EnemyObj *obj, u32 count);
extern s32 func_02059980(EnemyObj *obj);
extern void func_02075d00(EnemyGfx *gfx, EnemyModel *model, u32 flags);
extern void func_0207709c(EnemyRes **res);
extern EnemyRes *func_020770e4(u8 id);
extern u8 func_02075538(u8 mode, s16 speed);
extern u8 func_02090d00(u8 mode, s16 speed, u32 count);
extern void *func_0209fd04(u32 count);
extern void func_020a00cc(EnemyGfx **gfx, u32 kind);
extern void func_020f4094(void *dst, void *tex);
extern void func_020f80b4(void *tex, u32 a, u32 b);
extern void func_020f8124(void *tex, u32 a, u32 b);
extern u8 *func_020f81d4(void *data);

/* The US and EU versions have one more resource below these ids. */
#ifdef VERSION_JP
#define ENEMY_RES_SPECIAL 0x90  /* kind 0x23, variant 6 */
#define ENEMY_GFX_KIND    0x72
#else
#define ENEMY_RES_SPECIAL 0x91
#define ENEMY_GFX_KIND    0x73
#endif

extern EnemySlot *data_0213fc68;          /* JP 0x0213FC68 */
extern u8 *data_02121a10[];               /* JP 0x02121A10: resource ids by kind and variant */

void Enemy_AllocGfx(void)
{
    EnemyGfx *gfx;

    func_020a00cc(&data_027e000c->gfx, ENEMY_GFX_KIND);
    gfx = data_027e000c->gfx;
    gfx->slot = func_0209fd04(1);
}

void Enemy_ResetGfx(void);

void Enemy_LoadGfx(void)
{
    EnemyGfx *gfx = data_027e000c->gfx;
    u8 *hdr;
    u8 id;

    if (gfx == NULL) {
        return;
    }
    if (gfx->res != NULL) {
        func_0207709c(&gfx->res);
    }
    if (data_027e000c->kind == 0x23 && data_027e000c->variant == 6) {
        id = ENEMY_RES_SPECIAL;
    } else {
        id = data_02121a10[data_027e000c->kind - 0x20][data_027e000c->variant];
    }
    gfx->res = func_020770e4(id);
    hdr = func_020f81d4(gfx->res->data);
    gfx->tex = hdr + *(s32 *)(hdr + *(u16 *)(hdr + 0xE) + 0xC);
    func_020f4094(gfx->unk20, gfx->tex);
    Enemy_ResetGfx();
}

void Enemy_ResetGfx(void)
{
    EnemyGfx *gfx = data_027e000c->gfx;

    func_020f8124(gfx->tex, 0, 0x400);
    func_020f80b4(gfx->tex, 0, 0xF);
    func_02075d00(gfx, data_027e000c->model, 0);
    gfx->slotPtr = &data_0213fc68[gfx->slot->index];
}

/*
 * Step the animation (returns the animation flags), then copy the joint
 * slot and enable either the model's slot or the graphics slot.
 */
u8 Enemy_Animate(u8 mode, s16 speed)
{
    u8 ret;
    EnemySlot *other;
    EnemySlot *slot;

    if (func_02059980(data_027e000c)) {
        ret = func_02090d00(mode, speed, 7);
    } else {
        ret = func_02075538(mode, speed);
    }
    slot = &data_0213fc68[data_027e000c->gfx->slot->index];
    *slot = *func_01ff9190(data_027e000c, 7);
    other = &data_0213fc68[data_027e000c->model->slot->index] + 7;
    if (data_027e000c->variant != 0) {
        other->flags &= ~1;
        {
            EnemyVec v = {0};
            other->vec = v;
        }
    } else {
        other->flags |= 1;
        slot->flags &= ~1;
        {
            EnemyVec v = {0};
            slot->vec = v;
        }
    }
    return ret;
}
