/*
 * scd_ops.c - room script opcode handlers 0x2F-0x4B.
 * JP (ABHJ) 0x020B219C - 0x020B231C (one contiguous run of handlers).
 *
 * The handlers are laid out in descending opcode order, which mwccarm
 * produces from source written in ascending order, so they appear here
 * in opcode order.
 */
#include "types.h"
#include "scd.h"

/* Not yet identified. JP addresses. */
extern void func_020b61c0(void);
extern void func_020b61e4(void);

typedef struct UnkStruct_0213d628 {
    /* 0x00 */ u8 unk00[0x4C];
    /* 0x4C */ u16 unk4C;
} UnkStruct_0213d628;

typedef struct UnkStruct_0213d4a6 {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 unk01[0x37];
} UnkStruct_0213d4a6; /* size 0x38 */

extern u16 data_0213a730[];                /* JP 0x0213A730 */
extern u8 data_0213b8ce[][0x20];           /* JP 0x0213B8CE */
extern UnkStruct_0213d4a6 data_0213d4a6[]; /* JP 0x0213D4A6 */
extern UnkStruct_0213d628 data_0213d628;   /* JP 0x0213D628 */

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
