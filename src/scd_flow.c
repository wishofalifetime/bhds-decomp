/*
 * scd_flow.c - room script opcode handlers 0x00-0x16, including the
 * control-flow opcodes.
 * JP (ABHJ) 0x020B2A5C - 0x020B36C0: handlers, then the flag functions,
 * spawn-instruction helpers and Scd_ExecOpcode.
 *
 * Handlers are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd.h"
#include "scd_data.h"
#include "game.h"

/* Not yet identified. JP addresses. */
extern void func_0200ed14(void);
extern void func_020794a4(u32 a);
extern void func_02094ec4(u8 id);
extern void func_02094f38(u8 id);
extern void func_02094dc4(u32 a);
extern void func_02094e38(u8 id);
extern void func_02095518(void *obj);
extern void func_02092b10(u8 a, u16 b);
extern void func_020598ac(u8 a, u8 b);
extern void Bgm_ClearSlot(u8 a);
extern void Bgm_SetSlot(u8 a);




typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u8 unk04;
} UnkStruct_0213a6cc;

extern UnkStruct_0213a6cc data_0213a6cc;   /* JP 0x0213A6CC */
extern u32 data_0213b764[];                /* JP 0x0213B764: flags that notify on change */
extern u16 data_0213a730[];                /* JP 0x0213A730 */

void Scd_SetFlag(u32 *flags, u32 id);
void Scd_ClearFlag(u32 *flags, u32 id);

typedef struct UnkStruct_0213b49c {
    /* 0x000 */ u8 unk000[0x1FC];
    /* 0x1FC */ u32 unk1FC_0 : 1;
                u32 notify : 1;
} UnkStruct_0213b49c;

/* An object definition whose collected state is tracked by flag. */
typedef struct ScdObjDef {
    /* 0x000 */ u8 unk000[0x169];
    /* 0x169 */ u8 flags;
    /* 0x16A */ u8 unk16A[0x172 - 0x16A];
    /* 0x172 */ u16 id;
} ScdObjDef;

extern UnkStruct_0213b49c data_0213b49c;   /* JP 0x0213B49C */
extern u32 data_0213b6a4[];                /* JP 0x0213B6A4: bit n-0x40 per object 0x40-0x7F */
extern u32 data_0213b7b4[];                /* JP 0x0213B7B4: spawned objects */
extern u32 data_0213b80c[];                /* JP 0x0213B80C */

/* Run one opcode outside the interpreter loop. */
s32 Scd_ExecOpcode(u32 op)
{
    u8 i;

    for (i = 0; gScdOpcodeInfo[i].opcode != 0x5A; i++) {
        if (op == gScdOpcodeInfo[i].opcode) {
            gScd.pc += Scd_ExecDesc(gScd.pc, &gScdOpcodeInfo[i]);
            return TRUE;
        }
    }
    return gScdOpcodeTable[op]();
}

void Flag_Set(u32 *flags, u32 id)
{
    flags[id >> 5] |= 1 << (31 - (id & 0x1F));
}

void Flag_Clear(u32 *flags, u32 id)
{
    flags[id >> 5] &= ~(1 << (31 - (id & 0x1F)));
}

u32 Flag_Test(u32 *flags, u32 id)
{
    return flags[id >> 5] & (1 << (31 - (id & 0x1F)));
}

