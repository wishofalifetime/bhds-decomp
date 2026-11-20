/*
 * scd_ops2.c - second cluster of room script opcode handlers.
 * JP (ABHJ) 0x020B36C0 - 0x020B4C28 (the whole second cluster).
 *
 * NOT complete yet (32 of 36 functions match):
 *  - Scd_Op46 48/64: the operand pointer and its per-iteration copy have
 *    swapped registers (r1/r2). Tried: operand struct, byte pointer,
 *    s = src++ at loop top, declaration orders, src before/after storing pc.
 *  - Scd_ClearMaskedGroups 35/37: the original keeps +0xA8 as a load
 *    displacement with a per-element pointer. Tried: struct pointer at
 *    function and block scope, element pointer, [i + 3] indexing, pragmas.
 *  - Scd_Op22 105/118: only which of count/sum/i is zeroed first differs,
 *    plus the gScd/data_02147640 literal registers. Tried: declaration and
 *    statement orders, table/n locals, i initialised in the for statement.
 *    The 7-9 range test only matches written as (u8)(type + 0xF9) <= 2.
 *  - Scd_Op24 17/19: the entry index and table address registers swap.
 *
 * Handlers are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd.h"
#include "scd_data.h"
#include "game.h"
#include "obj.h"

/* Not yet identified. JP addresses. */
extern Obj *func_02053d0c(u8 id);
extern s32 func_02074cf0(u8 id);
extern void func_020ae8f4(void);
extern void func_020d8f74(Obj *obj, s8 a, s8 b, s8 c);
extern void func_020d8ff0(Obj *obj, u8 a, u8 b, u8 c);
extern u32 Sqrt(u32 x);  /* BIOS call stub, Thumb */
extern void func_02017be8(void *entry);
extern void func_02017eec(u32 mode, u8 a, u8 b, u32 c, void *target);
extern void func_02018188(u8 a, u8 b, s16 c, void *target, s32 *pos, u32 e);
extern s32 func_02059a9c(void *a, u8 b);
extern void func_02074f1c(u32 a);
extern void func_020b4c7c(u32 a, u8 b, u32 c, u8 d);
extern void func_0209f844(Obj *obj, u32 a, void *b, void *c);
extern void func_01ff8078(void *a, void *b, void *c);  /* ITCM */

typedef void (*ScdEntryFunc)(UnkEntry_02147d0c *entry);

typedef struct UnkStruct_02144640 {
    /* 0x00 */ u8 unk00[0x30];
    /* 0x30 */ u16 unk30;        /* bits 2-3 cleared by opcode 0x20 */
} UnkStruct_02144640;

extern ScdEntryFunc data_02120cec[];        /* JP 0x02120CEC */
extern UnkStruct_02144640 data_02144640;    /* JP 0x02144640 */
extern u8 data_027e003c[];                  /* DTCM */
extern u8 data_027e009c[];                  /* DTCM */
extern u8 data_027e0148[];                  /* DTCM */

typedef struct UnkEntry_02148320 {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u8 unk02[0xE - 0x2];
    /* 0x0E */ u16 mask;         /* changed by opcode 0x4E */
    /* 0x10 */ u8 unk10[0xBC - 0x10];
} UnkEntry_02148320; /* size 0xBC, 0x50 entries */

extern UnkEntry_02148320 data_02148320[];  /* JP 0x02148320 */
extern UnkEntry_02148320 data_0214bde0[];  /* JP 0x0214BDE0: end of data_02148320 */
/* Objects returned by func_02053d0c and the player at data_02144584.
 * Their 0x90-0xC0 fields do not fit the Obj layout used by opcode 0x1F. */
