/*
 * sndse2.c - the five sound effect channels: loading and playback.
 * JP (ABHJ) 0x020B79B4 - 0x020B7BD0. Built with -O2 (see the pragma).
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "snd.h"

#pragma optimization_level 2

/* Not yet identified. JP addresses. */
extern void func_020b82cc(void *data);
extern void *func_020b82e4(u32 size);
extern void func_0200d564(s32 a, u32 id, u32 pitch, u32 b);
extern void func_0200d5a0(s32 a, s16 id, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, s32 pitch);
extern void func_0200d65c(u32 a);
extern void func_0200d6a8(s32 a, s16 id, u32 frames);
extern void func_0200d764(u16 id, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern s32 func_0200d7f4(u32 x);
extern s32 func_0200d804(u32 x);
extern void func_0200d8a4(u32 id);
extern void func_0200d924(u32 id, void *buf);
extern s32 func_0200d9e0(u32 id);

extern s16 data_02123442[];  /* JP 0x02123442: by pitch step */

/* Restart effect i if it is loaded. */
void SndSe_Play(s32 i)
{
    SndChannel *c = &data_0213a548.se[i];

    if (c->id != 0) {
        if (c->state != 0) {
            func_0200d6a8(-1, c->id, 0);
        }
        func_0200d764(c->id, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        c->status = 1;
        c->state = 1;
    }
}

void SndSe_Stop(s32 i)
{
    SndChannel *c = &data_0213a548.se[i];

    if (c->id != 0 && c->state != 0) {
        func_0200d6a8(-1, c->id, 0);
        c->status = 0;
        c->state = 0;
    }
}

/* Load effect id into slot i and play it. */
void SndSe_Load(s32 i, u16 id)
{
    SndChannel *c = &data_0213a548.se[i];
    s32 size;

    SndCh_StopAll();
    SndSe_Free(i);
    size = func_0200d9e0(id);
    if (size > 0) {
        c->buf = func_020b82e4(size);
        if (c->buf != NULL) {
            c->id = id;
            func_0200d924(id, c->buf);
            func_0200d764(id, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        }
        c->status = 1;
        c->state = 1;
    }
}

void SndSe_Free(s32 i)
{
    SndChannel *c = &data_0213a548.se[i];

    func_0200d6a8(-1, c->id, 0);
    if (c->buf != NULL) {
        func_020b82cc(c->buf);
        c->buf = NULL;
        func_0200d8a4(c->id);
    }
    c->id = 0;
    c->status = 0;
    c->state = 0;
}

void SndSe_StopAll(void)
{
    s32 i;

    func_0200d65c(0);
    for (i = 0; i < 5; i++) {
        SndSe_Free(i);
    }
}
