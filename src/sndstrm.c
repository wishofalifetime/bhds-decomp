/*
 * sndstrm.c - sound system setup and the streamed sound player.
 * JP (ABHJ) 0x020B6E48 - 0x020B7048.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"

typedef struct SndStream {
    /* 0x000 */ u8 unk000[0x114];
    /* 0x114 */ u8 state;       /* 1: stopped, 2: started, 3+: playing */
    /* 0x115 */ u8 unk115[3];
    /* 0x118 */ s16 unk118;
    /* 0x11A */ u8 unk11A[2];
    /* 0x11C */ void *data;
} SndStream;

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[0xA];
    /* 0x0A */ u8 unk0A;
} UnkStruct_0213a6cc;

/* Not yet identified. JP addresses. */
extern s32 func_0200d350(u32 mode);
extern void func_0200d374(u32 a);
extern void func_0200d380(u32 a);
extern BOOL func_0200d38c(void);
extern void func_0200d3b0(u32 a);
extern u32 func_0200d958(u32 id, u32 kind);
extern s32 func_020b70e4(void);
extern void func_020b7410(void);
extern void func_020b79b4(void);
extern void func_020b7f70(void);
extern void func_020b8028(u32 slot, u32 id);
extern void func_020b82cc(void *data);

extern SndStream data_0213a548;             /* JP 0x0213A548 */
extern UnkStruct_0213a6cc data_0213a6cc;    /* JP 0x0213A6CC */

void SndStrm_Reset(void);
void SndStrm_Stop(void);
void SndStrm_Pause(void);
s32 SndStrm_SetMode(u32 mode);

void SndStrm_Start(void)
{
    func_0200d380(0);
    data_0213a548.unk118 = 0;
    data_0213a548.state = 2;
}

void SndStrm_Pause(void)
{
    if (data_0213a548.state >= 3) {
        func_0200d374(0);
        data_0213a548.state = 1;
    }
}

void SndStrm_Stop(void)
{
    SndStream *s = &data_0213a548;

    if (func_0200d38c()) {
        func_0200d374(0);
    }
    if (s->data != NULL) {
        func_0200d3b0(0);
        func_020b82cc(s->data);
    }
    s->data = NULL;
    s->unk118 = 0;
    s->state = 0;
}

s16 SndStrm_GetUnk118(void)
{
    return data_0213a548.unk118;
}

s32 SndStrm_SetMode(u32 mode)
{
    u32 m;

    switch (mode) {
    case 0:
        m = 0;
        break;
    case 1:
        m = 2;
        break;
    case 2:
        m = 3;
        break;
    default:
        return 0;
    }
    return func_0200d350(m);
}

s32 SndStrm_Open(void)
{
    s32 ret = func_020b70e4();

    SndStrm_SetMode(data_0213a6cc.unk0A);
    SndStrm_Start();
    return ret;
}

void SndStrm_Reset(void)
{
    SndStrm_Pause();
    SndStrm_Stop();
}

/* Largest func_0200d958(id, 6) for ids lo..hi. */
u32 Snd_GetMaxSize(u32 lo, u32 hi)
{
    u32 max = 0;

    for (; lo <= hi; lo++) {
        u32 n = func_0200d958(lo, 6);

        if (max < n) {
            max = n;
        }
    }
    return max;
}

void Snd_LoadBank0C(void)
{
    func_020b8028(0xC, 1);
}

void Snd_Init(void)
{
    SndStrm_Reset();
    func_020b7410();
    func_020b79b4();
    func_020b7f70();
}
