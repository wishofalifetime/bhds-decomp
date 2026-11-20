/*
 * itemstack.c - combining item stacks, and two scene flag helpers.
 * JP (ABHJ) 0x020B9988 - 0x020B9ABC.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"

/* An inventory slot. */
typedef struct ItemSlot {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 count;
} ItemSlot;

/* Per-item data, 4 bytes per item. */
typedef struct ItemStackInfo {
    /* 0x00 */ u8 maxCount;
    /* 0x01 */ u8 unk01[3];
} ItemStackInfo;

extern ItemStackInfo data_0211d738[];  /* JP 0x0211D738 */
extern u32 data_0213b784[];            /* JP 0x0213B784: flags */

void Item_MergeInto(ItemSlot *src, ItemSlot *dst);
void Item_MoveInto(ItemSlot *src, ItemSlot *dst);

void Item_Nop(void)
{
}

void Item_MergeFrom(ItemSlot *dst, ItemSlot *src)
{
    Item_MergeInto(src, dst);
}

/* Add src's count to dst up to its maximum; src keeps the rest or is emptied. */
void Item_MergeInto(ItemSlot *src, ItemSlot *dst)
{
    s32 max = data_0211d738[dst->id].maxCount;
    u16 sum = src->count + dst->count;

    if (sum > max) {
        dst->count = max;
        src->count = sum - max;
    } else {
        dst->count = sum;
        src->id = 0;
    }
}

/* Combine two stacks of up to 250. */
void Item_MergeBullets(ItemSlot *a, ItemSlot *b)
{
    u16 sum = a->count + b->count;

    if (sum > 250) {
        a->count = 250;
        b->count = sum - 250;
    } else {
        a->count = sum;
        b->id = 0;
    }
}

void Scene_SetUnk278Flag(void)
{
    data_0213b214.unk278 = 2;
    Flag_Set(data_0213b784, 0x16);
}

void Scene_SetUnk278(void)
{
    data_0213b214.unk278 = 3;
}

void Item_MoveFrom(ItemSlot *dst, ItemSlot *src)
{
    Item_MoveInto(src, dst);
}

/* Move up to dst's maximum from src into dst. */
void Item_MoveInto(ItemSlot *src, ItemSlot *dst)
{
    u8 max = data_0211d738[dst->id].maxCount;

    if (src->count > max) {
        dst->count = max;
        src->count -= max;
    } else {
        dst->count = src->count;
        src->id = 0;
    }
}
