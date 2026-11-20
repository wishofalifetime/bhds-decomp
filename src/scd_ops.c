/*
 * scd_ops.c - room script opcode handlers.
 * JP (ABHJ) 0x020B1F5C - 0x020B291C (one contiguous run of handlers).
 * Opcode 0x17, which follows this run, is in scd_op17.c until it matches.
 *
 * The handlers are laid out in descending opcode order, which mwccarm
 * produces from source written in ascending order, so they appear here
 * in opcode order. Opcodes 0x55/0x56 reuse the 0x1B/0x18 handlers.
 */
#include "types.h"
#include "scd.h"
#include "game.h"
#include "obj.h"
#include "scd_data.h"

/* Not yet identified. JP addresses. */
extern u32 Scd_GetRoomModel(u8 kind);
extern void func_020b641c(u16 id);
extern void func_020b7e0c(u8 a, s32 b, s32 *pos, s8 c);
extern void func_020b7e80(u8 a, s32 b, s8 x, s8 angle, s8 c);
extern Obj *func_02053d0c(u8 id);
extern Obj *func_0205550c(u8 *pc, u8 slot);
extern s32 func_02074cf0(u8 id);
extern void func_02079544(u32 id);
extern void func_020a00f4(Obj *obj, u32 id);
extern void func_020a2d70(Obj *obj);
extern void func_020a3040(u32 id, ObjModel *model, Obj *obj);
extern void func_020f7d54(void *a, u32 b);
extern void func_020f7f04(void *a, u32 b, u32 c);
extern void Bgm_RestoreSlots(void);
extern void Bgm_SaveSlots(void);
extern void func_020b7cf0(u8 a, s16 b);
extern void func_020a0204(u8 arg);
extern void func_020a025c(u8 arg);
extern void func_02087380(void);
extern void func_02087d04(u8 *pc);
extern void func_020c4194(u32 kind, u32 value);

typedef struct UnkStruct_0213d628 {
    /* 0x00 */ u8 unk00[0x4C];
    /* 0x4C */ u16 unk4C;
    /* 0x4E */ u16 unk4E;
    /* 0x50 */ u16 unk50;
    /* 0x52 */ s16 unk52;
} UnkStruct_0213d628;

typedef struct UnkStruct_0213d4a6 {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 unk01[0x37];
} UnkStruct_0213d4a6; /* size 0x38 */

extern u8 data_02121f38[];                 /* JP 0x02121F38 */
extern u16 data_0213a730[];                /* JP 0x0213A730 */
extern u32 data_0213b7ac[];                /* JP 0x0213B7AC: flag words */
extern u8 data_0213b8ce[][0x20];           /* JP 0x0213B8CE */
extern UnkStruct_0213d4a6 data_0213d4a6[]; /* JP 0x0213D4A6 */
extern UnkStruct_0213d628 data_0213d628;   /* JP 0x0213D628 */

extern u16 data_02122084[];                /* JP 0x02122084 */
extern u8 data_0214459c[];                 /* JP 0x0214459C */
extern Obj *data_027e000c;                 /* DTCM: last spawned object */
extern s32 data_027e0054[3];               /* DTCM: position scratch */

/* 0x18 (also 0x56): place an item (26-byte instruction). */
s32 Scd_Op18(void)
{
    u8 *pc = gScd.pc;

    gScd.pc = pc + 0x1A;
    Scd_SetItem(pc);
    return TRUE;
}

/* 0x19 idx, value */
s32 Scd_Op19(void)
{
    u8 *pc;
    u8 idx;
    u8 value;
    Obj *entry;

    pc = gScd.pc;
    idx = pc[1];
    value = pc[2];
    gScd.pc = pc + 4;
    entry = data_0214c9e8[idx];
    entry->unk08 = value;
    return TRUE;
}

/* 0x1A id: TRUE if func_02074cf0(id) finds something. */
s32 Scd_Op1A(void)
{
    u8 *pc = gScd.pc;
    u8 id;

    id = pc[1];
    gScd.pc = pc + 2;
    return func_02074cf0(id) != -1;
}

void Scd_OnSpawn(u32 kind);

/* 0x1B (also 0x55): spawn an object from a 22-byte instruction. */
s32 Scd_Op1B(void)
{
    u8 *pc = gScd.pc;

    gScd.pc = pc + 0x16;
    if (pc[3] == 0xFF || !Scd_IsSpawned(pc)) {
        Obj *obj = func_0205550c(pc, pc[0x12] & 0xF);
        data_027e000c = obj;
        Scd_OnSpawn(obj->kind);
    }
    return TRUE;
}