typedef struct Actor {
    /* 0x00 */ u8 unk00[0x08];
    /* 0x08 */ u8 unk08;
    /* 0x09 */ u8 unk09;
    /* 0x0A */ u8 unk0A;
    /* 0x0B */ u8 unk0B[0x44 - 0x0B];
    /* 0x44 */ s16 unk44;
    /* 0x46 */ u8 unk46[0x82 - 0x46];
    /* 0x82 */ u8 unk82;
    /* 0x83 */ u8 unk83[0x90 - 0x83];
    /* 0x90 */ u8 unk90;
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 unk92;
    /* 0x93 */ u8 unk93;
    /* 0x94 */ s16 unk94;
    /* 0x96 */ u8 unk96;
    /* 0x97 */ u8 unk97;
    /* 0x98 */ u8 unk98;
    /* 0x99 */ u8 unk99[0xA1 - 0x99];
    /* 0xA1 */ u8 unkA1;
    /* 0xA2 */ u8 unkA2;
    /* 0xA3 */ u8 unkA3;
    /* 0xA4 */ u8 unkA4[2];
    /* 0xA6 */ u16 unkA6;
    /* 0xA8 */ u16 unkA8;
    /* 0xAA */ u8 unkAA[0xC0 - 0xAA];
    /* 0xC0 */ u8 unkC0;
    /* 0xC1 */ u8 unkC1[0xEC - 0xC1];
    /* 0xEC */ u16 unkEC;
} Actor;

extern Actor data_02144584;                /* JP 0x02144584: the player */
extern u8 data_027e007c[];                 /* DTCM */
extern u8 data_0213dca4[];                 /* JP 0x0213DCA4 */


typedef struct UnkScriptRef {
    /* 0x00 */ u8 *script;
    /* 0x04 */ u8 unk04[8];
} UnkScriptRef; /* data_02147d0c entries seen from their script field */

extern UnkScriptRef data_02147d14[];       /* JP 0x02147D14 (data_02147d0c + 8) */
extern u16 data_0213a6a8;                  /* JP 0x0213A6A8: group mask for 0x49 */

/* 0x20 ?, s16 a, b, c, x, y, z: set the player's angle and position. */
s32 Scd_Op20(void)
{
    u8 *pc = gScd.pc;
    u16 bits = data_02144640.unk30;
    s16 x;
    s16 y;
    s16 z;

    gScd.pc = pc + 0xE;
    data_02144640.unk30 = bits & 0xFFF3;
    data_02144540.unk86 = *(s16 *)(pc + 2);
    data_02144540.unk88 = *(s16 *)(pc + 4);
    data_02144540.unk8A = *(s16 *)(pc + 6);
    x = *(s16 *)(pc + 8);
    data_02143640.unkF74[0] = x;
    y = *(s16 *)(pc + 0xA);
    data_02143640.unkF74[1] = y;
    z = *(s16 *)(pc + 0xC);
    data_02143640.unkF74[2] = z;
    data_02144540.initX = x;
    data_02144540.initY = y;
    data_02144540.initZ = z;
    return TRUE;
}

/* 0x21 id, s16 a, b, c, x, y, z: set object id's angle and position. */
s32 Scd_Op21(void)
{
    u8 *pc = gScd.pc;
    Obj *obj;

    gScd.pc = pc + 0xE;
    obj = func_02053d0c(pc[1]);
    obj->unkEC &= 0xFFF3;
    obj->unk42 = *(s16 *)(pc + 2);
    obj->unk44 = *(s16 *)(pc + 4);
    obj->unk46 = *(s16 *)(pc + 6);
    obj->posX = *(s16 *)(pc + 8);
    obj->posY = *(s16 *)(pc + 0xA);
    obj->posZ = *(s16 *)(pc + 0xC);
    obj->initX = obj->posX;
    obj->initY = obj->posY;
    obj->initZ = obj->posZ;
    return TRUE;
}

