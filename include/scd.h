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

/*
 * A script task. gScd.tasks[0] is the head of a doubly linked list of
 * running tasks (next/prev are pool indices, 0 ends the list).
 */
typedef struct ScdTask {
    /* 0x00 */ u8 type;         /* 0: free; selects the run condition */
    /* 0x01 */ u8 next;
    /* 0x02 */ u8 prev;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ void *target;    /* object the task acts on */
    /* 0x08 */ u8 *pc;          /* current opcode */
} ScdTask; /* size 0xC */

#define SCD_TASK_COUNT 0x80


/* Interpreter state. JP 0x0213D428. Only the fields used so far are known. */
typedef struct ScdState {
    /* 0x000 */ u8 *pc;          /* current instruction */
    /* 0x004 */ u8 unk004[0x10 - 0x04];
    /* 0x010 */ u8 *stack[0x10];   /* block stack: end address of each open block */
    /* 0x050 */ u8 **stackPtr;   /* next free slot in stack */
    /* 0x054 */ u8 unk054[4];
    /* 0x058 */ void *target;      /* set by opcode 0x3D, released by 0x3E */
    /* 0x05C */ u8 unk05C[0x74 - 0x5C];
    /* 0x074 */ u16 unk74;         /* low 4 bits changed by the task runner */
    /* 0x076 */ u8 unk076[0x23C - 0x76];
    /* 0x23C */ u8 seq[4];         /* set by Scd_SetSeq */
    /* 0x240 */ s32 unk240;        /* set by opcode 0x2B */
    /* 0x244 */ u8 unk244;
    /* 0x245 */ u8 unk245;         /* set by Scd_SetSeq, cleared by 0x33 cmd 1 */
    /* 0x246 */ u8 unk246;
    /* 0x247 */ u8 unk247;       /* set by opcode 0x29 */
    /* 0x248 */ u8 depth;        /* nesting level */
    /* 0x249 */ u8 unk249;
    /* 0x24A */ u8 objCount;     /* incremented by opcode 0x1F */
    /* 0x24B */ u8 itemCount;    /* incremented by opcode 0x18 */
    /* 0x24C */ u8 unk24C[0x24E - 0x24C];
    /* 0x24E */ u8 testKind;     /* set by opcode 0x53 */
    /* 0x24F */ u8 testValue;    /* compared by opcode 0x54 */
    /* 0x250 */ u8 unk250[4];
    /* 0x254 */ ScdTask tasks[SCD_TASK_COUNT];
    /* 0x854 */ u8 unk854[0x890 - 0x854];
    /* 0x890 */ s32 savedPos[3];   /* player position, saved each frame */
    /* 0x89C */ u8 unk89C[0x8C8 - 0x89C];
    /* 0x8C8 */ u8 unk8C8;       /* set by opcode 0x17 */
} ScdState;

/*
 * Opcodes that Scd_ExecOpcode runs as a separate task. The handler picks
 * the task's target object and returns the instruction length when the
 * descriptor has none.
 */
typedef s32 (*ScdDescHandler)(u8 *pc, ScdTask *task);

typedef struct ScdOpcodeInfo {
    /* 0x00 */ u8 opcode;
    /* 0x01 */ u8 length;       /* 0: the handler returns it */
    /* 0x02 */ u8 unk02;        /* copied to the new task */
    /* 0x03 */ u8 unk03;
    /* 0x04 */ ScdDescHandler handler;
} ScdOpcodeInfo;

extern ScdHandler gScdOpcodeTable[SCD_OPCODE_COUNT];  /* JP 0x02122200 */
extern ScdOpcodeInfo gScdOpcodeInfo[];                /* JP 0x02122014 */
extern ScdState gScd;                                 /* JP 0x0213D428 */

s32 Scd_LookupPair(u8 table, u8 key); /* JP 0x020B1A2C */
s32 Scd_Step(ScdTask *task);       /* JP 0x020B1A60 */
s32 Scd_ExecDesc(u8 *pc, ScdOpcodeInfo *info);  /* JP 0x020B1BB8 */
void Scd_SetItem(u8 *pc);           /* JP 0x020B1C14 */
s32 Scd_ExecOpcode(u32 op);         /* JP 0x020B3640 */
void Scd_SetSeq(u8 a, u8 b, u8 c, u8 d);  /* JP 0x020B4C7C */
u32 Scd_IsSpawned(u8 *insn);        /* JP 0x020B341C */

#endif /* BHDS_SCD_H */
