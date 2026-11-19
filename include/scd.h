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
    /* 0x050 */ u32 *stackPtr;   /* grows up by 4 per nested block */
    /* 0x054 */ u8 unk054[0x248 - 0x54];
    /* 0x248 */ u8 depth;        /* nesting level */
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

s32 Scd_Step(ScdTask *task);  /* JP 0x020B1A60 */

#endif /* BHDS_SCD_H */
