/*
 * enemy_main.c - creating an enemy and running it each frame.
 * JP (ABHJ) 0x020BC498 - 0x020BC624.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "enemy.h"
#include "scd.h"
#include "scd_data.h"

/* Not yet identified. JP addresses. */
extern u8 func_0200ea0c(s32 *pos, void *a, u8 b);
extern void func_02090e54(EnemyObj *obj, u8 a);
extern void func_020a2c88(EnemyObj *obj);
extern void func_020a7fe4(void);
extern void Scd_UpdateRotNode(EnemyObj *obj);

extern VoidFunc data_021242d0[];          /* JP 0x021242D0: by top-level state */
extern VoidFunc data_021242f8[];          /* JP 0x021242F8: setup by kind - 0x20 */

/* Run the enemy unless the game is paused by the script. */
void Enemy_Frame(void)
{
    EnemyObj *e;

    if ((data_02143640.unk0F == 0 && (gScd.unk74 & 2)) || data_027e000c->unk090 == 0) {
        data_021242d0[data_027e000c->unk090]();
        func_020a7fe4();
        Scd_UpdateRotNode(data_027e000c);
        e = data_027e000c;
        if (e->unk168 != 0) {
            func_02090e54(e, e->unk0C1);
        }
    }
    data_027e000c->unk018 = 0;
    data_027e000c->unk00B = func_0200ea0c(&data_027e000c->posX, data_0214b640.unk7B0, data_027e000c->unk00B);
}

void Enemy_Create(void)
{
    data_027e000c->unk090 = 1;
    data_027e000c->unk091 = 0;
    data_027e000c->range = 0;
    data_027e000c->unk093 = 0;
    data_027e000c->unk042 = 0;
    data_027e000c->unk046 = 0;
    data_027e000c->unk094 = -1;
    data_027e000c->unk0AE = 0;
    data_027e000c->unk0C1 = 8;
    data_027e000c->unk0C2 = 0;
    data_027e000c->unk0C4 = 0;
    Enemy_AllocGfx();
    data_021242f8[data_027e000c->kind - 0x20]();
    Enemy_LoadGfx();
    func_020a2c88(data_027e000c);
}