/* 0x47 id, s16 a, b, c, x, y, z: same as 0x21 for the object tables. */
s32 Scd_Op47(void)
{
    u8 *pc = gScd.pc;
    u8 id;
    Obj *obj;

    gScd.pc = pc + 0xE;
    id = pc[1];
    if (id < 0x80) {
        obj = data_0214d56c[id];
    } else {
        obj = data_0214c9e8[id - 0x80];
    }
    obj->unk42 = *(s16 *)(pc + 2);
    obj->unk44 = *(s16 *)(pc + 4);
    obj->unk46 = *(s16 *)(pc + 6);
    obj->posX = *(s16 *)(pc + 8);
    obj->posY = *(s16 *)(pc + 0xA);
    obj->posZ = *(s16 *)(pc + 0xC);
    obj->initX = obj->posX;
    obj->initY = obj->posY;
    obj->initZ = obj->posZ;
    return TRUE;
}

/* 0x22 id, op, value: count/sum entries of data_02147640.unk538 in a category and compare. */
s32 Scd_Op22(void)
{
    u8 *pc = gScd.pc;
    u32 i = 0;
    u32 sum = 0;
    u32 count = 0;
    u8 id = pc[1];
    u8 op = pc[2];
    u8 rhs = pc[3];

    gScd.pc = pc + 4;
    for (; i < data_02147640.unk540; i++) {
        u8 type = data_02147640.unk538[i][0];

        if (id == type || id == type - 9) {
            sum = data_02147640.unk538[i][1];
        }
        switch (id - 10) {
        case 1:
            if (type == 2 || type == 4) {
                count++;
                sum += data_02147640.unk538[i][1];
            }
            break;
        case 2:
            if (type == 3) {
                count++;
                sum += data_02147640.unk538[i][1];
            }
            break;
        case 4:
            if (type == 5) {
                count++;
                sum += data_02147640.unk538[i][1];
            }
            break;
        case 5:
            if (type == 6) {
                count++;
                sum += data_02147640.unk538[i][1];
            }
            break;
        case 6:
        case 7:
        case 8:
            if ((u8)(type + 0xF9) <= 2) {
                count++;
                sum += data_02147640.unk538[i][1];
            }
            break;
        case 0:
            if (type == 10) {
                sum = data_02147640.unk538[i][1];
                count++;
            }
            break;
        }
    }
    if (count != 0) {
        switch (op) {
        case 0:
            return sum == rhs;
        case 1:
            return sum > rhs;
        case 2:
            return sum >= rhs;
        case 3:
            return sum < rhs;
        case 4:
            return sum <= rhs;
        case 5:
            return sum != rhs;
        }
    }
    return FALSE;
}

/* 0x23 on: set or clear flag bit 20. */
s32 Scd_Op23(void)
{
    u8 *pc = gScd.pc;

    gScd.pc = pc + 2;
    if (pc[1] == 0) {
        data_0213b750.flags &= ~0x100000;
    } else {
        data_0213b750.flags |= 0x100000;
    }
    return TRUE;
}

/* 0x24 idx, func: call data_02120cec[func] on entry idx of data_02147d0c. */
s32 Scd_Op24(void)
{
    u8 *pc = gScd.pc;
    UnkEntry_02147d0c *entry = &data_02147d0c[pc[1]];
    u8 func = pc[2];

    gScd.pc = pc + 4;
    data_02120cec[func](entry);
    return TRUE;
}

/* 0x28 ?, id, cmd, ...: change an actor's state (length depends on cmd). */
s32 Scd_Op28(void)
{
    u8 *pc = gScd.pc;
    Actor *actor = (Actor *)func_02053d0c(pc[2]);
    u32 len;

    switch (pc[3]) {
    case 0:
        actor->unk0A = pc[4];
        len = 6;
        break;
    case 1:
        actor->unk90 = 2;
        actor->unk91 = 0;
        actor->unk92 = 0;
        actor->unk93 = 0;
        actor->unk94 = *(s16 *)(pc + 4);
        actor->unk96 = pc[6];
        actor->unkEC = 1;
        len = 8;
        break;
    case 2:
        actor->unk92 = pc[4];
        actor->unk93 = 0;
        len = 6;
        break;
    case 3:
        switch (pc[5]) {
        case 0:
            actor->unk08 = pc[4];
            break;
        case 1:
            actor->unk08 |= pc[4];
            break;
        case 2:
            actor->unk08 ^= pc[4];
            break;
        }
        len = 6;
        break;
    case 5:
        actor->unk44 = *(s16 *)(pc + 4);
        len = 6;
        break;
    case 6:
        actor->unk98 = 0;
        len = 4;
        break;
    case 8:
        actor->unk90 = 9;
        actor->unk91 = 0;
        actor->unk92 = 0;
        actor->unk93 = 0;
        len = 4;
        break;
    case 9:
        len = 6;
        break;
    case 10:
        actor->unk93 = pc[4];
        len = 6;
        break;
    }
    gScd.pc += len;
    return TRUE;
}