/* Called after opcode 0x1B spawns an object of the given kind. */
void Scd_OnSpawn(u32 kind)
{
    func_02079544(0);
    switch (kind) {
    case 0:
    case 1:
    case 0x11:
    case 0x16:
        func_02079544(4);
        func_02079544(0x20);
        break;
    case 2:
        func_02079544(4);
        break;
    case 3:
    case 4:
        func_02079544(0x1E);
        break;
    case 5:
        func_02079544(0x1C);
        return;
    case 6:
    case 10:
        break;
    case 8:
        func_02079544(0x1E);
        return;
    case 9:
        func_02079544(0x1D);
        break;
    default:
        return;
    }
    func_02079544(3);
}

/* 0x1C a, s16 b, flags */
s32 Scd_Op1C(void)
{
    u8 *pc = gScd.pc;
    u8 flags;
    u8 a;
    s16 b;

    flags = pc[4];
    a = pc[1];
    b = *(s16 *)(pc + 2);
    gScd.pc = pc + 6;
    data_0213a71c.unk08 = a;
    data_0213a71c.unk18 = b;
    data_0214b640.unk7AB = (flags & 1) ? 0xFF : 0;
    data_0214b640.unk7AA = (flags & 2) ? 0xFF : 0;
    data_0214b640.unk7A9 = (flags & 4) ? 0xFF : 0;
    if (data_0213a71c.unk18 != 0) {
        data_0213a71c.unk16 = data_0213a71c.unk18 > 0 ? 0 : 0x7FFF;
    }
    return TRUE;
}

/* 0x1E ?, a, idx: func_020b641c(a + data_02122084[idx]). */
s32 Scd_Op1E(void)
{
    u8 *pc;
    u8 idx;
    u8 a;
    u16 b;

    pc = gScd.pc;
    idx = pc[3];
    a = pc[2];
    gScd.pc = pc + 4;
    b = data_02122084[idx];
    func_020b641c(a + b);
    return TRUE;
}

/* 0x1F: create a scene object from a 28-byte instruction. */
s32 Scd_Op1F(void)
{
    u8 *pc = gScd.pc;
    Obj *obj;
    u8 parent;
    u32 id;

    gScd.pc = pc + 0x1C;
    obj = data_0214d56c[pc[1] & 0x3F];
    parent = pc[3];
    switch (parent) {
    case 0xFF:
        obj->parent = NULL;
        break;
    case 0xFE:
        obj->parent = data_0214459c;
        break;
    default:
        if (parent < 0x80) {
            obj->parent = (u8 *)data_0214d56c[parent] + 0x18;
        } else {
            obj->parent = (u8 *)func_02053d0c(parent & 0x7F) + 0x18;
        }
        break;
    }

    obj->unk07 = 0;
    obj->unk08 = pc[2];
    obj->kind = pc[1] & 0x7F;
    obj->unk42 = 0;
    obj->unk4E = *(s16 *)(pc + 0xA);
    obj->unk44 = obj->unk4E;
    obj->unk46 = 0;
    obj->unk0C = &obj->unk94;
    obj->unk94 = 0x8000;
    obj->unk96 = *(u16 *)(pc + 0x18);
    obj->unk98 = *(u16 *)(pc + 0x16);
    obj->unk9A = *(u16 *)(pc + 0x1A);
    obj->unk9C = *(u16 *)(pc + 0x16);
    obj->unk9E = *(u16 *)(pc + 0x14);
    obj->unkA0 = *(s16 *)(pc + 0xC);
    obj->unkA4 = *(s16 *)(pc + 0xE);
    obj->unkA8 = *(s16 *)(pc + 0x10);
    obj->unkAC = *(s16 *)(pc + 0x12);
    obj->initX = *(s16 *)(pc + 4);
    obj->posX = obj->initX;
    obj->initY = *(s16 *)(pc + 6);
    obj->posY = obj->initY;
    obj->initZ = *(s16 *)(pc + 8);
    obj->posZ = obj->initZ;
    obj->unk50 = 0x7FFF;

    id = Scd_GetRoomModel(obj->kind);
    if (id < OBJ_MODEL_COUNT) {
        func_020a00f4(obj, id);
        func_020f7d54(obj->model->unk0C, 0xB);
        if (id == OBJ_MODEL_SPECIAL) {
            func_020f7f04(obj->model->unk0C, 2, 0x2B);
        }
        func_020a3040(id, obj->model, obj);
        func_020a2d70(obj);
        if (id >= OBJ_MODEL_TRACKED_LO && id <= OBJ_MODEL_TRACKED_HI) {
            data_0213d628.unk52 = obj->posY;
        }
    } else {
        obj->model = NULL;
    }

    if ((obj->kind & 0xF) >= data_0214b640.room->unk02) {
        data_0214b640.room->unk02 = (obj->kind & 0xF) + 1;
    }
    gScd.objCount++;
    obj->flags = 0x40000000;
    if (pc[2] & 0x10) {
        obj->flags |= 0x40;
        obj->unk07 = 1;
    }
    return TRUE;
}