/* Record that obj has been dealt with, notifying for ids 0x40-0x7F. */
void Scd_MarkObjFlag(ScdObjDef *obj)
{
    if (data_0213a6cc.unk04 != 0) {
        if (!Flag_Test(data_0213b7b4, obj->id) || obj->id == 0xFF) {
            if (obj->id >= 0x40 && obj->id != 0xFF) {
                if (data_0213b49c.notify == 1) {
                    func_02094e38(obj->id);
                }
                if (obj->id < 0x80) {
                    s32 n = obj->id - 0x40;

                    if (!(data_0213b6a4[n / 32] & (1 << (n & 0x1F)))) {
                        if (obj->id >= 0x60) {
                            if (obj->flags & 2) {
                                func_02094dc4(2);
                                n = obj->id - 0x40;
                                data_0213b6a4[n / 32] |= 1 << (n & 0x1F);
                            }
                        } else if (obj->id >= 0x40) {
                            if (obj->flags & 4) {
                                func_02094dc4(1);
                                n = obj->id - 0x40;
                                data_0213b6a4[n / 32] |= 1 << (n & 0x1F);
                            }
                        }
                    }
                }
            }
            func_02095518(obj);
        }
    }
    Flag_Set(data_0213b7b4, obj->id);
}

/* TRUE if the object of spawn instruction insn (0x1B/0x55) already exists. */
u32 Scd_IsSpawned(u8 *insn)
{
    if (insn[0] != 0x55) {
        return Flag_Test(data_0213b7b4, insn[3]);
    }
    return Flag_Test(data_0213b7b4, insn[3] + 0x100);
}

/* Object id of the 0x18/0x56 instruction that ends just before pc + 0x18. */
u16 Scd_Op18ObjId(u8 *pc)
{
    u8 *insn = pc - 2;

    return insn[0] == 0x56 ? insn[0x16] + 0x100 : insn[0x16];
}

u32 Scd_TestOp18Flag(u8 *pc)
{
    return Flag_Test(data_0213b80c, Scd_Op18ObjId(pc));
}

void Scd_ClearOp18Flag(u8 *pc)
{
    Flag_Clear(data_0213b80c, Scd_Op18ObjId(pc));
}

/* Object id of a spawn instruction; 0x55 addresses ids 0x100-0x1FF. */
u16 Scd_SpawnObjId(u8 *insn)
{
    return insn[0] == 0x55 ? insn[3] + 0x100 : insn[3];
}

/* 0x00: end of script. */
s32 Scd_Op00(void)
{
    gScd.depth = 0;
    return FALSE;
}

/* 0x01 len: open a block of len bytes; its end goes on the block stack. */
s32 Scd_Op01(void)
{
    u8 *pc;
    u8 **sp;
    u8 len;

    pc = gScd.pc;
    len = pc[1];
    gScd.pc = pc + 2;
    sp = gScd.stackPtr;
    gScd.stackPtr = sp + 1;
    *sp = pc + 2 + len;
    gScd.depth++;
    return TRUE;
}

/* 0x02 skip: else - close the block and jump over the else part. */
s32 Scd_Op02(void)
{
    u8 *pc;

    gScd.stackPtr--;
    gScd.depth--;
    pc = gScd.pc;
    gScd.pc = pc + pc[1];
    return TRUE;
}

/* 0x03: end of block. */
s32 Scd_Op03(void)
{
    gScd.stackPtr--;
    gScd.depth--;
    gScd.pc += 2;
    return TRUE;
}

/* 0x04 set, id, invert: test a flag. */
s32 Scd_Op04(void)
{
    u8 *pc = gScd.pc;
    u8 set = pc[1];
    u8 id = pc[2];
    u8 invert = pc[3];

    gScd.pc = pc + 4;
    if (set == 10) {
        return (data_02143640.unkFDA & 0x80) ? TRUE : FALSE;
    }
    return invert ^ (Flag_Test(data_02121fd8[set], id) ? TRUE : FALSE);
}

