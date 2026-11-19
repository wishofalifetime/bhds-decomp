#ifndef BHDS_SCD_H
#define BHDS_SCD_H

#include "types.h"

/*
 * Room script (SCD) bytecode interpreter.
 * 90 opcodes (0x00-0x59); 0x5A terminates the opcode descriptor table.
 */
#define SCD_OPCODE_COUNT 0x5A

typedef void (*ScdHandler)(void);

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
extern u8 *gScdPc;                                    /* JP 0x0213D428 */

void Scd_Step(ScdTask *task);  /* JP 0x020B1A60 */

#endif /* BHDS_SCD_H */
