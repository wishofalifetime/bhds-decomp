/*
 * scd.c - room script (SCD) interpreter core.
 * JP (ABHJ) 0x020B1A2C - 0x020B1F5C: pair lookup, Scd_Step, the
 * descriptor-table handlers and their dispatcher, and item placement.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd.h"
#include "scd_data.h"
#include "obj.h"

/* Not yet identified. JP addresses. */
extern ScdTask *Scd_AllocTask(void);
extern Obj *func_02053d0c(u8 id);
extern u32 Scd_TestOp18Flag(u8 *pc);
extern void func_020a00f4(Obj *obj, u32 id);
extern void func_020a3040(u32 id, ObjModel *model, Obj *obj);
extern void func_020f7d54(void *a, u32 b);
extern void func_020f80b4(void *a, u32 b, float c);
extern u16 func_020181b8(u32 a, u32 b, u32 c, void *target, s32 *pos, u32 e);
extern u32 Item_GetHeight(u8 item);
extern u32 Item_GetMarkerId(u8 item);
extern u32 Item_GetMarker(u8 item, u8 *pc);
extern u32 Item_GetModel(u8 item);

extern u8 *data_02121f8c[];  /* JP 0x02121F8C: (key, value) pair lists */
extern u8 data_02121f1c[];   /* JP 0x02121F1C: opcode 0x28 lengths by command */
extern u8 data_02121f28[];   /* JP 0x02121F28: opcode 0x33 lengths by command */
extern u8 data_02144584[];   /* JP 0x02144584: the player */
extern u8 data_0213dca0[];   /* JP 0x0213DCA0 */
extern u8 data_0213dca4[];   /* JP 0x0213DCA4 */
extern s32 data_027e0054[3]; /* DTCM: position scratch */

/* Marker model whose position is lowered; one higher outside JP (see obj.h). */
#ifdef VERSION_JP
#define ITEM_MARKER_LOW 0x152
#else
#define ITEM_MARKER_LOW 0x153
#endif

/* Opcodes 0x18/0x56: place an item from a 26-byte instruction. */
void Scd_SetItem(u8 *pc)
{
    UnkEntry_02147d0c *entry = &data_02147d0c[pc[1] & 0x7F];
    BOOL present;
    Obj *obj;
    u32 flags = *(u16 *)(pc + 0x18) & 0xFFFE;
    void *target;
    u32 id;

    if (Scd_TestOp18Flag(pc + 2) || pc[0x16] == 0xFF) {
        present = TRUE;
    } else {
        present = FALSE;
    }
    if (present) {
        if (pc[0xA] >= 0x54) {
            entry->unk00 = 0xD;
        } else if (pc[0xA] >= 0x4E) {
            entry->unk00 = 0xF;
        } else {
            entry->unk00 = 4;
        }
    } else {
        entry->unk00 = 0;
    }
    *(u16 *)(pc + 0x18) &= 1;
    entry->unk01 = pc[0x17];
    entry->unk02 = *(u16 *)(pc + 0x18);
    entry->unk04 = pc[0xC];
    entry->unk06 = pc[0x16];
    entry->script = pc + 2;
    if (entry > data_02147640.lastEntry) {
        data_02147640.lastEntry = entry;
    }

    obj = data_0214c9e8[pc[0xC]];
    if (pc[0xD] == 0xFF) {
        obj->parent = NULL;
        target = (u8 *)obj + 0x1C;
    } else if (pc[0xD] == 0xFE) {
        obj->parent = data_0213dca0;
        target = data_0213dca4;
    } else {
        obj->parent = (u8 *)data_0214d56c[pc[0xD]] + 0x18;
        target = (u8 *)data_0214d56c[pc[0xD]] + 0x1C;
    }
    obj->unk92 = 0;
    obj->unk08 = present != FALSE;
    obj->kind = 0;
    obj->unk42 = 0;
    obj->unk44 = *(s16 *)(pc + 0x14);
    obj->unk46 = 0;
    obj->posX = *(s16 *)(pc + 0xE);
    obj->posY = *(s16 *)(pc + 0x10);
    obj->posZ = *(s16 *)(pc + 0x12);
    obj->unk50 = 0x7FFF;

    id = Item_GetModel(pc[0xA]);
    if (id < OBJ_MODEL_COUNT) {
        func_020a00f4(obj, id);
        func_020f7d54(obj->model->unk0C, 0xA);
        func_020f80b4(obj->model->unk0C, 1, 0.5f);
        func_020a3040(id, obj->model, obj);
    } else {
        obj->model = NULL;
    }
    obj->flags = 0x40000000;
    if (pc[1] & 0x80) {
        obj->flags |= 0x40;
        obj->unk07 = 1;
    }

    if (present) {
        u32 n = Item_GetMarker(pc[0xA], pc);

        if (n < 0x32) {
            u32 m = Item_GetMarkerId(pc[0xA]);
            s16 dy;

            if (flags & 0x8000) {
                dy = -(((flags & 0xF0) >> 4) << 5);
            } else {
                dy = -(Item_GetHeight(pc[0xA]) << 5);
            }
            if (pc[0xD] == 0xFF) {
                data_027e0054[0] = 0;
                data_027e0054[1] = dy;
                data_027e0054[2] = 0;
            } else {
                data_027e0054[0] = *(s16 *)(pc + 0xE);
                data_027e0054[1] = *(s16 *)(pc + 0x10) + dy;
                data_027e0054[2] = *(s16 *)(pc + 0x12);
            }
            if (n == ITEM_MARKER_LOW) {
                data_027e0054[2] = -0x60;
            }
            obj->unk92 = func_020181b8(n, m, 0, target, data_027e0054, 0);
        }
    }

    if (pc[0xC] >= data_0214b640.room->unk03) {
        data_0214b640.room->unk03 = pc[0xC] + 1;
    }
    gScd.itemCount++;
}

