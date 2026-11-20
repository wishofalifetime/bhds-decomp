/*
 * invuse.c - inventory: combining two items and using healing items.
 * JP (ABHJ) 0x020B9ABC - 0x020BA1B0.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "game.h"
#include "scd_data.h"

/* An inventory slot. */
typedef struct ItemSlot {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 count;
} ItemSlot;

/* Per-item data, 4 bytes per item. JP 0x0211D738 */
typedef struct ItemStackInfo {
    /* 0x00 */ u8 maxCount;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ s8 combineGroup;  /* -1: cannot be combined */
    /* 0x03 */ u8 unk03;
} ItemStackInfo;

/* Combine rule: other item, results for both slots, handler. */
typedef struct CombineRule {
    /* 0x00 */ u8 with;
    /* 0x01 */ u8 resultA;
    /* 0x02 */ u8 resultB;
    /* 0x03 */ u8 kind;          /* handler in data_021241d8; 6 and up need an empty slot */
} CombineRule;

/* Inventory screen work area (data_0213b214.work). */
typedef struct InvWork {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 slotA;
    /* 0x02 */ u8 slotB;
    /* 0x03 */ u8 unk03[0xC - 0x3];
    /* 0x0C */ u16 message;
    /* 0x0E */ u8 unk0E;
    /* 0x0F */ u8 state;
    /* 0x10 */ s32 unk10;
} InvWork;

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u8 unk04;
} UnkStruct_0213a6cc;

typedef struct UnkStruct_0213b49c {
    /* 0x000 */ u8 unk000[0x1D4];
    /* 0x1D4 */ u8 unk1D4;
    /* 0x1D5 */ u8 unk1D5[0x1FC - 0x1D5];
    /* 0x1FC */ u32 unk1FC_0 : 1;
                u32 notify : 1;
} UnkStruct_0213b49c;

typedef BOOL (*BOOLFunc)(void);
typedef void (*CombineFunc)(ItemSlot *a, ItemSlot *b);

#define INV_WORK ((InvWork *)data_0213b214.work)
#define INV_SLOTS ((ItemSlot *)data_02147640.unk538)

/* Not yet identified. JP addresses. */
extern s32 func_02070d2c(u8 a);
extern void func_020ad2e4(s16 health);
extern void Hud_SetMode(u32 kind);
extern void Snd_Play0(u32 id);

extern ItemStackInfo data_0211d738[];     /* JP 0x0211D738 */
extern u8 *data_0211d6a0[];               /* JP 0x0211D6A0: combine rules by group (count, then rules) */
extern u8 data_0211d684[];                /* JP 0x0211D684: healing item effects from item 0x41 */
extern u16 data_0211dafc[];               /* JP 0x0211DAFC: heal percentage */
extern CombineFunc data_021241d8[];       /* JP 0x021241D8: combine handlers by kind */
extern VoidFunc data_021241c8[];          /* JP 0x021241C8: inventory handlers by state */
extern VoidFunc data_021241d0[];          /* JP 0x021241D0: item use handlers by state */
extern BOOLFunc data_021241f8[];          /* JP 0x021241F8: item use handlers by item range */
extern u8 data_0211d678[];                /* JP 0x0211D678: first item id of each range */
extern u32 data_0213d48c[];               /* JP 0x0213D48C: key item flags */
extern UnkStruct_0213a6cc data_0213a6cc;  /* JP 0x0213A6CC */
extern UnkStruct_0213b49c data_0213b49c;  /* JP 0x0213B49C */
extern u32 data_0213b764[];               /* JP 0x0213B764: flags */

u32 Inv_CheckCombine(void);
void Inv_SetUsedItem(void);

void Inv_UseUpdate(void)
{
    data_021241d0[INV_WORK->state]();
}

/* Use the selected item; consume one if the handler for its range accepts it. */
void Inv_Use(void)
{
    u8 i;
    u32 slot;

    INV_WORK->state = 1;
    slot = INV_WORK->slotA;
    for (i = 9; data_0213b214.itemId < data_0211d678[i]; i--) {
    }
    if (data_021241f8[i]()) {
        if (INV_SLOTS[slot].count != 0) {
            u8 *p = &INV_SLOTS->count;
            p[slot * 2]--;
            if (INV_SLOTS[slot].count == 0) {
                INV_SLOTS[slot].id = 0;
                data_0213b214.itemId = 0;
            }
        }
    } else {
        INV_WORK->message = i + 0x76;
    }
}

void Inv_Close(void)
{
    if (data_0213a71c.unk05 & 0x80) {
        return;
    }
    data_0213b214.unk279 |= 2;
    INV_WORK->unk10 = 0;
}

BOOL Inv_ReturnFalse(void)
{
    return FALSE;
}

/* Use a key item: allowed only if its flag is set. */
BOOL Inv_UseKeyItem(void)
{
    if (!Flag_Test(data_0213d48c, data_0213b214.itemId - 0x1B)) {
        return FALSE;
    }
    data_0213a71c.unk12 = data_0213b214.itemId;
    data_0213b214.unk279 |= 0x80;
    return TRUE;
}

BOOL Inv_UseItem3E(void)
{
    if (data_0213b214.itemId != 0x3E) {
        return FALSE;
    }
    return Inv_UseKeyItem();
}

