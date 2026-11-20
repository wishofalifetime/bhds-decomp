/*
 * sndplay.c - effect channel update and the sound play entry points.
 * JP (ABHJ) 0x020B7BD0 - 0x020B7F70.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "snd.h"
#include "obj.h"

/* Part of the music state at JP 0x0213A900 (see bgm.c). */
typedef struct SndEffectOut {
    /* 0x00 */ u8 unk00[0x3E];
    /* 0x3E */ s16 pan;         /* set by func_01ffc2b4 */
    /* 0x40 */ s16 volume;
} SndEffectOut;

/* Not yet identified. JP addresses. */
extern void func_020b82cc(void *data);
extern void *func_020b82e4(u32 size);
extern void func_01ffc2b4(void *pos);  /* ITCM */
extern void func_0200d638(u32 a);
extern void func_0200d680(s32 a, u32 b, u32 c, u32 d);
extern void func_0200d6d0(u32 kind, u32 id, u32 a, u32 flags, u32 b, u32 c, s32 d, u32 e, s32 f, s32 g, u32 h);
extern void func_0200d8a4(u32 id);
extern void func_0200d924(u32 id, void *buf);
extern s32 func_0200d9e0(u32 id);
extern u32 SndSe_GetPlayState(u16 x);

extern SndEffectOut data_0213a900;  /* JP 0x0213A900 */
extern Obj *data_027e000c;          /* DTCM: current object */

void Snd_Play(u32 kind, u32 id);
void Snd_Play2(u32 id);
void Snd_PlayAt(u32 kind, u32 id, void *pos, s8 d);

void Snd_Play(u32 kind, u32 id)
{
    func_0200d6d0(kind, id, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void Snd_Play0(u32 id)
{
    Snd_Play(0, id);
}

void Snd_Play2(u32 id)
{
    Snd_Play(2, id);
}

void Snd_Play6(u32 id)
{
    Snd_Play(6, id);
}

void Snd_Play2Alt(u32 id)
{
    Snd_Play2(id);
}

/* Play with an explicit pan and angle; -1 leaves either unset. */
void Snd_PlayPanned(u32 kind, u32 id, s8 x, s8 angle, s16 c)
{
    u32 flags = 0;

    if (x > -1) {
        flags |= 4;
    }
    if (angle > -1) {
        flags |= 0x1000;
    }
    if (c != 0) {
        flags |= 0x10000;
    }
    func_0200d6d0(kind, id, 0, flags, 0, 0, x, 0, angle, c, 0);
}

/* Play positioned at pos (volume and pan from func_01ffc2b4). */
void Snd_PlayAt(u32 kind, u32 id, void *pos, s8 d)
{
    func_01ffc2b4(pos);
    func_0200d6d0(kind, id, 0, 0x11004, 0, 0, data_0213a900.volume, 0, (s8)data_0213a900.pan, d, 0);
}

void Snd_PlayAt0(u32 id, void *pos, s8 d)
{
    Snd_PlayAt(0, id, pos, d);
}

void Snd_PlayAt1(u32 id, void *pos, s8 d)
{
    Snd_PlayAt(1, id, pos, d);
}

void Snd_PlayAt6(u32 id, void *pos, s8 d)
{
    Snd_PlayAt(6, id, pos, d);
}

void Snd_PlayAtObject3(u32 id, s8 d)
{
    Snd_PlayAt(3, id, &data_027e000c->posX, d);
}

void Snd_PlayAt4(u32 id, void *pos, s8 d)
{
    Snd_PlayAt(4, id, pos, d);
}

void Snd_PlayAtObject5(u32 id, s8 d)
{
    Snd_PlayAt(5, id, &data_027e000c->posX, d);
}

void Snd_Stop(u32 a, u32 b)
{
    func_0200d680(-1, a, b, 0);
}

void Snd_StopAll(void)
{
    func_0200d638(0);
}

/* Free effect slots whose sound has finished. */
void SndSe_Update(void)
{
    u32 i;

    for (i = 0; i < 5; i++) {
        SndChannel *c = &data_0213a548.se[i];

        switch (c->status) {
        case 1:
            if (SndSe_GetPlayState(c->id) == 0) {
                c->status = 0;
                c->state = 0;
            }
            break;
        case 4:
            if (SndSe_GetPlayState(c->id) == 0) {
                if (c->buf != NULL) {
                    func_020b82cc(c->buf);
                    c->buf = NULL;
                    func_0200d8a4(c->id);
                }
                c->id = 0;
                c->status = 0;
                c->state = 0;
            }
            break;
        }
    }
}

/* Load effect id into slot i without playing it. */
BOOL SndSe_Preload(s32 i, u32 id)
{
    SndChannel *c = &data_0213a548.se[i];

    SndSe_Free(i);
    c->buf = func_020b82e4(func_0200d9e0(id));
    if (c->buf == NULL) {
        return FALSE;
    }
    c->id = id;
    func_0200d924(c->id, c->buf);
    return TRUE;
}