/* 0x33 cmd, ...: change the player's state (length depends on cmd). */
s32 Scd_Op33(void)
{
    u8 *pc = gScd.pc;
    Actor *player = &data_02144584;
    u32 len;

    switch (pc[1]) {
    case 0:
        func_02074f1c(0);
        len = 2;
        break;
    case 1:
        player->unk96 = pc[2];
        gScd.unk245 = 0;
        func_020b4c7c(1, 0, 0, 1);
        gScd.unk240 = 0x8000;
        player->unkA2 = 0;
        player->unkA3 = 0;
        player->unkA1 = 0;
        player->unk98 = 3;
        player->unkA8 = 100;
        player->unkA6 = 0;
        len = 4;
        break;
    case 3:
        switch (pc[3]) {
        case 0:
            player->unk08 = pc[2];
            break;
        case 1:
            player->unk08 |= pc[2];
            break;
        case 2:
            player->unk08 ^= pc[2];
            break;
        }
        len = 4;
        break;
    case 4:
        player->unk92 = 1;
        player->unk93 = 6;
        len = 2;
        break;
    case 5:
        player->unk44 = *(s16 *)(pc + 2);
        len = 4;
        break;
    case 6:
        player->unk98 = 0;
        len = 2;
        break;
    case 7:
        player->unkA2 = 0;
        player->unkA3 = 0;
        player->unkA1 = 0;
        player->unk98 = 3;
        player->unk96 = 0;
        func_020b4c7c(1, 0, 0, 2);
        gScd.unk240 = 0x8000;
        len = 2;
        break;
    case 8:
        switch (pc[3]) {
        case 0:
            player->unkC0 = pc[2];
            break;
        case 1:
            player->unkC0 |= pc[2];
            break;
        case 2:
            player->unkC0 ^= pc[2];
            break;
        }
        len = 4;
        break;
    case 9:
        len = 4;
        break;
    case 10:
        if (pc[4] != 0) {
            player->unkEC |= 0x40;
            player->unk82 = 0;
        } else {
            player->unkEC &= ~0x40;
            player->unk82 = 1;
        }
        len = 4;
        break;
    case 11:
        data_02143640.unkF4A = pc[2];
        len = 4;
        break;
    case 12:
        if (pc[2] == 0) {
            data_0213b214.unk94 = 0;
        } else {
            data_0213b214.unk94 = 1;
        }
        len = 4;
        break;
    }
    gScd.pc += len;
    return TRUE;
}

/* 0x2A a, b, kind, s16 x, y, z, d: like 0x3D without keeping the target. */
s32 Scd_Op2A(void)
{
    u8 kind;
    void *target;
    u8 a;
    u8 b;
    s16 d;
    u8 *pc = gScd.pc;
    s32 pos[3];

    gScd.pc = pc + 0xC;
    kind = pc[3];
    a = pc[1];
    b = pc[2];
    pos[0] = *(s16 *)(pc + 4);
    pos[1] = *(s16 *)(pc + 6);
    pos[2] = *(s16 *)(pc + 8);
    d = *(s16 *)(pc + 0xA);
    switch (kind) {
    case 0:
        target = data_027e007c;
        break;
    case 1:
        target = data_0213dca4;
        break;
    default:
        if (kind & 0x80) {
            target = (u8 *)data_0214d56c[kind & 0x7F] + 0x1C;
        } else {
            func_0209f844(func_02053d0c(kind - 2), 0, data_027e0148, data_027e003c);
            func_01ff8078(data_027e0148, data_027e003c, data_027e009c);
            target = data_027e009c;
        }
        break;
    }
    func_02018188(a, b, d, target, pos, 0);
    return TRUE;
}

