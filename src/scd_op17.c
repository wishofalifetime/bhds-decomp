/*
 * scd_op17.c - room script opcode 0x17, split out of scd_ops.c.
 * JP (ABHJ) 0x020B291C - 0x020B2A5C.
 * Status: NOT matching (64/80). Same instructions, but the original keeps
 * the script pointer in r6 and the first operand in r4 (swapped here).
 * Tried: statement orders, declaration orders, u32 operand, instruction
 * struct. Move back to the top of scd_ops.c once it matches.
 */
#include "types.h"
#include "scd.h"
#include "game.h"
#include "obj.h"

/* Not yet identified. JP addresses. */
extern void func_020b7e0c(u8 a, s32 b, s32 *pos, s8 c);
extern void func_020b7e80(u8 a, s32 b, s8 x, s8 angle, s8 c);
extern Obj *func_02053d0c(u8 id);

extern s32 data_027e0054[3];  /* DTCM: position scratch */

/* 0x17 a, b, ?, c, mode, ...: positioned action. */
s32 Scd_Op17(void)
{
    u8 *pc = gScd.pc;
    u8 a;
    s32 b;

    gScd.pc = pc + 10;
    a = pc[1];
    b = Scd_LookupPair(a, pc[2]);
    if (a == 6 && b == 2) {
        gScd.unk8C8 = 1;
    }
    switch (pc[5]) {
    case 0:
        data_027e0054[0] = *(u16 *)(pc + 6);
        data_027e0054[1] = 0;
        data_027e0054[2] = *(u16 *)(pc + 8);
        break;
    case 1:
        data_027e0054[0] = data_02143640.unkF74[0];
        data_027e0054[1] = data_02143640.unkF74[1];
        data_027e0054[2] = data_02143640.unkF74[2];
        break;
    case 2: {
        Obj *obj = func_02053d0c(pc[6]);
        data_027e0054[0] = obj->posX;
        data_027e0054[1] = obj->posY;
        data_027e0054[2] = obj->posZ;
        break;
    }
    case 3: {
        s8 x = *(s16 *)(pc + 6);
        s8 angle = *(s16 *)(pc + 8);
        func_020b7e80(a, b, x, angle + 0x40, (s8)pc[4]);
        return TRUE;
    }
    }
    func_020b7e0c(a, b, data_027e0054, (s8)pc[4]);
    return TRUE;
}
