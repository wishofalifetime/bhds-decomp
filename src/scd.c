/*
 * scd.c - room script (SCD) bytecode interpreter.
 * JP (ABHJ): Scd_Step at 0x020B1A60. Handlers live at 0x020B1F5C-0x020B4B94.
 */
#include "types.h"
#include "scd.h"

/* Run the opcode at task->pc and return the handler's status. */
s32 Scd_Step(ScdTask *task)
{
    u8 *pc = task->pc;
    ScdHandler handler = gScdOpcodeTable[*pc];

    gScd.pc = pc;
    return handler();
}