/* 0x3D a, b, kind, s16 x, y, z, d: set a target and start func_02018188 on it. */
s32 Scd_Op3D(void)
{
    u8 kind;
    void *target;
    u8 a;
    u8 b;
    s16 d;
    u8 *pc = gScd.pc;
    s32 pos[4];

    gScd.pc = pc + 0xC;
    kind = pc[3];
    a = pc[1];
    b = pc[2];
    pos[0] = *(s16 *)(pc + 4);
    pos[1] = *(s16 *)(pc + 6);
    pos[2] = *(s16 *)(pc + 8);
    d = *(s16 *)(pc + 0xA);
    switch (kind) {
    case 0:
        target = data_027e007c;
        break;
    case 1:
        target = data_0213dca4;
        break;
    default:
        if (kind & 0x80) {
            target = (u8 *)data_0214d56c[a] + 0x1C;
        } else {
            target = (u8 *)func_02053d0c(a - 2) + 0x1C;
        }
        break;
    }
    func_02018188(a, b, d, target, pos, 0);
    data_0213a71c.unk0B = a;
    gScd.target = target;
    return TRUE;
}

/* 0x3E: release the target set by 0x3D. */
s32 Scd_Op3E(void)
{
    gScd.pc += 2;
    if (gScd.target != NULL) {
        func_02017eec(9, data_0213a71c.unk0B, 0, 0, gScd.target);
        gScd.target = NULL;
    }
    return TRUE;
}

/* 0x42 a, b */
s32 Scd_Op42(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[1];
    u8 b = pc[2];

    gScd.pc = pc + 4;
    func_02017eec(3, a, b, 0, NULL);
    return TRUE;
}

/* 0x48: reset all 0x50 entries of data_02148320. */
s32 Scd_Op48(void)
{
    s32 i;

    gScd.pc += 2;
    data_0214b640.unk7A0 = 0;
    for (i = 0; i < 0x50; i++) {
        func_02017be8(&data_02148320[i]);
    }
    return TRUE;
}

/* 0x4E mode, u16 mask: set (0), clear (1) or toggle (2) mask on every active entry. */
s32 Scd_Op4E(void)
{
    u8 *pc = gScd.pc;
    u16 mask = *(u16 *)(pc + 2);
    u8 mode = pc[1];
    UnkEntry_02148320 *entry;

    gScd.pc = pc + 4;
    for (entry = data_02148320; entry < data_0214bde0; entry++) {
        if (entry->unk00 || entry->unk01) {
            switch (mode) {
            case 0:
                entry->mask |= mask;
                break;
            case 1:
                entry->mask &= ~mask;
                break;
            case 2:
                entry->mask ^= mask;
                break;
            }
        }
    }
    return TRUE;
}

/* 0x2B a, b, c */
s32 Scd_Op2B(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[1];
    u8 b = pc[2];
    u8 c = pc[3];

    gScd.pc = pc + 4;
    func_020b4c7c(8, data_02143640.unkFD5, 0, a + 2);
    gScd.unk240 = func_02059a9c(&data_02144584, b);
    data_02143640.unkFE6 = c;
    data_02143640.unkFDC = 0;
    data_02143640.unkFE7 = 0;
    return TRUE;
}

