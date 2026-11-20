/*
 * scd_ops.c - room script opcode handlers 0x25-0x59 (first run).
 * JP (ABHJ) 0x020B1F5C - 0x020B23E4 (one contiguous run of handlers).
 *
 * The handlers are laid out in descending opcode order, which mwccarm
 * produces from source written in ascending order, so they appear here
 * in opcode order. Opcodes 0x55/0x56 share handlers with 0x1B/0x18 and
 * live elsewhere.
 */
#include "types.h"
#include "scd.h"
#include "game.h"

/* Not yet identified. JP addresses. */
extern s32 func_020b1a2c(u8 a, u8 b);
extern s32 func_020b35e0(void *obj, u32 id);
extern void func_020b61c0(void);
extern void func_020b61e4(void);
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
} UnkStruct_0213d628;

typedef struct UnkStruct_0213d4a6 {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 unk01[0x37];
} UnkStruct_0213d4a6; /* size 0x38 */

typedef struct UnkEntry_0213b850 {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 unk01;
} UnkEntry_0213b850;

typedef struct UnkStruct_0213b750 {
    /* 0x000 */ u8 unk000[0x100];
    /* 0x100 */ UnkEntry_0213b850 entries[0x30];
} UnkStruct_0213b750;

extern u8 data_02121f38[];                 /* JP 0x02121F38 */
extern u16 data_0213a730[];                /* JP 0x0213A730 */
extern UnkStruct_0213b750 data_0213b750;   /* JP 0x0213B750 */
extern u8 data_0213b7ac[];                 /* JP 0x0213B7AC */
extern u8 data_0213b8ce[][0x20];           /* JP 0x0213B8CE */
extern UnkStruct_0213d4a6 data_0213d4a6[]; /* JP 0x0213D4A6 */
extern UnkStruct_0213d628 data_0213d628;   /* JP 0x0213D628 */

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
    func_020b61c0();
    return TRUE;
}

/* 0x4B */
s32 Scd_Op4B(void)
{
    gScd.pc += 2;
    func_020b61e4();
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
        value = func_020b35e0(data_0213b7ac, 0x25) ? 1 : 0;
        value |= func_020b35e0(data_0213b7ac, 0x26) ? 2 : 0;
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
    s16 value = func_020b1a2c(a, pc[2]);

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
