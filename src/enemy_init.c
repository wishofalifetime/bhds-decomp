/*
 * enemy_init.c - setup for each enemy kind: shadow offset and colour,
 * starting animation and the per-kind data table.
 * JP (ABHJ) 0x020BBE28 - 0x020BC498.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "enemy.h"
#include "scd_data.h"

/* Not yet identified. JP addresses. */
extern void func_02055434(EnemyObj *obj);
extern void func_02055740(EnemyObj *obj, u32 a);
extern void func_020557c8(EnemyObj *obj, u32 a);
extern void func_02072bcc(EnemyModel *model, EnemyGfx *gfx);
extern u8 func_02075538(u8 mode, s16 speed);
extern void func_02075c90(void *dst, s32 a, s32 b);
extern void func_02091250(void *dst, s32 a, s32 b);
extern void func_0209132c(void *dst, s32 a, s32 b);
extern void func_02091360(void *dst, float a, float b);

extern u16 data_02124254[];               /* JP 0x02124254 */
extern u16 data_02124260[];               /* JP 0x02124260 */
extern u16 data_0212426c[];               /* JP 0x0212426C */
extern u16 data_02124278[];               /* JP 0x02124278 */
extern u16 data_02124284[];               /* JP 0x02124284 */
extern u16 data_02124290[];               /* JP 0x02124290 */
extern u16 data_0212429c[];               /* JP 0x0212429C */
extern u16 data_021242a8[];               /* JP 0x021242A8 */
extern u16 data_021242b4[];               /* JP 0x021242B4 */
extern u32 data_0213b764[];               /* JP 0x0213B764: flags */
extern u32 data_0213b784[];               /* JP 0x0213B784: flags */
extern u32 data_027e0024;                 /* DTCM: shadow colour */
extern s16 data_027e0028[3];              /* DTCM: shadow offset */

void Enemy_InitKind01(void)
{
    Enemy_Init(1);
    data_027e0028[0] = 0;
    data_027e0028[1] = 0;
    data_027e0028[2] = -0x50;
    data_027e0024 = 0x808080;
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk00C = data_021242a8;
}

void Enemy_InitKind30(void)
{
    Enemy_Init(0x30);
    data_027e0028[0] = 0;
    data_027e0028[1] = 0;
    data_027e0028[2] = -0x50;
    data_027e0024 = 0x808080;
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk00C = data_0212429c;
}

void Enemy_InitKind21(void)
{
    if (data_027e000c->kind == 0x2A) {
        Enemy_Init(0x2A);
        data_027e000c->variant = 0;
    } else {
        Enemy_Init(0x21);
    }
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk00C = data_02124290;
}

void Enemy_InitKind22(void)
{
    if (Flag_Test(data_0213b784, 0x70)) {
        Enemy_Init(0x2F);
    } else {
        Enemy_Init(0x22);
    }
    data_027e000c->unk098 = 0;
    if (Flag_Test(data_0213b764, 0xC0) && data_027e000c->kind == 0x2C) {
        data_027e000c->unk168 = 1;
        data_027e000c->unk007 = 0;
        data_027e000c->unk0A3 = 0;
        Enemy_SetAnim(0x58);
        func_02072bcc(data_027e000c->model, data_027e000c->gfx);
        data_027e000c->subState = 0x3D;
        data_027e000c->model->anim->frame = data_027e000c->subState << 12;
        func_02091360(data_027e000c->unk054, 1.8f, 3.0f);
        func_02055434(data_027e000c);
        func_0209132c(data_027e000c->unk054, 0, 0);
    }
    Enemy_Animate(0, 0x1000);
    data_027e000c->unk00C = data_02124284;
}

void Enemy_InitKind23(void)
{
    Enemy_Init(0x23);
    data_027e000c->unk098 = 0;
    if (Flag_Test(data_0213b784, 0x37)) {
        Enemy_SetAnim(0x8063);
        func_02072bcc(data_027e000c->model, data_027e000c->gfx);
        data_027e000c->model->anim->frame = data_027e000c->model->anim->data->frameCount << 12;
        func_02091360(data_027e000c->unk054, 1.8f, 3.0f);
        func_02055434(data_027e000c);
        func_0209132c(data_027e000c->unk054, 0, 0);
        func_02091250(data_027e000c->unk054, 0x7DC, 0x85C);
        func_02075538(0, 0x400);
    } else {
        Enemy_Animate(0, 0x400);
    }
    data_027e000c->unk00C = data_02124278;
    if (*(u16 *)&data_0213a71c == 0x1104) {
        data_027e000c->unk008 |= 2;
    }
}

void Enemy_InitKind24(void)
{
    Enemy_Init(0x24);
    data_027e0028[0] = 0;
    data_027e0028[1] = 0;
    data_027e0028[2] = 0;
    data_027e0024 = 0xFFFF50;
    data_027e000c->state = 0;
    data_027e000c->subState = 0;
    data_027e000c->unk0A3 = 0;
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk080 = 0xC;
    data_027e000c->unk00C = data_021242b4;
}

/* Set up as kind 0x16 internally, then restore the real kind. */
void Enemy_InitKind16(void)
{
    u8 kind = data_027e000c->kind;

    data_027e000c->kind = 0x16;
    func_02055740(data_027e000c, 5);
    func_020557c8(data_027e000c, 0x11);
    data_027e000c->state = 0xFF;
    data_027e000c->unk0A3 = 0;
    data_027e000c->unk082 = 1;
    func_02075c90(data_027e000c->unk054, 0x3000, 0x1CCC);
    data_027e000c->anim = 0;
    data_027e000c->kind = kind;
    data_027e0028[0] = -600;
    data_027e0028[2] = 200;
    data_027e0024 = 0xFFFF50;
    data_027e0028[1] = 0;
    data_027e000c->state = 0;
    data_027e000c->subState = 0;
    data_027e000c->unk0A3 = 0;
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk00C = data_02124260;
}

void Enemy_InitKind26(void)
{
    Enemy_Init(0x26);
    data_027e0028[0] = 0;
    data_027e0028[1] = 0;
    data_027e0028[2] = 0;
    data_027e0024 = 0x606060;
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk00C = data_02124254;
}

void Enemy_InitKind28(void)
{
    Enemy_Init(0x28);
    data_027e0028[0] = -600;
    data_027e0028[1] = 0;
    data_027e0028[2] = 200;
    data_027e0024 = 0x404040;
    data_027e000c->unk098 = 0;
    Enemy_Animate(0, 0x400);
    data_027e000c->unk00C = data_0212426c;
}