/* 0x2C id: clear entry id of data_02147640.unk538. */
s32 Scd_Op2C(void)
{
    u8 *pc = gScd.pc;
    u8 id = pc[1];
    u8 n;

    gScd.pc = pc + 2;
    n = func_02074cf0(id);
    data_02147640.unk538[n][0] = 0;
    data_02147640.unk538[n][1] = 0;
    return TRUE;
}

/* 0x2D: run opcode 0x24, set flag bit 10, clear bit 11 and stop the script. */
s32 Scd_Op2D(void)
{
    Scd_Op24();
    data_0213b750.flags |= 0x400;
    data_0213b750.flags &= ~0x800;
    return FALSE;
}

/* 0x30 group, item, flags, u16 a, b, c, d: set a room group item. */
s32 Scd_Op30(void)
{
    u8 *pc = gScd.pc;
    RoomData *room = data_0214b640.room;
    u8 group;
    u8 flags;
    RoomGroupItem *item;

    gScd.pc = pc + 0xC;
    group = pc[1];
    flags = pc[3];
    item = &room->groups->groups[group][pc[2]];
    if (flags != 0) {
        item->flags &= 0xF0FF;
        item->flags |= (flags << 8) & 0xF00;
    }
    item->unk04 = *(u16 *)(pc + 4);
    item->unk06 = *(u16 *)(pc + 6);
    item->unk00 = *(u16 *)(pc + 8);
    item->unk02 = *(u16 *)(pc + 0xA);
    return TRUE;
}

/* 0x32: 4-byte instruction with no effect on DS. */
s32 Scd_Op32(void)
{
    gScd.pc += 4;
    return TRUE;
}

/* 0x34 mode, slot, a, b, c: act on object slot (0-63). */
s32 Scd_Op34(void)
{
    u8 *pc = gScd.pc;
    s8 mode = pc[1];
    s32 slot = pc[2] & 0x3F;
    s8 a = pc[3];
    s8 b = pc[4];
    s8 c = pc[5];
    Obj *obj = data_0214d56c[slot];

    gScd.pc = pc + 6;
    switch (mode) {
    case 0:
        func_020d8ff0(obj, a, b, c);
        break;
    case 1:
        func_020d8f74(obj, a, b, c);
        break;
    case 2:
        obj->unk52 = a;
        break;
    case 3: {
        s8 v = obj->unk52 + a;

        if (v > 31) {
            v = 31;
        }
        if (v < 0) {
            v = 0;
        }
        obj->unk52 = v;
        break;
    }
    }
    return TRUE;
}

/* 0x35 table, idx, value: set unk08 of an object in either object table. */
s32 Scd_Op35(void)
{
    u8 *pc = gScd.pc;
    u8 table = pc[1];
    u8 idx = pc[2];
    u8 value = pc[3];

    gScd.pc = pc + 4;
    switch (table) {
    case 0:
        data_0214d56c[idx]->unk08 = value;
        break;
    case 1:
        data_0214c9e8[idx]->unk08 = value;
        break;
    }
    return TRUE;
}

/* 0x36 slot, op, value: compare object slot's unk92 with value. */
s32 Scd_Op36(void)
{
    u8 *pc = gScd.pc;
    u8 slot = pc[1];
    u8 op = pc[2];
    u8 value = pc[3];

    gScd.pc = pc + 4;
    switch (op) {
    case 0:
        return value == data_0214d56c[slot]->unk92;
    case 1:
        return data_0214d56c[slot]->unk92 > value;
    case 2:
        return data_0214d56c[slot]->unk92 >= value;
    case 3:
        return data_0214d56c[slot]->unk92 < value;
    case 4:
        return data_0214d56c[slot]->unk92 <= value;
    case 5:
        return value != data_0214d56c[slot]->unk92;
    }
    return FALSE;
}

/* 0x38 invert, u16 mask: test bits of data_0213a71c.unk20. */
s32 Scd_Op38(void)
{
    u8 *pc = gScd.pc;
    u8 invert = pc[1];
    u16 mask = *(u16 *)(pc + 2);

    gScd.pc = pc + 4;
    if (invert) {
        return (data_0213a71c.unk20 & mask) == 0;
    }
    return data_0213a71c.unk20 & mask;
}

