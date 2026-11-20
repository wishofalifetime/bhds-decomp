#ifndef BHDS_SCD_DATA_H
#define BHDS_SCD_DATA_H

#include "types.h"
#include "obj.h"

/*
 * Globals touched by several room script opcodes. Layouts are partial and
 * named by JP address until their purpose is known.
 */

typedef struct UnkStruct_0213a71c {
    /* 0x00 */ u8 stage;
    /* 0x01 */ u8 room;
    /* 0x02 */ u8 unk02;         /* saved to unk04 by opcode 0x09, restored by 0x0A */
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u8 unk04;
    /* 0x05 */ u8 unk05[0x08 - 0x05];
    /* 0x08 */ u8 unk08;
    /* 0x09 */ u8 unk09;
    /* 0x0A */ u8 unk0A;         /* copied from an object by opcode 0x39 */
    /* 0x0B */ u8 unk0B;         /* object id targeted by opcode 0x3D */
    /* 0x0C */ u8 unk0C[0x12 - 0x0C];
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
    /* 0x048 */ u32 flags2;         /* bits 17 and 28 checked by the BGM code */
    /* 0x04C */ u8 unk04C[0x100 - 0x4C];
    /* 0x100 */ UnkEntry_0213b850 entries[0x30];
} UnkStruct_0213b750;

extern UnkStruct_0213b750 data_0213b750;  /* JP 0x0213B750 */

/* Per-room data reached through data_0214b640.room. Partial. */
typedef struct RoomPoint {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
    /* 0x0C */ u8 unk0C;
    /* 0x0D */ u8 unk0D;
    /* 0x0E */ u8 unk0E;
    /* 0x0F */ u8 pad0F;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u16 unk12;
} RoomPoint; /* size 0x14, set by opcode 0x46 */

typedef struct RoomArea {
    /* 0x00 */ s32 unk00;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 unk0C;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ u8 unk18[8];
    /* 0x20 */ s32 unk20;
    /* 0x24 */ u8 unk24[8];
} RoomArea; /* size 0x2C, set by opcode 0x40 */

typedef struct RoomLink {
    /* 0x00 */ u16 unk00;
    /* 0x02 */ u16 unk02;
    /* 0x04 */ u8 unk04[0x10];
} RoomLink; /* size 0x14 */

typedef struct RoomGroupItem {
    /* 0x00 */ u16 unk00;
    /* 0x02 */ u16 unk02;
    /* 0x04 */ u16 unk04;
    /* 0x06 */ u16 unk06;
    /* 0x08 */ u16 unk08;
    /* 0x0A */ u16 flags;        /* bits 8-11 set by opcode 0x30 */
} RoomGroupItem; /* size 0xC */

/*
 * Collision boxes of a room, split into 4 groups. In the room file
 * groups[0] is the total and groups[1..4] are counts; Coll_InitRoom turns
 * them into pointers, so group i runs from groups[i] to groups[i + 1].
 */
typedef struct RoomGroupTable {
    /* 0x00 */ s16 unk00;
    /* 0x02 */ s16 unk02;
    /* 0x04 */ RoomGroupItem *groups[5];
} RoomGroupTable; /* RoomGroupItem data follows */

typedef struct RoomData {
    /* 0x00 */ u8 count;         /* entries in data_02143640.unkA8 to scan */
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u8 unk02;         /* one more than the highest object level (0x1F) */
    /* 0x03 */ u8 unk03;         /* one more than the highest item slot (0x18) */
    /* 0x04 */ u8 unk04[2];
    /* 0x06 */ u16 unk06[3];
    /* 0x0C */ RoomPoint points[3];
    /* 0x48 */ struct RoomLink *links;  /* 0x14-byte entries, set by opcode 0x3A */
    /* 0x4C */ struct RoomGroupTable *groups;  /* set by opcode 0x30 */
    /* 0x50 */ u8 unk50[0x70 - 0x50];
    /* 0x70 */ RoomArea areas[1]; /* real length unknown */
} RoomData;

typedef struct UnkStruct_0214b640 {
    /* 0x000 */ u8 unk000[0x7A0];
    /* 0x7A0 */ u8 unk7A0;       /* cleared by opcode 0x48 */
    /* 0x7A1 */ u8 unk7A1[0x7A9 - 0x7A1];
    /* 0x7A9 */ u8 unk7A9;
    /* 0x7AA */ u8 unk7AA;
    /* 0x7AB */ u8 unk7AB;
    /* 0x7AC */ RoomData *room;
} UnkStruct_0214b640;

