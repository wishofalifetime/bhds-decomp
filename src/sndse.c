/*
 * sndse.c - the five sound effect channels: state and volume.
 * JP (ABHJ) 0x020B77C8 - 0x020B79B4.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "snd.h"

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

void SndSe_FadeOut(s32 i, u32 frames)
{
    SndChannel *c = &data_0213a548.se[i];

    if (c->status != 0) {
        func_0200d6a8(-1, c->id, frames);
        c->status = 4;
        c->state = 4;
    }
}

void SndSe_PauseAll(u32 a)
{
    s32 i;

    func_0200d65c(a);
    for (i = 0; i < 5; i++) {
        SndChannel *c = &data_0213a548.se[i];

        if (c->status != 0) {
            c->status = 4;
            c->state = 4;
        }
    }
}

/* Map the driver's effect state for x to the channel status bits. */
u32 SndSe_GetPlayState(u32 x)
{
    s32 a = func_0200d804(x);

    switch (func_0200d7f4(x)) {
    case 0:
        return 2;
    case 1:
        return 4;
    case -1:
        return 0;
    case 2:
        return 8;
    case 3:
        switch (a) {
        case 1:
            return 0x40;
        case 2:
            return 0;
        default:
            return 1;
        }
    default:
        return 0;
    }
}

u8 SndSe_GetStatus(s32 i)
{
    return data_0213a548.se[i].status;
}

/* Set effect i's pitch step (0-400). */
void SndSe_SetPitch(s32 i, u32 step)
{
    if (step <= 0x190) {
        SndChannel *c = &data_0213a548.se[i];

        func_0200d564(-1, c->id, step, 4);
        func_0200d5a0(-1, c->id, 0, 0x10000, 0, 0, 0, 0, 0, data_02123442[step]);
    }
}

u32 SndSe_GetId(s32 i)
{
    return data_0213a548.se[i].id;
}
