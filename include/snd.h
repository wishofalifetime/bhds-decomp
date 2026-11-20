#ifndef BHDS_SND_H
#define BHDS_SND_H

#include "types.h"

/*
 * Sound driver state. JP 0x0213A548. Four sound channels, then the
 * streamed sound player.
 */
typedef struct SndChannel {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 status;       /* 1, 2, 4, 8, 0x10, 0x40, 0x80; 0: idle */
    /* 0x02 */ u8 volume;
    /* 0x03 */ u8 pad03;
    /* 0x04 */ u32 id;          /* sound playing */
    /* 0x08 */ void *buf;
} SndChannel;

typedef struct SndStream {
    /* 0x000 */ u8 unk000[0xE4];
    /* 0x0E4 */ SndChannel chan[4];
    /* 0x114 */ u8 state;       /* 1: loaded, 2: started, 3: playing */
    /* 0x115 */ u8 delay;       /* frames before playback is tracked */
    /* 0x116 */ s16 target;     /* position at which the stream counts as playing */
    /* 0x118 */ s16 pos;
    /* 0x11A */ u8 unk11A[2];
    /* 0x11C */ void *data;
} SndStream;

extern SndStream data_0213a548;  /* JP 0x0213A548 */

/* sndstrm.c */
void Snd_Init(void);                  /* JP 0x020B6E48 */
void SndStrm_Reset(void);             /* JP 0x020B6ECC */
BOOL SndStrm_Open(s16 target);        /* JP 0x020B6EE8 */
s32 SndStrm_SetMode(u32 mode);        /* JP 0x020B6F14 */
void SndStrm_Stop(void);              /* JP 0x020B6F78 */
void SndStrm_Pause(void);             /* JP 0x020B6FD0 */
void SndStrm_Start(void);             /* JP 0x020B700C */
void SndStrm_Update(void);            /* JP 0x020B7048 */

/* sndstrm_load.c */
BOOL SndStrm_Load(s16 target);        /* JP 0x020B70E4 */

/* sndch.c */
s16 SndStrm_GetPos(void);             /* JP 0x020B7178 */
void SndCh_StopAll(void);             /* JP 0x020B7410 */
void SndCh_Stop(u32 ch);              /* JP 0x020B7480 */
void SndCh_SetVolume(u32 ch, u8 volume);  /* JP 0x020B7360 */
void SndCh_SetPan(u32 ch, u8 pan);    /* JP 0x020B7314 */

#endif /* BHDS_SND_H */