extern UnkStruct_0214b640 data_0214b640;  /* JP 0x0214B640 */

typedef struct UnkStruct_02144540 {
    /* 0x00 */ u8 unk00[0x3C];
    /* 0x3C */ u16 unk3C;
    /* 0x3E */ u16 unk3E;
    /* 0x40 */ u16 unk40;
    /* 0x42 */ u8 unk42[0x80 - 0x42];
    /* 0x80 */ s16 initX;        /* set with the position by opcode 0x20 */
    /* 0x82 */ s16 initY;
    /* 0x84 */ s16 initZ;
    /* 0x86 */ s16 unk86;
    /* 0x88 */ s16 unk88;        /* range-checked by opcode 0x3F */
    /* 0x8A */ s16 unk8A;
    /* 0x8C */ u8 unk8C[0xD8 - 0x8C];
    /* 0xD8 */ s16 unkD8;
    /* 0xDA */ u8 unkDA[0xDE - 0xDA];
    /* 0xDE */ s16 unkDE;        /* set by 0x41, adjusted by 0x45 */
} UnkStruct_02144540;

extern UnkStruct_02144540 data_02144540;  /* JP 0x02144540 */

typedef struct UnkEntry_02147d0c {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u16 unk02;
    /* 0x04 */ u16 unk04;
    /* 0x06 */ u16 unk06;
    /* 0x08 */ u8 *script;       /* points into the running script */
} UnkEntry_02147d0c; /* size 0xC */

typedef struct UnkStruct_02147640 {
    /* 0x000 */ u8 unk000[0x538];
    /* 0x538 */ u8 (*unk538)[2]; /* (type, value) pairs */
    /* 0x53C */ u8 unk53C[4];
    /* 0x540 */ u8 unk540;        /* number of unk538 pairs */
    /* 0x541 */ u8 unk541[0x84C - 0x541];
    /* 0x84C */ UnkEntry_02147d0c *lastEntry;  /* highest data_02147d0c entry used */
} UnkStruct_02147640;

extern UnkEntry_02147d0c data_02147d0c[];  /* JP 0x02147D0C */
extern UnkStruct_02147640 data_02147640;   /* JP 0x02147640 */

typedef struct UnkStruct_0213b214 {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 unk01;
    /* 0x02 */ u8 unk02;
    /* 0x03 */ u8 unk03;
    /* 0x04 */ u8 unk04[0x16 - 0x04];
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17[0x94 - 0x17];
    /* 0x94 */ u8 unk94;         /* set by opcode 0x33 cmd 12 */
} UnkStruct_0213b214;

extern UnkStruct_0213b214 data_0213b214;  /* JP 0x0213B214 */

typedef struct UnkStruct_02144640 {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u8 unk04;
    /* 0x05 */ u8 unk05[0x30 - 0x05];
    /* 0x30 */ u16 unk30;        /* bits 2-3 cleared by opcode 0x20 */
    /* 0x32 */ u8 unk32[0xAE - 0x32];
    /* 0xAE */ u8 unkAE;
    /* 0xAF */ u8 unkAF[0xE3 - 0xAF];
    /* 0xE3 */ u8 unkE3;
} UnkStruct_02144640;

extern UnkStruct_02144640 data_02144640;  /* JP 0x02144640 */

/* Object tables: ids 0x00-0x7F and 0x80-0xBF (64 slots, also used by 0x1F). */
extern Obj *data_0214c9e8[];  /* JP 0x0214C9E8 */
extern Obj *data_0214d56c[];  /* JP 0x0214D56C */

/* Bit flags, most significant bit first within each 32-bit word. */
u32 Flag_Test(u32 *flags, u32 id);   /* JP 0x020B35E0 */
void Flag_Clear(u32 *flags, u32 id); /* JP 0x020B35FC */
void Flag_Set(u32 *flags, u32 id);   /* JP 0x020B3620 */

extern u32 *data_02121fd8[];  /* JP 0x02121FD8: flag arrays used by opcodes 0x04/0x05 */

#endif /* BHDS_SCD_DATA_H */