/* 0x25 ?, arg, on: call one of two functions with arg. */
s32 Scd_Op25(void)
{
    u8 *pc = gScd.pc;
    u8 on = pc[3];
    u8 arg = pc[2];

    gScd.pc = pc + 4;
    if (on) {
        func_020a025c(arg);
    } else {
        func_020a0204(arg);
    }
    return TRUE;
}

/* 0x27: 2-byte instruction with no effect on DS. */
s32 Scd_Op27(void)
{
    gScd.pc += 2;
    return TRUE;
}

/* 0x29 idx */
s32 Scd_Op29(void)
{
    u8 idx;
    u8 kind;

    data_02143640.unk10 = 1;
    idx = gScd.pc[1];
    gScd.pc += 2;
    kind = data_02121f38[idx];
    if (kind == 5) {
        func_02087380();
    }
    func_020c4194(0x20, kind);
    gScd.unk247 = TRUE;
    return TRUE;
}

/* 0x2F: 4-byte instruction with no effect on DS. */
s32 Scd_Op2F(void)
{
    gScd.pc += 4;
    return TRUE;
}

/* 0x31 idx, u16 value: data_0213a730[idx] = value. */
s32 Scd_Op31(void)
{
    u8 *pc = gScd.pc;
    u8 idx = pc[1];
    u16 value = *(u16 *)(pc + 2);

    gScd.pc = pc + 4;
    data_0213a730[idx] = value;
    if (idx == 4) {
        data_0213d628.unk4C = 0;
    }
    return TRUE;
}

/* 0x37 row, col, value: data_0213b8ce[row][col] = value. */
s32 Scd_Op37(void)
{
    u8 *pc = gScd.pc;

    gScd.pc = pc + 4;
    data_0213b8ce[pc[1]][pc[2]] = pc[3];
    return TRUE;
}

/* 0x43: 4-byte instruction with no effect on DS. */
s32 Scd_Op43(void)
{
    gScd.pc += 4;
    return TRUE;
}

/* 0x44 idx: clear entry idx of data_0213d4a6. */
s32 Scd_Op44(void)
{
    u8 *pc = gScd.pc;
    u8 idx = pc[1];

    gScd.pc = pc + 2;
    data_0213d4a6[idx].active = 0;
    return TRUE;
}

/* 0x4A */
s32 Scd_Op4A(void)
{
    gScd.pc += 2;
    Bgm_RestoreSlots();
    return TRUE;
}

/* 0x4B */
s32 Scd_Op4B(void)
{
    gScd.pc += 2;
    Bgm_SaveSlots();
    return TRUE;
}

/* 0x53 kind, u16 value: set up the value that opcode 0x54 tests. */
s32 Scd_Op53(void)
{
    u8 *pc = gScd.pc;
    u8 kind = pc[1];
    u16 value = *(u16 *)(pc + 2);

    gScd.pc = pc + 4;
    if (kind == 2) {
        value = Flag_Test(data_0213b7ac, 0x25) ? 1 : 0;
        value |= Flag_Test(data_0213b7ac, 0x26) ? 2 : 0;
    }
    gScd.testKind = kind;
    data_0213d628.unk50 = value;
    gScd.testValue = 0;
    func_020c4194(kind, value);
    return TRUE;
}

/* 0x54 op, value: compare gScd.testValue with value. */
s32 Scd_Op54(void)
{
    u8 *pc;
    u8 op;
    u8 rhs;
    u8 lhs;

    pc = gScd.pc;
    op = pc[1];
    rhs = pc[2];
    gScd.pc = pc + 4;
    lhs = gScd.testValue;
    switch (op) {
    case 0:
        return lhs == rhs;
    case 1:
        return lhs > rhs;
    case 2:
        return lhs >= rhs;
    case 3:
        return lhs < rhs;
    case 4:
        return lhs <= rhs;
    case 5:
        return lhs != rhs;
    case 6:
        return (lhs & rhs) != 0;
    }
    return FALSE;
}

/* 0x57: 16-byte instruction handled by func_02087d04. */
s32 Scd_Op57(void)
{
    func_02087d04(gScd.pc);
    gScd.pc += 0x10;
    return TRUE;
}

/* 0x58 a, b */
s32 Scd_Op58(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[1];
    s16 value = Scd_LookupPair(a, pc[2]);

    gScd.pc += 4;
    func_020b7cf0(a, value);
    return TRUE;
}

/* 0x59 id: TRUE if id is in data_0213b750.entries. */
s32 Scd_Op59(void)
{
    u8 *pc = gScd.pc;
    u8 id = pc[1];
    s32 i;

    gScd.pc = pc + 2;
    for (i = 0; i < 0x30; i++) {
        if (id == data_0213b750.entries[i].id) {
            return TRUE;
        }
    }
    return FALSE;
}