/* Run a descriptor-table opcode as a new task; returns its length. */
s32 Scd_ExecDesc(u8 *pc, ScdOpcodeInfo *info)
{
    ScdTask *task = Scd_AllocTask();

    task->pc = pc;
    task->type = info->unk02;
    if (info->length != 0) {
        info->handler(pc, task);
        return info->length;
    }
    return info->handler(pc, task);
}

/* 0x20 and 0x2B: act on the player. */
s32 Scd_Desc20(u8 *pc, ScdTask *task)
{
    task->target = data_02144584;
    return 0;
}

/* 0x21: act on object pc[1]. */
s32 Scd_Desc21(u8 *pc, ScdTask *task)
{
    task->target = func_02053d0c(pc[1]);
    return 0xE;
}

/* 0x28: act on object pc[2]; length depends on the command. */
s32 Scd_Desc28(u8 *pc, ScdTask *task)
{
    task->target = func_02053d0c(pc[2]);
    return data_02121f1c[pc[3]];
}

/* 0x33: act on the player; length depends on the command. */
s32 Scd_Desc33(u8 *pc, ScdTask *task)
{
    task->target = data_02144584;
    return data_02121f28[pc[1]];
}

/* 0x34: act on object slot pc[2]. */
s32 Scd_Desc34(u8 *pc, ScdTask *task)
{
    task->target = data_0214d56c[pc[2] & 0x3F];
    return 6;
}

/* 0x35: act on entry pc[2] of either object table. */
s32 Scd_Desc35(u8 *pc, ScdTask *task)
{
    if (pc[1] == 0) {
        task->target = data_0214d56c[pc[2]];
    } else {
        task->target = data_0214c9e8[pc[2]];
    }
    return 4;
}

/* 0x47: act on object slot pc[1]. */
s32 Scd_Desc47(u8 *pc, ScdTask *task)
{
    task->target = data_0214d56c[pc[1]];
    return 0xE;
}

/* 0x4D: act on the player. */
s32 Scd_Desc4D(u8 *pc, ScdTask *task)
{
    task->target = data_02144584;
    return 2;
}

/* 0x4E, 0x25, 0x15, 0x46: no target. */
s32 Scd_DescNone(u8 *pc, ScdTask *task)
{
    task->target = NULL;
    return 0;
}

/* Run the opcode at task->pc and return the handler's status. */
s32 Scd_Step(ScdTask *task)
{
    u8 *pc = task->pc;
    ScdHandler handler = gScdOpcodeTable[*pc];

    gScd.pc = pc;
    return handler();
}

/* Value for key in pair list table (sorted by key), or 0. */
s32 Scd_LookupPair(u8 table, u8 key)
{
    u8 *p = data_02121f8c[table];

    for (; p[0] <= key; p += 2) {
        if (p[0] == key) {
            return p[1];
        }
    }
    return 0;
}
