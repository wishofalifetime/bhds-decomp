#ifndef BHDS_SCD_DATA_H
#define BHDS_SCD_DATA_H

#include "types.h"

/*
 * Globals touched by several room script opcodes. Layouts are partial and
 * named by JP address until their purpose is known.
 */

typedef struct UnkStruct_0213a71c {
    /* 0x00 */ u8 unk00[0x02];
    /* 0x02 */ u8 unk02;         /* saved to unk04 by opcode 0x09, restored by 0x0A */
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u8 unk04;
    /* 0x05 */ u8 unk05[0x08 - 0x05];
    /* 0x08 */ u8 unk08;
    /* 0x09 */ u8 unk09[0x12 - 0x09];
    /* 0x12 */ u8 unk12;         /* tested by opcode 0x10 */
    /* 0x13 */ u8 unk13;         /* tested by opcode 0x11 */
    /* 0x14 */ u8 unk14[0x16 - 0x14];
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ u8 unk1A[0x20 - 0x1A];
    /* 0x20 */ u16 unk20;
    /* 0x22 */ u16 unk22;
} UnkStruct_0213a71c;

extern UnkStruct_0213a71c data_0213a71c;  /* JP 0x0213A71C */

typedef struct UnkEntry_0213b850 {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 unk01;
} UnkEntry_0213b850;

typedef struct UnkStruct_0213b750 {
    /* 0x000 */ u8 unk000[0x44];
    /* 0x044 */ u32 flags;          /* bits 0-1 set by opcode 0x0F, bit 20 by 0x09/0x0A */
    /* 0x048 */ u8 unk048[0x100 - 0x48];
    /* 0x100 */ UnkEntry_0213b850 entries[0x30];
} UnkStruct_0213b750;

extern UnkStruct_0213b750 data_0213b750;  /* JP 0x0213B750 */

/* Bit flags, most significant bit first within each 32-bit word. */
u32 Flag_Test(u32 *flags, u32 id);   /* JP 0x020B35E0 */
void Flag_Clear(u32 *flags, u32 id); /* JP 0x020B35FC */
void Flag_Set(u32 *flags, u32 id);   /* JP 0x020B3620 */

extern u32 *data_02121fd8[];  /* JP 0x02121FD8: flag arrays used by opcodes 0x04/0x05 */

#endif /* BHDS_SCD_DATA_H */
