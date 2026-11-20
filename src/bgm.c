/*
 * bgm.c - background music selection.
 * JP (ABHJ) 0x020B5B9C - 0x020B6758.
 *
 * Two music channels follow a requested track id: a change fades the old
 * track out before the new one starts, and the volume follows the room.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "game.h"
#include "scd_data.h"
#include "obj.h"

typedef struct BgmChannel {
    /* 0x00 */ u8 state;        /* 0: playing, 1: fading out, 2: start the request */
    /* 0x01 */ u8 ch;
    /* 0x02 */ u8 volume;
    /* 0x03 */ u8 unk03;        /* last data_0213a71c.unk02 seen */
    /* 0x04 */ u16 cur;         /* track playing */
    /* 0x06 */ u16 req;         /* track requested */
} BgmChannel;

typedef struct BgmSlot {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 unk01[2];
    /* 0x03 */ u8 track;
} BgmSlot;

typedef struct BgmState {
    /* 0x00 */ BgmChannel chan[2];
    /* 0x10 */ BgmSlot slots[3];
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u16 slotMask;    /* bits 3-5: active slots; saved in the high byte */
    /* 0x20 */ u8 mode;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u8 unk24[2];
    /* 0x26 */ u8 unk26;
    /* 0x27 */ u8 unk27;
    /* 0x28 */ u16 unk28;
} BgmState;

typedef struct UnkStruct_0213b414 {
    /* 0x00 */ u8 unk00[0x7A];
    /* 0x7A */ u16 unk7A;
} UnkStruct_0213b414;

typedef struct UnkStruct_0214c640 {
    /* 0x000 */ u8 unk000[0x3F4];
    /* 0x3F4 */ u8 unk3F4;
} UnkStruct_0214c640;

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u8 unk04;
} UnkStruct_0213a6cc;

typedef struct UnkStruct_0213b49c {
    /* 0x000 */ u8 unk000[0x1FC];
    /* 0x1FC */ u32 unk1FC_0 : 1;
                u32 notify : 1;
                u32 variant : 2;   /* replaces the stage when data_0213a6cc.unk04 is set */
} UnkStruct_0213b49c;

/* Stage and room of data_0213a71c as one value: room << 8 | stage. */
#define CUR_STAGE_ROOM (*(u16 *)&data_0213a71c)

/* Not yet identified. JP addresses. */
extern void func_020c3ecc(void);
extern void func_020c4194(u32 kind, u32 value);
extern void func_020c4278(void);
extern BOOL func_02088388(void);
extern void SndCh_FadeTo(u8 ch, u32 volume, u32 frames);
extern void SndCh_ApplyEffect(u8 ch, void *data);
extern void SndCh_SetVolume(u8 ch, u32 volume);
extern void SndCh_FadeOut(u8 ch, u32 frames);
extern void SndCh_Stop(u8 ch);
extern void SndCh_Play(u8 ch, u16 track);
extern s32 SndCh_GetStatus(u8 ch);
extern void func_020b7ef8(u32 id);
extern s8 func_02073c4c(s16 x, s16 z);
extern void func_02105ca8(const void *src, void *dst, u32 size);  /* probably MI_CpuCopy8 */

extern BgmState data_0213a900;              /* JP 0x0213A900 */
extern UnkStruct_0213b414 data_0213b414;    /* JP 0x0213B414 */
extern u32 data_0213b7a4[];                 /* JP 0x0213B7A4: flags */
extern UnkStruct_0214c640 data_0214c640;    /* JP 0x0214C640 */
extern u8 data_0214ca5c[];                  /* JP 0x0214CA5C */
extern u8 data_02122a9c[][0x10];            /* JP 0x02122A9C */
extern UnkStruct_0213a6cc data_0213a6cc;    /* JP 0x0213A6CC */
extern UnkStruct_0213b49c data_0213b49c;    /* JP 0x0213B49C */
extern u32 data_0213b764[];                 /* JP 0x0213B764: flags */
extern u32 data_0213b80c[];                 /* JP 0x0213B80C: flags */
extern u8 data_0213b8ce[][0x20];            /* JP 0x0213B8CE: slot setup by stage, room */
extern u16 data_02122a7c[];                 /* JP 0x02122A7C: tracks played on channel 0, 0-terminated */
extern u8 data_02122aec[][3];               /* JP 0x02122AEC: slot tracks */
extern const u8 data_02122b9c[];            /* JP 0x02122B9C: initial data_0213b8ce */
extern u8 data_02122c7c[][0x20][4];         /* JP 0x02122C7C: slot track sets by stage, room */
extern u16 *data_02123008[];                /* JP 0x02123008: by variant, then room */
extern u16 *data_02123014[];                /* JP 0x02123014: by variant, then room */
extern u16 *data_02123048[];                /* JP 0x02123048: by stage, then room */
extern u16 *data_02123064[];                /* JP 0x02123064: by stage, then room */
extern Obj *data_027e000c;                  /* DTCM: current object */

