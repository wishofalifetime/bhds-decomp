#ifndef BHDS_SCD_H
#define BHDS_SCD_H

#include "types.h"

/*
 * Room script (SCD) bytecode interpreter.
 * 90 opcodes (0x00-0x59); 0x5A terminates the opcode descriptor table.
 * Each handler reads its operands at gScd.pc, advances gScd.pc past the
 * instruction and returns nonzero to keep running (opcode 0x00 returns 0).
 */
#define SCD_OPCODE_COUNT 0x5A

typedef s32 (*ScdHandler)(void);

/* Interpreter state. JP 0x0213D428. Only the fields used so far are known. */
typedef struct ScdState {
    /* 0x000 */ u8 *pc;          /* current instruction */
    /* 0x004 */ u8 unk004[0x50 - 0x04];
    /* 0x050 */ u8 **stackPtr;   /* block stack: end address of each open block */
    /* 0x054 */ u8 unk054[4];
    /* 0x058 */ void *target;      /* set by opcode 0x3D, released by 0x3E */
    /* 0x05C */ u8 unk05C[0x240 - 0x5C];
    /* 0x240 */ s32 unk240;        /* set by opcode 0x2B */
    /* 0x244 */ u8 unk244;
    /* 0x245 */ u8 unk245;         /* cleared by opcode 0x33 cmd 1 */
    /* 0x246 */ u8 unk246;
    /* 0x247 */ u8 unk247;       /* set by opcode 0x29 */
    /* 0x248 */ u8 depth;        /* nesting level */
    /* 0x249 */ u8 unk249;
    /* 0x24A */ u8 objCount;     /* incremented by opcode 0x1F */
    /* 0x24B */ u8 unk24B[0x24E - 0x24B];
    /* 0x24E */ u8 testKind;     /* set by opcode 0x53 */
    /* 0x24F */ u8 testValue;    /* compared by opcode 0x54 */
    /* 0x250 */ u8 unk250[0x8C8 - 0x250];
    /* 0x8C8 */ u8 unk8C8;       /* set by opcode 0x17 */
} ScdState;

/* A running script. Only the fields used so far are known. */
typedef struct ScdTask {
    /* 0x00 */ u8 unk00[0x08];
    /* 0x08 */ u8 *pc;          /* current opcode */
} ScdTask;

/* Opcodes with alternate handlers when run outside the interpreter loop. */
typedef struct ScdOpcodeInfo {
    /* 0x00 */ u8 opcode;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u16 unk02;
    /* 0x04 */ void *handler;
} ScdOpcodeInfo;

extern ScdHandler gScdOpcodeTable[SCD_OPCODE_COUNT];  /* JP 0x02122200 */
extern ScdOpcodeInfo gScdOpcodeInfo[];                /* JP 0x02122014 */
extern ScdState gScd;                                 /* JP 0x0213D428 */

s32 Scd_Step(ScdTask *task);       /* JP 0x020B1A60 */
s32 Scd_ExecOpcode(u32 op);         /* JP 0x020B3640 */
u32 Scd_IsSpawned(u8 *insn);        /* JP 0x020B341C */

#endif /* BHDS_SCD_H */
