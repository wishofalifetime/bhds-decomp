/*
 * sndch.c - the four sound channels.
 * JP (ABHJ) 0x020B7178 - 0x020B7680.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "snd.h"

/* Sound header returned by func_0200da8c. */
typedef struct SndHeader {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u8 volume;
} SndHeader;

/* Part of the music state at JP 0x0213A900 (see bgm.c). */
typedef struct SndEffectOut {
    /* 0x00 */ u8 unk00[0x3E];
    /* 0x3E */ s16 pan;         /* set by func_01ffc2b4 */
    /* 0x40 */ s16 volume;
} SndEffectOut;

/* Not yet identified. JP addresses. */
extern void func_020b82cc(void *data);
extern void *func_020b82e4(u32 size);
extern void func_01ffc2b4(void *data);     /* ITCM */
extern s32 func_0200d3f8(void);
extern s32 func_0200d404(u16 ch, u32 a);
extern void func_0200d410(u16 id, u32 i, u8 pan);
extern void func_0200d448(u16 id, u8 volume);
extern void func_0200d470(u32 a);
extern void func_0200d47c(u16 id);
extern void func_0200d4a0(u16 id, u32 frames);
extern void func_0200d4ac(u16 id, u32 a, u32 b, u32 c, u32 d);
extern void func_0200d4e8(u16 ch);
extern void func_0200d4f4(u16 ch, void *buf, s32 size, u32 a, u32 b);
extern void func_0200d514(u16 id, u32 volume, u16 frames);
extern s32 func_0200d54c(u32 x);
extern s32 func_0200d558(u32 x);
extern SndHeader *func_0200da8c(u32 id);

extern SndEffectOut data_0213a900;          /* JP 0x0213A900 */

/* Map the driver's state for x to this game's channel status bits. */
u32 Snd_GetPlayState(u32 x)
{
    s32 a = func_0200d558(x);

    switch (func_0200d54c(x)) {
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
        case 5:
            return 0x40;
        case 4:
            return 0;
        default:
            return 1;
        }
    default:
        return 0;
    }
}

u8 SndCh_GetStatus(u32 ch)
{
    return data_0213a548.chan[ch].status;
}

/* Number of channels with a nonzero status. */
u8 SndCh_CountActive(void)
{
    u32 i;
    u32 n = 0;

    for (i = 0; i < 4; i++) {
        if (data_0213a548.chan[i].status != 0) {
            n++;
        }
    }
    return n;
}

/* Load sound id into channel ch's buffer and start it. */
void SndCh_Play(u32 ch, u16 id)
{
    SndChannel *c = &data_0213a548.chan[ch];
    s32 size;

    SndCh_Stop(ch);
    size = func_0200d404(ch, 4);
    if (size > 0) {
        c->buf = func_020b82e4(size);
        if (c->buf != NULL) {
            func_0200d4f4(ch, c->buf, size, 1, 1);
            func_0200d4ac(id, 0, 0, 4, 0);
            c->id = id;
            c->state = 1;
            c->status = 1;
        }
    }
}

void SndCh_Stop(u32 ch)
{
    SndChannel *c = &data_0213a548.chan[ch];

    if (c->id != 0) {
        func_0200d4a0(c->id, 0);
        func_0200d4e8(ch);
        if (c->buf != NULL) {
            func_020b82cc(c->buf);
            c->buf = NULL;
        }
        c->id = 0;
        c->state = 0;
        c->status = 0;
    }
}

void SndCh_StopAll(void)
{
    u32 i;

    func_0200d470(0);
    for (i = 0; i < 4; i++) {
        SndChannel *c = &data_0213a548.chan[i];

        func_0200d4e8(i);
        if (c->buf != NULL) {
            func_020b82cc(c->buf);
            c->buf = NULL;
        }
        c->id = 0;
        c->state = 0;
        c->status = 0;
    }
}

/* Fade channel ch out over frames. */
void SndCh_FadeOut(u32 ch, u32 frames)
{
    SndChannel *c = &data_0213a548.chan[ch];

    if (c->status != 0) {
        func_0200d4a0(c->id, frames);
        c->state = 4;
        c->status = 4;
    }
}

void SndCh_PauseAll(u32 a)
{
    u32 i;

    func_0200d470(a);
    for (i = 0; i < 4; i++) {
        data_0213a548.chan[i].state = 4;
        data_0213a548.chan[i].status = 4;
    }
}

void SndCh_SetVolume(u32 ch, u8 volume)
{
    func_0200d448(data_0213a548.chan[ch].id, volume);
}

void SndCh_SetPan(u32 ch, u8 pan)
{
    u8 i;

    for (i = 0; i < 2; i++) {
        func_0200d410(data_0213a548.chan[ch].id, i, pan);
    }
}

/* Apply a sound effect setting block to channel ch. */
void SndCh_ApplyEffect(u32 ch, void *data)
{
    func_01ffc2b4(data);
    SndCh_SetVolume(ch, data_0213a900.volume);
    SndCh_SetPan(ch, data_0213a900.pan);
}

/* Restart channel ch's sound from its header volume. */
void SndCh_Restart(u32 ch, u32 frames)
{
    SndChannel *c = &data_0213a548.chan[ch];
    SndHeader *h = func_0200da8c(c->id);

    if (c->status != 0) {
        func_0200d514(c->id, 0, frames);
        c->volume = h->volume;
        c->state = 0x10;
        c->status = 0x10;
    }
}

/* Resume a paused channel at its volume. */
void SndCh_Resume(u32 ch, u32 frames)
{
    SndChannel *c = &data_0213a548.chan[ch];

    if (c->status == 0x40) {
        func_0200d47c(c->id);
        func_0200d514(c->id, c->volume, frames);
        c->status = 0x80;
    }
}

/* Move channel ch's volume to volume over frames. */
void SndCh_FadeTo(u32 ch, u32 volume, u32 frames)
{
    SndChannel *c = &data_0213a548.chan[ch];

    func_0200d514(c->id, volume, frames);
}

s16 SndStrm_GetPos(void)
{
    BOOL idle = TRUE;
    s32 pos;

    if (data_0213a548.state != 1 && data_0213a548.state != 0) {
        idle = FALSE;
    }
    if (idle) {
        pos = 0;
    } else {
        pos = func_0200d3f8();
    }
    return pos;
}