void Bgm_ClearSlots(void);
void Bgm_ClearSlot(u32 i);
void Bgm_SetSlots(u32 bits);
void Bgm_SetSlot(u32 i);
void Bgm_UpdateChannel(BgmChannel *c, u8 ch, u16 req);
void Bgm_SetSlotTracks(u32 n);
void Bgm_ApplySlot(u32 bits, u32 i);

/* Play the room's second sound from the stage or variant table. */
void Bgm_PlayRoomSound2(void)
{
    u16 id;

    if (data_0213a6cc.unk04 == 0) {
        id = data_02123064[data_0213a71c.stage][data_0213a71c.room];
    } else {
        id = data_02123014[data_0213b49c.variant][data_0213a71c.room];
    }
    func_020b7ef8(id);
}

/* Play the room's sound from the stage or variant table. */
void Bgm_PlayRoomSound(void)
{
    u16 id;

    if (data_0213a6cc.unk04 == 0) {
        id = data_02123048[data_0213a71c.stage][data_0213a71c.room];
    } else {
        id = data_02123008[data_0213b49c.variant][data_0213a71c.room];
    }
    func_020b7ef8(id);
}

/* Play the stage's sound. */
void Bgm_PlayStageSound(void)
{
    u32 id;

    switch (data_0213a71c.stage) {
    case 5:
        if (data_0213a71c.room == 0x10) {
            id = 0x39;
            break;
        }
    case 0:
        id = 0xC;
        break;
    case 1:
    case 6:
        if (data_0213a71c.room == 0x1B || data_0213a71c.room == 6) {
            id = 0x39;
        } else {
            id = 0xC;
        }
        break;
    case 4:
    case 0x11:
        id = 0x8D;
        break;
    case 2:
    case 3:
    case 7:
    case 8:
    case 9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
    case 0x10:
    default:
        id = 1;
        break;
    }
    func_020b7ef8(id);
}

/* Room 0x15 of stages 0 and 5: play sound 2 unless the flags say otherwise. */
void Bgm_CheckRoom15(void)
{
    if (data_0213a71c.room != 0x15) {
        return;
    }
    if (data_0213a71c.stage != 0 && data_0213a71c.stage != 5) {
        return;
    }
    if (Flag_Test(data_0213b80c, 0)) {
        return;
    }
    if (Flag_Test(data_0213b80c, 1)) {
        return;
    }
    if (data_02143640.unkF4D == 0) {
        if (Flag_Test(data_0213b764, 0x20)) {
            return;
        }
    } else if (Flag_Test(data_0213b764, 0x29)) {
        if (Flag_Test(data_0213b764, 7)) {
            return;
        }
        if (Flag_Test(data_0213b764, 0x5F)) {
            return;
        }
    }
    func_020b7ef8(2);
}

/* Play sound base, adjusted for where the current object stands. */
void Bgm_PlayAtObject(u8 base)
{
    s32 adjust;

    if (data_0213b750.flags2 & 1) {
        adjust = -3;
    } else {
        adjust = 0;
    }
    func_020b7ef8((u8)(base + (adjust + func_02073c4c(data_027e000c->posX, data_027e000c->posZ))));
}

/* Play a jingle: listed tracks on channel 0, others on channel 1. */
void Bgm_PlayJingle(u16 track)
{
    u16 *p = data_02122a7c;
    u8 ch = 1;
    u16 t;

    while (1) {
        t = *p++;
        if (t == 0) {
            break;
        }
        if (t == track) {
            ch = 0;
            break;
        }
    }
    SndCh_Play(ch, track);
    data_0213b750.flags |= 0x20000;
}

void Bgm_StopJingle(void)
{
    SndCh_Stop(0);
    SndCh_Stop(1);
    data_0213b750.flags &= ~0x20000;
}

