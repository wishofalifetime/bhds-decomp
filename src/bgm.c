/*
 * bgm.c - background music selection.
 * JP (ABHJ) 0x020B5B9C - 0x020B6204.
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

/* Stage and room of data_0213a71c as one value: room << 8 | stage. */
#define CUR_STAGE_ROOM (*(u16 *)&data_0213a71c)

/* Not yet identified. JP addresses. */
extern void func_020c3ecc(void);
extern void func_020c4194(u32 kind, u32 value);
extern void func_020c4278(void);
extern BOOL func_02088388(void);
extern void func_020b71c4(u8 ch, u32 volume, u32 frames);
extern void func_020b72d0(u8 ch, void *data);
extern void func_020b7360(u8 ch, u32 volume);
extern void func_020b73cc(u8 ch, u32 frames);
extern void func_020b7480(u8 ch);
extern void func_020b74f8(u8 ch, u16 track);
extern s32 func_020b75d0(u8 ch);

extern BgmState data_0213a900;              /* JP 0x0213A900 */
extern UnkStruct_0213b414 data_0213b414;    /* JP 0x0213B414 */
extern u32 data_0213b7a4[];                 /* JP 0x0213B7A4: flags */
extern UnkStruct_0214c640 data_0214c640;    /* JP 0x0214C640 */
extern u8 data_0214ca5c[];                  /* JP 0x0214CA5C */
extern u8 data_02122a9c[][0x10];            /* JP 0x02122A9C */

void Bgm_ClearSlots(void);
void Bgm_ClearSlot(u32 i);
void Bgm_SetSlots(u32 bits);
void Bgm_SetSlot(u32 i);
void Bgm_UpdateChannel(BgmChannel *c, u8 ch, u16 req);

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
                func_020b73cc(ch, 0xB4);
                break;
            case 9:
                c->state = 2;
                func_020b7480(ch);
                break;
            default:
                c->cur = 0;
                c->state = 1;
                func_020b73cc(ch, 0x18);
                break;
            }
        }
        break;
    case 1:
        if (func_020b75d0(ch) != 4) {
            c->state = 2;
            func_020b7480(ch);
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
            func_020b74f8(ch, c->req & 0x7F);
        }
        break;
    }

    if (c->state == 1) {
        return;
    }
    switch (c->cur) {
    case 5:
    case 0x28:
        if (func_020b75d0(ch) == 0) {
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
            func_020b7360(ch, (u8)volume);
        }
        return;
    case 0x51:
        if (data_0214c640.unk3F4 & 1) {
            func_020b72d0(ch, data_0214ca5c);
        } else if (c->volume == 0x7F) {
            c->volume = 0x7E;
            func_020b7360(ch, 0);
            func_020b71c4(ch, 0x7E, 0x96);
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
            func_020b72d0(ch, data_02122a9c[n]);
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
