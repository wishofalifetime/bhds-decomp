#ifndef BHDS_GAME_H
#define BHDS_GAME_H

#include "types.h"

/* Main game state. JP: 0x0213AC38 (BSS). Field names are placeholders. */
typedef struct GameState {
    /* 0x00 */ u8 unk00[0x0A];
    /* 0x0A */ u8 unk0A;
    /* 0x0B */ u8 unk0B[0xA4 - 0x0B];
    /* 0xA4 */ u16 frameCount;
    /* 0xA6 */ u16 unkA6;
    /* 0xA8 */ VoidFunc nextScene;
    /* 0xAC */ VoidFunc curScene;
    /* 0xB0 */ u32 unkB0;
    /* 0xB4 */ u8 unkB4;
    /* 0xB5 */ u8 unkB5;
    /* 0xB6 */ u8 unkB6;
    /* 0xB7 */ u8 unkB7;
} GameState;

extern GameState gGame;

/* JP: 0x02143640 */
typedef struct UnkStruct_02143640 {
    /* 0x00 */ u8 unk00[0x0F];
    /* 0x0F */ u8 unk0F;
    /* 0x10 */ u8 unk10;
} UnkStruct_02143640;

extern UnkStruct_02143640 data_02143640;

/* JP: 0x0213AD04 and 0x0213AD24, types unknown */
extern u8 data_0213ad04[];
extern u8 data_0213ad24[];

#endif /* BHDS_GAME_H */
