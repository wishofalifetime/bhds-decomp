/*
 * reward.c - the bonus item scene after clearing the game: each bonus
 * item is shown as a turning model, then the rewards are saved.
 * JP (ABHJ) 0x020BDC48 - 0x020BE0C4.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "exam.h"
#include "game.h"
#include "scd_data.h"

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 clearCount;
    /* 0x02 */ u8 unk02;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u8 unk04;
} UnkStruct_0213a6cc;

/* A bonus item and the condition for getting it. */
typedef struct RewardEntry {
    /* 0x00 */ u8 item;          /* 0: end of the list */
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u16 unk02;
    /* 0x04 */ BOOL (*check)(void);  /* NULL: always */
} RewardEntry;

/* Not yet identified. JP addresses. */
extern void func_02071e94(u8 a, u8 b);
extern s32 func_02074b40(void);
extern s32 func_02074cf0(u8 id);
extern void func_020928e8(void);
extern void func_02092940(u16 msg, u32 a, u32 b, u32 arg);
extern void func_020c3224(void *a);
extern void func_020c31c8(u8 item);
extern void func_020c4174(u32 a);
extern void func_020c4188(u32 a, u32 b);
extern void ItemSprite_Draw(ItemSprite *s);
extern void ItemSprite_Free(ItemSprite *s);

extern VoidFunc data_02124424[];          /* JP 0x02124424: by data_0213b214.state */
extern RewardEntry data_021244c4[];       /* JP 0x021244C4 */
extern UnkStruct_0213a6cc data_0213a6cc;  /* JP 0x0213A6CC */
extern u16 data_0213a708;                 /* JP 0x0213A708: nonzero while fading */
extern UnkStruct_0213b414 data_0213b414;  /* JP 0x0213B414 */
extern u8 data_0213b46c[];                /* JP 0x0213B46C (data_0213b214.model) */
extern u32 data_0213b784[];               /* JP 0x0213B784: flags */
extern u8 data_0213b8b2[];                /* JP 0x0213B8B2: (item, count) pairs */
extern u8 data_0213b8b3[];                /* JP 0x0213B8B3 */

void Reward_SetSlot(u8 item, u8 count);
BOOL Reward_HasItem(u8 id);

/* Save the rewards and leave. */
void Reward_Finish(void)
{
    if (data_0213a708 != 0) {
        return;
    }
    func_020928e8();
    if (Flag_Test(data_0213b784, 0x7E)) {
        data_0213b214.unk1E |= 1;
    }
    if (Reward_HasItem(0x38)) {
        data_0213b214.unk1E |= 2;
    }
    switch (data_0213a6cc.clearCount) {
    case 2:
    case 3:
        data_0213a6cc.clearCount -= 2;
        data_0213a6cc.unk04 = 0;
        func_02071e94(data_0213a6cc.unk02, data_0213a6cc.clearCount);
        func_020c4188(0x17, 1);
        break;
    default:
        data_0213a6cc.unk04 = 0;
        func_02071e94(data_0213a6cc.unk02, data_0213a6cc.clearCount);
        Flag_Set(data_0213b784, 0x7B);
        if (data_0213b214.unk1E & 1) {
            Flag_Set(data_0213b784, 0x7E);
            Reward_SetSlot(10, 4);
        }
        if (data_0213b214.unk1E & 2) {
            Reward_SetSlot(0x38, 1);
        }
        if (data_0213b214.unk1E != 0) {
            func_020c4188(0x17, 1);
        } else {
            func_020c4174(0);
        }
        break;
    }
}

void Reward_SetSlot(u8 item, u8 count)
{
    s32 i = func_02074b40();

    if (i != -1) {
        data_0213b8b2[i * 2] = item;
        data_0213b8b3[i * 2] = count;
    }
}

void Reward_Draw(void)
{
    ItemSprite_Draw((ItemSprite *)data_0213b214.work);
    if (data_0213b214.model != NULL) {
        if (data_0213b214.unk28 == 1) {
            if (data_0213b214.unk274 < 144.0) {
                data_0213b214.unk274 = 0;
                data_0213b214.unk28 = 2;
            } else {
                data_0213b214.unk274 -= 288.0;
            }
            data_0213b414.rotY -= 0x1000;
            data_0213b414.rotX -= 0x2000;
        }
        func_020c3224(data_0213b46c);
    }
}

void Reward_FreeIcon(void)
{
    ItemSprite_Free((ItemSprite *)data_0213b214.work);
}

/* Find the next bonus item whose condition holds. */
u8 Reward_Next(void)
{
    u32 i;
    RewardEntry *e;

    if (data_0213a6cc.clearCount >= 2) {
        return 0;
    }
    i = data_0213b214.unk24;
    e = &data_021244c4[i];
    while (e->item != 0) {
        if (e->check == NULL || e->check()) {
            break;
        }
        e++;
        i++;
    }
    data_0213b214.itemId = e->item;
    data_0213b214.unk24 = i + 1;
    data_0213b214.unk25 = e->unk01;
    data_0213b214.speed = e->unk02;
    return e->item;
}

/* Bonus for a fast or very long clear. */
BOOL Reward_CheckTime(void)
{
    BOOL ret = FALSE;

    if (!Flag_Test(data_0213b784, 0x7E)) {
        if (data_0213b750.playTime <= 0x9E340) {
            ret = TRUE;
        }
        if (data_0213b750.playTime >= 0x62E080) {
            ret = TRUE;
        }
    }
    if (ret) {
        data_0213b214.unk1E |= 1;
    }
    return ret;
}

BOOL Reward_CheckItem38(void)
{
    if (gGame.unkB8[0] >= 5 && !Reward_HasItem(0x38)) {
        data_0213b214.unk1E |= 2;
        return TRUE;
    }
    return FALSE;
}

/* Is the item in the inventory or the item box? */
BOOL Reward_HasItem(u8 id)
{
    BOOL ret = TRUE;
    u32 i;

    if (func_02074cf0(id) < 0) {
        for (i = 0; i < 0x30; i++) {
            if (id == data_0213b750.entries[i].id) {
                goto done;
            }
        }
        ret = FALSE;
    }
done:
    return ret;
}

void Reward_ShowMsg(u16 msg)
{
    func_02092940(msg, 0, 0x13, 0x8220);
}

void Reward_Main(void)
{
    data_02124424[data_0213b214.state]();
}