/* Clear the jingle flag once both channels are idle. */
void Bgm_UpdateJingle(void)
{
    if (data_0213b750.flags & 0x20000) {
        s32 a = SndCh_GetStatus(1);
        s32 b = SndCh_GetStatus(0);

        if (a == 0 && b == 0) {
            data_0213b750.flags &= ~0x20000;
        }
    }
}

void Bgm_ResetRoomTable(void)
{
    func_02105ca8(data_02122b9c, data_0213b8ce, 0xE0);
}

/* Load the three slot tracks from set n (0xFF: set 0x39). */
void Bgm_SetSlotTracks(u32 n)
{
    u8 *src;
    u32 i;

    if (n == 0xFF) {
        n = 0x39;
    }
    src = data_02122aec[n];
    for (i = 0; i < 3; i++) {
        data_0213a900.slots[i].track = src[i];
    }
}

/* Set up the music slots for the current room. */
void Bgm_InitRoom(void)
{
    u8 set;
    u8 room;
    u8 cfg;

    if (data_0213a6cc.unk04 == 0) {
        set = data_0213a71c.stage;
    } else {
        set = data_0213b49c.variant;
        if (set != 0) {
            set += 2;
        }
    }
    room = data_0213a71c.room;
    data_0213a900.unk1D = data_0213b8ce[set][room];
    cfg = data_0213a900.unk1D;
    if (cfg == 0xFF) {
        Bgm_ClearSlots();
        data_0213a900.slotMask = 0xFF;
    } else {
        Bgm_SetSlotTracks(data_02122c7c[set][room][cfg & 3]);
        switch (cfg >> 6) {
        case 0:
            Bgm_ApplySlot(cfg, 0);
            Bgm_ApplySlot(cfg, 1);
            Bgm_ApplySlot(cfg, 2);
            break;
        case 1:
            Bgm_ClearSlots();
            break;
        }
        data_0213a900.slotMask = cfg;
    }
}

/* Turn slot i on or off from bit i + 3 of bits. */
void Bgm_ApplySlot(u32 bits, u32 i)
{
    if (bits & (1 << (i + 3))) {
        Bgm_SetSlot(i);
    } else {
        Bgm_ClearSlot(i);
    }
}


/* Push the slot mask into the high byte and clear every slot. */
void Bgm_SaveSlots(void)
{
    data_0213a900.slotMask <<= 8;
    Bgm_ClearSlots();
}

/* Restore the slot mask saved by Bgm_SaveSlots. */
void Bgm_RestoreSlots(void)
{
    data_0213a900.slotMask >>= 8;
    Bgm_SetSlots(data_0213a900.slotMask);
}

/* Activate the slots whose bits (3-5) are set. */
void Bgm_SetSlots(u32 bits)
{
    if (data_0213a900.unk1D != 0xFF) {
        if (bits & 8) {
            Bgm_SetSlot(0);
        }
        if (bits & 0x10) {
            Bgm_SetSlot(1);
        }
        if (bits & 0x20) {
            Bgm_SetSlot(2);
        }
    }
}

void Bgm_SetSlot(u32 i)
{
    if (!(data_0213b750.flags2 & 0x10000000)) {
        data_0213a900.slots[i].active = 1;
        data_0213a900.slotMask |= 1 << (i + 3);
    }
}

void Bgm_ClearSlot(u32 i)
{
    data_0213a900.slots[i].active = 0;
    data_0213a900.slotMask &= ~(1 << (i + 3));
}

void Bgm_ClearSlots(void)
{
    Bgm_ClearSlot(0);
    Bgm_ClearSlot(1);
    Bgm_ClearSlot(2);
}