/* 0x39 id: copy object id's unk0A into data_0213a71c.unk0A. */
s32 Scd_Op39(void)
{
    u8 *pc = gScd.pc;
    u8 id = pc[1];

    gScd.pc = pc + 2;
    data_0213a71c.unk0A = func_02053d0c(id)->unk0A;
    return TRUE;
}

/* 0x3A idx, a, b: set room link idx. */
s32 Scd_Op3A(void)
{
    u8 *pc = gScd.pc;
    u8 idx = pc[1];
    u8 a = pc[2];
    u8 b = pc[3];

    gScd.pc = pc + 4;
    data_0214b640.room->links[idx].unk02 = a;
    data_0214b640.room->links[idx].unk00 = b;
    return TRUE;
}

/* 0x3B id, s16 a, s16 b: set two fields of object id if it is active. */
s32 Scd_Op3B(void)
{
    u8 *pc = gScd.pc;
    u8 id;
    Obj *obj;

    gScd.pc = pc + 6;
    id = pc[1];
    if (id < 0x80) {
        obj = data_0214c9e8[id];
    } else {
        obj = data_0214d56c[id & 0x7F];
    }
    if (obj->unk08) {
        obj->unk42 = *(s16 *)(pc + 2);
        obj->unk46 = *(s16 *)(pc + 4);
    }
    return TRUE;
}

/* 0x3C kind, idx, u16 dist: TRUE if the saved point is within dist of an object. */
s32 Scd_Op3C(void)
{
    u8 *pc = gScd.pc;
    u8 kind = pc[2];
    u8 idx = pc[3];
    u16 dist = *(u16 *)(pc + 4);
    s32 *pos;
    s32 dx;
    s32 dz;

    gScd.pc = pc + 6;
    switch (kind) {
    case 0:
        pos = &func_02053d0c(idx)->posX;
        break;
    case 1:
        pos = &data_0214d56c[idx]->posX;
        break;
    case 2:
        pos = &data_0214c9e8[idx]->posX;
        break;
    }
    dx = data_02143640.unkF74[0] - pos[0];
    dz = data_02143640.unkF74[2] - pos[2];
    return dist >= Sqrt(dx * dx + dz * dz);
}

/* 0x3F ?, u16 lo, u16 hi: TRUE if data_02144540.unk88 is in [lo, hi]. */
s32 Scd_Op3F(void)
{
    u8 *pc = gScd.pc;
    s16 cur = data_02144540.unk88;
    u16 lo;
    u16 hi;

    gScd.pc = pc + 6;
    lo = *(u16 *)(pc + 2);
    hi = *(u16 *)(pc + 4);
    return (u16)(cur - lo) <= hi - lo;
}

/* 0x40 idx, s16 a, b, c, d, e, f, g: set room area idx. */
s32 Scd_Op40(void)
{
    u8 *pc = gScd.pc;
    RoomData *room = data_0214b640.room;
    RoomArea *area;

    gScd.pc = pc + 0x10;
    area = &room->areas[pc[1]];
    area->unk00 = *(s16 *)(pc + 2);
    area->unk04 = *(s16 *)(pc + 4);
    area->unk08 = *(s16 *)(pc + 6);
    area->unk0C = *(s16 *)(pc + 8);
    area->unk10 = *(s16 *)(pc + 0xA);
    area->unk14 = *(s16 *)(pc + 0xC);
    area->unk20 = *(s16 *)(pc + 0xE);
    return TRUE;
}

/* 0x41 who, u16 value: set data_02144540.unkDE (who 0) or object who-1's unk9A. */
s32 Scd_Op41(void)
{
    u8 *pc = gScd.pc;
    u8 who = pc[1];
    u16 value = *(u16 *)(pc + 2);

    gScd.pc = pc + 4;
    if (who == 0) {
        data_02144540.unkDE = value;
    } else {
        func_02053d0c(who - 1)->unk9A = value;
    }
    return TRUE;
}

