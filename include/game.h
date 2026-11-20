#ifndef BHDS_GAME_H
#define BHDS_GAME_H

#include "types.h"

/* Main game state. JP: 0x0213AC38 (BSS). Field names are placeholders. */
typedef struct GameState {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 unk01;        /* reloads unk02 every frame */
    /* 0x02 */ volatile s8 unk02;  /* decremented by the VBlank handler */
    /* 0x03 */ volatile s8 vblankWait;  /* set by Main_EndFrame, cleared in VBlank */
    /* 0x04 */ u32 vblankCount;
    /* 0x08 */ u8 unk08;
    /* 0x09 */ u8 unk09;        /* nonzero blocks soft reset */
    /* 0x0A */ u8 unk0A;
    /* 0x0B */ u8 unk0B;
    /* 0x0C */ u8 unk0C[0x98];  /* copied from DTCM every frame */
    /* 0xA4 */ u16 frameCount;
    /* 0xA6 */ u16 unkA6;
    /* 0xA8 */ VoidFunc nextScene;
    /* 0xAC */ VoidFunc curScene;
    /* 0xB0 */ u32 unkB0;
    /* 0xB4 */ u8 unkB4;
    /* 0xB5 */ u8 unkB5;
    /* 0xB6 */ u8 unkB6;
    /* 0xB7 */ u8 unkB7;
    /* 0xB8 */ u8 unkB8[0xC8 - 0xB8];
    /* 0xC8 */ void *workBuf;   /* 0x10000 bytes from the main arena */
} GameState; /* size 0xCC */

extern GameState gGame;

/* 0x38-byte entries at data_02143640.unkA8 */
typedef struct UnkGroupEntry {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 group;    /* cleared by Scd_ClearMaskedGroups */
    /* 0x02 */ u8 unk02[0x36];
} UnkGroupEntry;

/* JP: 0x02143640 */
typedef struct UnkStruct_02143640 {
    /* 0x000 */ u8 unk000[0x0F];
    /* 0x00F */ u8 unk0F;
    /* 0x010 */ u8 unk10;
    /* 0x011 */ u8 unk011[0xA8 - 0x011];
    /* 0x0A8 */ UnkGroupEntry unkA8[64];  /* real length unknown */
    /* 0xEA8 */ u8 unkEA8[0xF4A - 0xEA8];
    /* 0xF4A */ u8 unkF4A;       /* set by opcode 0x33 cmd 11 */
    /* 0xF4B */ u8 unkF4B[0xF74 - 0xF4B];
    /* 0xF74 */ s32 unkF74[3];  /* a position, copied by script opcode 0x17 */
    /* 0xF80 */ u8 unkF80[0xFD5 - 0xF80];
    /* 0xFD5 */ u8 unkFD5;       /* passed on by opcode 0x2B */
    /* 0xFD6 */ u8 unkFD6[0xFDA - 0xFD6];
    /* 0xFDA */ u8 unkFDA;       /* bit 7 answers flag set 10 in opcodes 0x04/0x05 */
    /* 0xFDB */ u8 unkFDB;
    /* 0xFDC */ u8 unkFDC;
    /* 0xFDD */ u8 unkFDD[0xFE6 - 0xFDD];
    /* 0xFE6 */ u8 unkFE6;
    /* 0xFE7 */ u8 unkFE7;
} UnkStruct_02143640;

extern UnkStruct_02143640 data_02143640;

/* JP addresses */
void Main_Init(void);             /* 0x02078344 (EU: different code) */
void Main_EndFrame(void);         /* 0x02077FE8 */
void Main_VBlankHandler(void);    /* 0x02078068 */
void Main_InitVBlank(void);       /* 0x020780E4 */
void Main_LidSleep(void);         /* 0x0207812C */
BOOL Main_CheckSoftReset(void);   /* 0x020781E0 */
void Main_InitHeaps(void);        /* 0x0207825C */
void Fade_Update(void);           /* 0x0206FA80 */

#endif /* BHDS_GAME_H */