/* Pick the tracks for both channels. */
void Bgm_Update(void)
{
    u16 main = 0;
    u16 sub = 0;
    u32 i;
    s32 kind;
    u32 track;

    if (data_0213a900.mode == 2) {
        if (data_0213a900.unk22 != data_0213a900.unk23) {
            data_0213a900.unk22 = data_0213a900.unk23;
        }
        main = data_0213a900.unk22;
    } else if (data_0213a900.mode == 1 || data_0213a900.mode == 3) {
        main = data_0213a900.unk28;
        if (data_0213a900.unk26 != 0) {
            main = data_0213a900.unk26;
        } else if (data_0213a900.unk27 != 0) {
            main = data_0213a900.unk27;
        }
        for (i = 0; i < 3; i++) {
            if (data_0213a900.slots[i].active) {
                track = data_0213a900.slots[i].track;
                if (track < 0x4C) {
                    kind = 2;
                } else {
                    kind = 3;
                }
                if (kind != 2) {
                    sub = track;
                }
            }
        }
    } else if (func_02088388()) {
        if (CUR_STAGE_ROOM == 0xC03) {
            main = 0x46;
        } else {
            main = 0x4B;
        }
    } else {
        for (i = 0; i < 3; i++) {
            if (data_0213a900.slots[i].active) {
                track = data_0213a900.slots[i].track;
                if (track < 0x4C) {
                    kind = 2;
                } else {
                    kind = 3;
                }
                if (kind == 2) {
                    main = track;
                } else {
                    sub = track;
                }
            }
        }
        if (data_0213b750.flags2 & 0x20000) {
            if (Flag_Test(data_0213b7a4, 0xB)) {
                main = 0x35;
            }
            if (main == 0x2A && CUR_STAGE_ROOM != 0xD01) {
                main = 0x48;
            }
        }
    }
    Bgm_UpdateChannel(&data_0213a900.chan[0], 2, main);
    Bgm_UpdateChannel(&data_0213a900.chan[1], 3, sub);
}

/* Move channel c towards track req and keep its volume in step with the room. */
void Bgm_UpdateChannel(BgmChannel *c, u8 ch, u16 req)
{
    s32 volume;
    s32 n;

    c->ch = ch;
    c->req = req;
    switch (c->state) {
    case 0:
    default:
        if (c->cur != c->req) {
            switch (c->req) {
            case 0xD4:
                c->cur = c->req;
                SndCh_FadeOut(ch, 0xB4);
                break;
            case 9:
                c->state = 2;
                SndCh_Stop(ch);
                break;
            default:
                c->cur = 0;
                c->state = 1;
                SndCh_FadeOut(ch, 0x18);
                break;
            }
        }
        break;
    case 1:
        if (SndCh_GetStatus(ch) != 4) {
            c->state = 2;
            SndCh_Stop(ch);
        }
        break;
    case 2:
        c->state = 0;
        c->cur = c->req;
        if (c->req == 0) {
            c->volume = 0;
        } else {
            c->volume = 0x7F;
            c->unk03 = 0x7F;
            SndCh_Play(ch, c->req & 0x7F);
        }
        break;
    }

    if (c->state == 1) {
        return;
    }
    switch (c->cur) {
    case 5:
    case 0x28:
        if (SndCh_GetStatus(ch) == 0) {
            c->cur = 0;
        }
        return;
    case 0xF:
    case 0x1D:
        if (CUR_STAGE_ROOM == 0xF00) {
            volume = 0x7F;
        } else if (data_0213b214.unk16 == 0x1F) {
            switch (data_0213b414.unk7A) {
            case 0xF00:
                volume = 0x7F;
                break;
            case 0x500:
                volume = 0x15;
                break;
            case 0x300:
                volume = 0x1F;
                break;
            default:
                volume = 0x3F;
                break;
            }
        } else {
            switch (data_0213a71c.unk02) {
            case 3:
                volume = 0x3F;
                break;
            case 2:
                volume = 0x1F;
                break;
            default:
                volume = 0x15;
                break;
            }
        }
        if (c->volume != volume) {
            c->volume = volume;
            SndCh_SetVolume(ch, (u8)volume);
        }
        return;
    case 0x51:
        if (data_0214c640.unk3F4 & 1) {
            SndCh_ApplyEffect(ch, data_0214ca5c);
        } else if (c->volume == 0x7F) {
            c->volume = 0x7E;
            SndCh_SetVolume(ch, 0);
            SndCh_FadeTo(ch, 0x7E, 0x96);
        }
        return;
    }

    if (c->unk03 != data_0213a71c.unk02) {
        c->unk03 = data_0213a71c.unk02;
        switch (c->cur) {
        default:
            n = 0;
            break;
        case 0x4D:
            n = 1;
            break;
        case 0x3D:
            n = 2;
            break;
        case 0x59:
            n = 3;
            break;
        case 0x54:
            n = 4;
            break;
        }
        if (n != 0) {
            SndCh_ApplyEffect(ch, data_02122a9c[n]);
        }
    }
}

/* Start the sound system on first use, then run it. */
void Sound_Update(void)
{
    if (gGame.unkB4 == 0) {
        gGame.unkB4 = 1;
        func_020c4278();
        func_020c4194(0x21, 0);
    }
    func_020c3ecc();
}