/* Use a healing item: restore health and cure status as the item says. */
BOOL Inv_UseHealItem(void)
{
    u8 effect = data_0211d684[data_0213b214.itemId - 0x41];
    u8 cure = effect & 0xF0;
    u8 kind = effect & 0xF;
    s32 amount = func_02070d2c(data_02143640.unkF4D);
    u16 percent;
    u32 health;
    s16 cur;
    BOOL used;
    u8 hudKind;
    u8 bit;

    if (data_0213a6cc.unk04 != 0) {
        percent = data_0211dafc[(u8)((data_0213b49c.notify == 1) + data_02143640.unkF4D * 2)];
    } else {
        percent = 100;
    }
    switch (kind) {
    case 1:
        amount = amount / 3;
        break;
    case 2:
        amount = amount * 2 / 3;
        break;
    case 3:
        break;
    default:
        amount = 0;
        break;
    }
    health = (u32)(amount * percent) / 100;
    cur = data_02144540.unkD8;
    health += cur;
    used = FALSE;
    hudKind = 0;
    if (health > (u32)data_02144640.maxHealth) {
        health = data_02144640.maxHealth;
    }
    if (cur != health) {
        func_020ad2e4(health);
        hudKind = 1;
        used = TRUE;
        data_0213b214.unk279 |= 0x20;
    }
    if (used && data_0213a6cc.unk04 != 0) {
        data_0213b49c.unk1D4 &= ~4;
    }
    switch (cure) {
    case 0x10:
        bit = 0x20;
        break;
    case 0x20:
        bit = 2;
        break;
    default:
        bit = 0;
        break;
    }
    if (data_02144640.unk04 & bit) {
        data_02144640.unk04 &= ~bit;
        if (bit == 0x20) {
            Flag_Clear(data_0213b764, 0x43);
        }
        used = TRUE;
        data_0213b214.unk279 |= 0x10;
        hudKind += 2;
    }
    if (used) {
        Hud_SetMode(hudKind);
    }
    return used;
}

BOOL Inv_ReturnTrue(void)
{
    return TRUE;
}

void Inv_SetUsedItem(void)
{
    data_0213b214.itemId = INV_SLOTS[(s8)INV_WORK->slotA].id;
}

void Inv_Update(void)
{
    data_021241c8[INV_WORK->state]();
}

/* Try to combine the two selected items. */
void Inv_Combine(void)
{
    u32 message = 0;

    data_0213b214.unk278 = 0;
    if (data_0211d738[INV_SLOTS[INV_WORK->slotA].id].combineGroup >= 0 &&
        data_0211d738[INV_SLOTS[INV_WORK->slotB].id].combineGroup >= 0) {
        switch (Inv_CheckCombine()) {
        case 2:
        case 4:
            Inv_SetUsedItem();
            data_0213b214.unk279 |= 8;
            if (data_0213b214.unk278 != 0) {
                message = data_0213b214.unk278 + 0x70;
            }
            break;
        case 1:
            data_0213b214.unk278 = 1;
            message = 0x71;
            break;
        case 3:
            data_0213b214.unk278 = 5;
            message = 0x75;
            break;
        default:
            goto fail;
        }
        INV_WORK->state = 1;
        if (message != 0) {
            INV_WORK->message = message;
        }
        Snd_Play0(3);
        return;
    }
fail:
    data_0213b214.unk279 |= 1;
    INV_WORK->unk10 = 0;
    INV_WORK->message = 0;
}

void Inv_Cancel(void)
{
    if (data_0213a71c.unk05 & 0x80) {
        return;
    }
    if (data_0213b214.unk278 == 2) {
        data_0213b214.unk279 |= 0x40;
    }
    data_0213b214.unk279 |= 2;
    INV_WORK->unk10 = 0;
    INV_WORK->message = 0;
}

/*
 * Look up a combine rule for the two selected items. Returns 0 (none),
 * 1 (not allowed here), 3 (two of items 0x43-0x4B) or 4 (combined).
 */
u32 Inv_CheckCombine(void)
{
    ItemSlot *a = &INV_SLOTS[INV_WORK->slotA];
    ItemSlot *b = &INV_SLOTS[INV_WORK->slotB];
    CombineRule *r;
    u8 *rules;
    u8 idA = a->id;
    u8 n;

    rules = data_0211d6a0[data_0211d738[idA].combineGroup];
    r = (CombineRule *)(rules + 1);
    n = rules[0];

    while (n-- != 0) {
        if (b->id == r->with) {
            if (r->kind >= 6) {
                if (r->kind == 6) {
                    if (a->count != 0) {
                        return 0;
                    }
                } else if (b->count != 0) {
                    return 0;
                }
            }
            if (idA >= 0x13 && idA <= 0x1A && !(data_0213a71c.room == 9 && data_0213a71c.stage == 3)) {
                return 1;
            }
            a->id = r->resultA;
            b->id = r->resultB;
            data_021241d8[r->kind](a, b);
            return 4;
        }
        r++;
    }
    if (idA >= 0x43 && idA <= 0x4B && b->id >= 0x43 && b->id <= 0x4B) {
        return 3;
    }
    return 0;
}
