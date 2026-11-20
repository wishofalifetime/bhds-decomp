/*
 * scd_flow.c - room script opcode handlers 0x00-0x16, including the
 * control-flow opcodes.
 * JP (ABHJ) 0x020B2A5C - 0x020B3388 (contiguous run of handlers).
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
extern void func_02092b10(u8 a, u16 b);
extern void func_020598ac(u8 a, u8 b);
extern void func_020b60f8(u8 a);
extern void func_020b612c(u8 a);

typedef struct UnkEntry_02147d0c {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u16 unk02;
    /* 0x04 */ u16 unk04;
    /* 0x06 */ u16 unk06;
    /* 0x08 */ u8 *script;       /* points into the running script */
} UnkEntry_02147d0c; /* size 0xC */

typedef struct UnkStruct_02147640 {
    /* 0x000 */ u8 unk000[0x84C];
    /* 0x84C */ UnkEntry_02147d0c *lastEntry;  /* highest data_02147d0c entry used */
} UnkStruct_02147640;

typedef struct UnkStruct_02144540 {
    /* 0x00 */ u8 unk00[0x3C];
    /* 0x3C */ u16 unk3C;
    /* 0x3E */ u16 unk3E;
    /* 0x40 */ u16 unk40;
} UnkStruct_02144540;

extern UnkEntry_02147d0c data_02147d0c[];  /* JP 0x02147D0C */
extern UnkStruct_02144540 data_02144540;   /* JP 0x02144540 */
extern UnkStruct_02147640 data_02147640;   /* JP 0x02147640 */

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u8 unk04;
} UnkStruct_0213a6cc;

typedef struct UnkStruct_0213b214 {
    /* 0x00 */ u8 unk00[3];
    /* 0x03 */ u8 unk03;
} UnkStruct_0213b214;

extern UnkStruct_0213a6cc data_0213a6cc;   /* JP 0x0213A6CC */
extern UnkStruct_0213b214 data_0213b214;   /* JP 0x0213B214 */
extern u32 data_0213b764[];                /* JP 0x0213B764: flags that notify on change */
extern u16 data_0213a730[];                /* JP 0x0213A730 */

void Scd_SetFlag(u32 *flags, u32 id);
void Scd_ClearFlag(u32 *flags, u32 id);

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
    func_020b612c(a);
    return TRUE;
}

/* 0x16 a */
s32 Scd_Op16(void)
{
    u8 *pc = gScd.pc;
    u8 a = pc[1];

    gScd.pc = pc + 2;
    func_020b60f8(a);
    return TRUE;
}