/* 0x45 delta: data_02144540.unkDE += delta. */
s32 Scd_Op45(void)
{
    u8 *pc = gScd.pc;
    s16 cur = data_02144540.unkDE;
    u8 delta = pc[1];

    gScd.pc = pc + 2;
    data_02144540.unkDE = cur + delta;
    return TRUE;
}

/* Operands of opcode 0x46, one per room point. */
typedef struct ScdPointArg {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 a;
    /* 0x07 */ u8 b;
    /* 0x08 */ u8 c;
    /* 0x09 */ u8 d;
    /* 0x0A */ u16 e;
} ScdPointArg; /* size 0xC */

/* 0x46: set the room's three points and three values (44-byte instruction). */
s32 Scd_Op46(void)
{
    u8 *pc = gScd.pc;
    u32 i;
    ScdPointArg *src;
    u16 *vals;

    src = (ScdPointArg *)(pc + 2);
    gScd.pc = pc + 0x2C;
    for (i = 0; i < 3; i++) {
        ScdPointArg *s = src++;

        data_0214b640.room->points[i].x = s->x;
        data_0214b640.room->points[i].y = s->y;
        data_0214b640.room->points[i].z = s->z;
        data_0214b640.room->points[i].unk0C = s->a;
        data_0214b640.room->points[i].unk0D = s->b;
        data_0214b640.room->points[i].unk0E = s->c;
        data_0214b640.room->points[i].unk10 = s->d;
        data_0214b640.room->points[i].unk12 = s->e;
    }
    vals = (u16 *)src;
    for (i = 0; i < 3; i++) {
        data_0214b640.room->unk06[i] = *vals;
        vals++;
    }
    func_020ae8f4();
    return TRUE;
}

/* Clear every data_02143640.unkA8 entry whose group bit is set in data_0213a6a8. */
void Scd_ClearMaskedGroups(void)
{
    u32 i;
    u32 group;
    u16 mask = data_0213a6a8;

    for (group = 0; group < 16; group++, mask >>= 1) {
        if (mask & 1) {
            for (i = 0; i < data_0214b640.room->count; i++) {
                UnkGroupEntry *entry = &data_02143640.unkA8[i];

                if (group == entry->group) {
                    entry->active = 0;
                }
            }
        }
    }
    data_0213a6a8 = 0;
}

/* 0x49 group: add group to data_0213a6a8, or clear the mask (0xFF). */
s32 Scd_Op49(void)
{
    u8 *pc = gScd.pc;
    u8 group = pc[1];

    gScd.pc = pc + 2;
    if (group == 0xFF) {
        data_0213a6a8 = 0;
    } else {
        data_0213a6a8 |= 1 << group;
    }
    return TRUE;
}

/* 0x4C mode, ref, idx: copy a byte between data_0213a71c and a referenced script. */
s32 Scd_Op4C(void)
{
    u8 *pc = gScd.pc;
    u8 ref = pc[2];
    u8 mode = pc[1];
    u8 idx = pc[3];
    u8 *script;

    gScd.pc = pc + 4;
    script = data_02147d14[ref].script;
    switch (mode) {
    case 1:
        script[9] = ((u8 *)&data_0213a71c)[idx];
        break;
    case 0:
        ((u8 *)&data_0213a71c)[idx] = script[9];
        break;
    case 2: {
        s32 n = func_02074cf0(script[8]);

        if (n >= 0) {
            script[9] = data_02147640.unk538[n][1];
            ((u8 *)&data_0213a71c)[idx] = data_02147640.unk538[n][1];
        }
        break;
    }
    }
    return TRUE;
}

/* 0x4D: 2-byte instruction with no effect on DS. */
s32 Scd_Op4D(void)
{
    gScd.pc += 2;
    return TRUE;
}