/* 0x05 set, id, mode: set (0), clear (1) or toggle (2) a flag. */
s32 Scd_Op05(void)
{
    u8 *pc = gScd.pc;
    u8 set = pc[1];
    u8 id = pc[2];
    u8 mode = pc[3];
    u32 *flags;

    gScd.pc = pc + 4;
    if (set == 10) {
        return (data_02143640.unkFDA & 0x80) ? TRUE : FALSE;
    }
    flags = data_02121fd8[set];
    switch (mode) {
    case 0:
        Scd_SetFlag(flags, id);
        break;
    case 1:
        Scd_ClearFlag(flags, id);
        break;
    case 2:
        if (Flag_Test(flags, id)) {
            Scd_ClearFlag(flags, id);
        } else {
            Scd_SetFlag(flags, id);
        }
        break;
    }
    if (set == 6 && id == 0x17 && !Flag_Test(flags, id)) {
        data_0213a71c.unk20 &= 3;
        data_0213a71c.unk22 &= 3;
        data_0213b214.unk03 = 4;
    }
    if (set == 7 && id == 0x11 && mode == 1) {
        func_020794a4(1);
    }
    if (set == 5 && id == 0x11 && Flag_Test(flags, id)) {
        gScd.unk247 = TRUE;
    }
    return TRUE;
}

/* Set a flag; some flags also notify func_02094f38. */
void Scd_SetFlag(u32 *flags, u32 id)
{
    Flag_Set(flags, id);
    if (data_0213a6cc.unk04 != 0 && flags == data_0213b764 && (id & 0x80)) {
        func_02094f38(id);
    }
}

/* Clear a flag; some flags also notify func_02094ec4. */
void Scd_ClearFlag(u32 *flags, u32 id)
{
    Flag_Clear(flags, id);
    if (data_0213a6cc.unk04 != 0 && flags == data_0213b764 && (id & 0x80)) {
        func_02094ec4(id);
    }
}

/* 0x06 idx, op, value: compare byte idx of data_0213a71c with value. */
s32 Scd_Op06(void)
{
    u8 *pc = gScd.pc;
    u8 idx = pc[1];
    u8 op = pc[2];
    u8 rhs = pc[3];
    u8 lhs;

    gScd.pc = pc + 4;
    lhs = ((u8 *)&data_0213a71c)[idx];
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
    }
    return FALSE;
}

/* 0x07 ?, idx, op, u16 value: compare data_0213a730[idx] with value. */
s32 Scd_Op07(void)
{
    u8 *pc = gScd.pc;
    u8 idx = pc[2];
    u8 op = pc[3];
    u16 rhs = *(u16 *)(pc + 4);
    u16 lhs;

    gScd.pc = pc + 6;
    lhs = data_0213a730[idx];
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
    }
    return FALSE;
}

/* 0x08 idx, value: byte idx of data_0213a71c = value. */
s32 Scd_Op08(void)
{
    u8 *pc = gScd.pc;
    u8 idx = pc[1];
    u8 value = pc[2];

    gScd.pc = pc + 4;
    ((u8 *)&data_0213a71c)[idx] = value;
    return TRUE;
}

/* 0x09 value: save data_0213a71c.unk02, set it to value, set flag bit 20. */
s32 Scd_Op09(void)
{
    u8 *pc = gScd.pc;
    u8 saved = data_0213a71c.unk02;
    u8 value = pc[1];

    gScd.pc = pc + 2;
    data_0213a71c.unk04 = saved;
    data_0213a71c.unk02 = value;
    func_0200ed14();
    data_0213b750.flags |= 0x100000;
    return TRUE;
}

/* 0x0A: restore data_0213a71c.unk02 saved by 0x09, clear flag bit 20. */
s32 Scd_Op0A(void)
{
    u8 saved = data_0213a71c.unk04;

    gScd.pc += 2;
    data_0213a71c.unk02 = saved;
    func_0200ed14();
    data_0213b750.flags &= ~0x100000;
    return TRUE;
}

/* 0x0C idx, ..., b: start entry idx of data_02147d0c (26-byte instruction). */
s32 Scd_Op0C(void)
{
    u8 *pc = gScd.pc;
    u8 idx;
    UnkEntry_02147d0c *entry;

    gScd.pc = pc + 0x1A;
    idx = pc[1];
    entry = &data_02147d0c[idx];
    if (entry > data_02147640.lastEntry) {
        data_02147640.lastEntry = entry;
    }
    entry->unk00 = 1;
    entry->unk01 = pc[0x19];
    entry->unk02 = idx;
    entry->script = pc + 2;
    return TRUE;
}

