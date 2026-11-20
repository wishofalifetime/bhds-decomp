/*
 * sndstrm.c - sound system setup and the streamed sound player.
 * JP (ABHJ) 0x020B6E48 - 0x020B70E4.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "snd.h"

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
extern void SndSe_StopAll(void);
extern void SndBank_UnloadAll(void);
extern void SndBank_Load(u32 slot, u32 id);
extern BOOL func_0200d398(void);

extern UnkStruct_0213a6cc data_0213a6cc;    /* JP 0x0213A6CC */

void SndStrm_Update(void)
{
    SndStream *s = &data_0213a548;

    if (s->state == 0 || s->state == 1) {
        return;
    }
    if (!func_0200d398()) {
        return;
    }
    if (s->delay == 0) {
        switch (s->state) {
        case 2:
            s->pos = 0;
            if (SndStrm_GetPos() >= s->target) {
                s->state = 3;
            }
            break;
        case 3:
            s->pos = SndStrm_GetPos();
            break;
        }
    } else {
        s->delay--;
    }
}


void SndStrm_Start(void)
{
    func_0200d380(0);
    data_0213a548.pos = 0;
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
        Snd_Free(s->data);
    }
    s->data = NULL;
    s->pos = 0;
    s->state = 0;
}

s16 SndStrm_GetPosCached(void)
{
    return data_0213a548.pos;
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

BOOL SndStrm_Open(s16 target)
{
    BOOL ret = SndStrm_Load(target);

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
    SndBank_Load(0xC, 1);
}

void Snd_Init(void)
{
    SndStrm_Reset();
    SndCh_StopAll();
    SndSe_StopAll();
    SndBank_UnloadAll();
}