/* 0x0D idx, ...: fill entry idx of data_02147d0c (18-byte instruction). */
s32 Scd_Op0D(void)
{
    u8 *pc = gScd.pc;
    UnkEntry_02147d0c *entry;

    gScd.pc = pc + 0x12;
    entry = &data_02147d0c[pc[1]];
    if (entry > data_02147640.lastEntry) {
        data_02147640.lastEntry = entry;
    }
    entry->unk00 = pc[0xA];
    entry->unk01 = pc[0xB];
    entry->unk02 = *(u16 *)(pc + 0xC);
    entry->unk04 = *(u16 *)(pc + 0xE);
    entry->unk06 = *(u16 *)(pc + 0x10);
    entry->script = pc + 2;
    return TRUE;
}

/* 0x0B a, u16 b */
s32 Scd_Op0B(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[1];
    u16 b = *(u16 *)(pc + 2);

    gScd.pc = pc + 4;
    func_02092b10(a, b);
    return TRUE;
}

/* 0x0F mode, u16 x, u16 y, u16 z */
s32 Scd_Op0F(void)
{
    u8 *pc = gScd.pc;

    gScd.pc = pc + 8;
    data_0213b750.flags &= ~3;
    data_0213b750.flags |= pc[1];
    data_02144540.unk3C = *(u16 *)(pc + 2);
    data_02144540.unk3E = *(u16 *)(pc + 4);
    data_02144540.unk40 = *(u16 *)(pc + 6);
    return TRUE;
}

/* 0x10 value: TRUE if data_0213a71c.unk12 == value. */
s32 Scd_Op10(void)
{
    u8 *pc = gScd.pc;
    u8 cur = data_0213a71c.unk12;
    u8 value = pc[1];

    gScd.pc = pc + 2;
    return cur == value;
}

/* 0x11 value: TRUE if data_0213a71c.unk13 == value. */
s32 Scd_Op11(void)
{
    u8 *pc = gScd.pc;
    u8 cur = data_0213a71c.unk13;
    u8 value = pc[1];

    gScd.pc = pc + 2;
    return cur == value;
}

/* 0x12 idx, a, b, u16 c, u16 d, u16 e: fill entry idx of data_02147d0c. */
s32 Scd_Op12(void)
{
    u8 *pc = gScd.pc;
    UnkEntry_02147d0c *entry;

    gScd.pc = pc + 10;
    entry = &data_02147d0c[pc[1]];
    entry->unk00 = pc[2];
    entry->unk01 = pc[3];
    entry->unk02 = *(u16 *)(pc + 4);
    entry->unk04 = *(u16 *)(pc + 6);
    entry->unk06 = *(u16 *)(pc + 8);
    return TRUE;
}

/* 0x13 idx, a, b: set the first two bytes of entry idx of data_02147d0c. */
s32 Scd_Op13(void)
{
    u8 *pc = gScd.pc;
    u8 idx = pc[1];
    u8 a = pc[2];
    u8 b = pc[3];
    UnkEntry_02147d0c *entry;

    gScd.pc = pc + 4;
    entry = &data_02147d0c[idx];
    entry->unk00 = a;
    entry->unk01 = b;
    return TRUE;
}

/* 0x14 ?, a, b */
s32 Scd_Op14(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[2];
    u8 b = pc[3];

    gScd.pc = pc + 4;
    func_020598ac(a, b);
    return TRUE;
}

/* 0x15 a */
s32 Scd_Op15(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[1];

    gScd.pc = pc + 2;
    Bgm_SetSlot(a);
    return TRUE;
}

/* 0x16 a */
s32 Scd_Op16(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[1];

    gScd.pc = pc + 2;
    Bgm_ClearSlot(a);
    return TRUE;
}
