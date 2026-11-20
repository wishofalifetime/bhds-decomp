/*
 * sndbank.c - sound bank lists for the current room.
 * JP (ABHJ) 0x020B6758 - 0x020B6E48.
 *
 * The room's banks are collected into two zero-terminated id lists in the
 * music state and then loaded.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "game.h"
#include "scd.h"
#include "scd_data.h"

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u8 unk04;
} UnkStruct_0213a6cc;

typedef struct UnkStruct_0213b49c {
    /* 0x000 */ u8 unk000[0x1FC];
    /* 0x1FC */ u32 unk1FC_0 : 1;
                u32 notify : 1;
                u32 variant : 2;
} UnkStruct_0213b49c;

/* Part of the music state at JP 0x0213A900 (see bgm.c). */
typedef struct SndBankState {
    /* 0x00 */ u8 unk00[0x42];
    /* 0x42 */ u8 banks[0x10];     /* 0-terminated */
    /* 0x52 */ u8 unk52[0x10];
    /* 0x62 */ u8 banks2[0xC];     /* 0-terminated */
} SndBankState;

/* Not yet identified. JP addresses. */
extern BOOL func_02088388(void);
extern u8 func_020883b4(void);
extern u16 func_0201896c(u32 a, u32 b);
extern void func_020b8028(u32 slot, u32 id);
extern void Snd_PlayAt(u8 a, s32 b, s32 *pos, s8 c);
extern void func_02105c14(void *dst, u32 value, u32 size);  /* probably MI_CpuFill8 */

extern SndBankState data_0213a900;          /* JP 0x0213A900 */
extern UnkStruct_0213a6cc data_0213a6cc;    /* JP 0x0213A6CC */
extern UnkStruct_0213b49c data_0213b49c;    /* JP 0x0213B49C */
extern s8 data_02122ffc[];                  /* JP 0x02122FFC */
extern u32 data_02123020[];                 /* JP 0x02123020: alternate rooms by variant */
extern u32 data_0212302c[];                 /* JP 0x0212302C: alternate rooms by stage */
extern u8 data_02123080[];                  /* JP 0x02123080 */
extern u8 data_021230a0[];                  /* JP 0x021230A0 */
extern u8 data_02123304[][0x20];            /* JP 0x02123304: by stage (or 7 + variant), room */
extern s32 data_027e0054[3];                /* DTCM: position scratch */
extern u16 data_021230e4[];                 /* JP 0x021230E4: by unkF4D, alternate room */
extern u8 **data_021232bc[];                /* JP 0x021232BC: 0-terminated lists by stage, room */
extern u8 *data_02123dcc[];                 /* JP 0x02123DCC: 0-terminated lists by room */

u8 SndBank_IsAltRoom(void);
void SndBank_Add(u8 id);
void SndBank_LoadExtra(void);
void SndBank_LoadAll(void);

void SndBank_Clear(void)
{
    func_02105c14(data_0213a900.banks, 0, 0x10);
    func_02105c14(data_0213a900.banks2, 0, 0xC);
}

/* Collect and load every bank the current room needs. */
void SndBank_LoadRoom(void)
{
    u8 *p;
    u32 i;

    SndBank_LoadExtra();
    SndBank_Add(data_021230e4[SndBank_IsAltRoom() + data_02143640.unkF4D * 2]);
    SndBank_Add((u16)(func_02088388() ? 0xB7 : 0xA));
    if (data_0213a6cc.unk04 != 0) {
        SndBank_Add(0x21);
    }
    if (func_02088388()) {
        SndBank_Add(1);
    } else {
        p = data_021232bc[data_0213a71c.stage][data_0213a71c.room];
        while (*p != 0) {
            SndBank_Add(*p++);
        }
    }
    if (data_0213a6cc.unk04 != 0) {
        switch (data_0213b49c.variant) {
        case 0:
            switch (data_0213a71c.room) {
            case 6:
            case 0xB:
            case 0x11:
                SndBank_Add(0x77);
                break;
            case 0x19:
                p = data_02123dcc[data_0213a71c.room];
                while (*p != 0) {
                    SndBank_Add(*p++);
                }
                break;
            }
            break;
        case 1:
            switch (data_0213a71c.room) {
            case 0:
                SndBank_Add(0x6E);
                SndBank_Add(0x8E);
                break;
            case 9:
                SndBank_Add(0x77);
                break;
            }
            break;
        case 2:
            switch (data_0213a71c.room) {
            case 7:
            case 9:
            case 0xB:
                SndBank_Add(0x77);
                break;
            case 8:
                SndBank_Add(0x6D);
                break;
            }
            break;
        }
    }
    for (i = 0; i < 0xC; i++) {
        u32 id = data_0213a900.banks2[i];

        if (id == 0) {
            break;
        }
        SndBank_Add(id);
    }
    SndBank_LoadAll();
}
u32 SndBank_GetA(u32 n);
u32 SndBank_GetB(u32 n);
void SndBank_GetSpecial(u32 n, u32 b, u16 *out);
BOOL SndBank_IsSpecial(u32 n);
void SndBank_Add2(u8 id);

/* Load the collected banks; banks 0x0B-0x12 share slot 0. */
void SndBank_LoadAll(void)
{
    s32 slot;
    u32 i;

    slot = 2;
    for (i = 0; i < 0x10; i++) {
        u8 id = data_0213a900.banks[i];

        if (id == 0) {
            break;
        }
        if (id >= 0xB && id <= 0x12) {
            func_020b8028(0, id);
        } else {
            func_020b8028(slot, id);
            slot++;
        }
        if (slot >= 0x10) {
            break;
        }
    }
    if (gScd.unk8C8 == 1) {
        data_027e0054[0] = data_02143640.unkF74[0];
        data_027e0054[1] = data_02143640.unkF74[1];
        data_027e0054[2] = data_02143640.unkF74[2];
        Snd_PlayAt(6, 2, data_027e0054, 0);
    }
}

/* Add id to the first list unless it is already there. */
void SndBank_Add(u8 id)
{
    u32 i;

    for (i = 0; i < 0x10; i++) {
        if (id == data_0213a900.banks[i]) {
            return;
        }
        if (data_0213a900.banks[i] == 0) {
            data_0213a900.banks[i] = id;
            return;
        }
    }
}

/* Add id to the second list unless it is already there. */
void SndBank_Add2(u8 id)
{
    u32 i;

    for (i = 0; i < 0xC; i++) {
        if (id == data_0213a900.banks2[i]) {
            return;
        }
        if (data_0213a900.banks2[i] == 0) {
            data_0213a900.banks2[i] = id;
            return;
        }
    }
}

void SndBank_LoadExtra(void)
{
    func_020b8028(0, (u16)data_02122ffc[*data_02147640.unk53C]);
}

/* Collect the banks for room kind n. */
void SndBank_AddRoom(u32 n, u32 b)
{
    u16 special[2];

    if (SndBank_IsSpecial(n)) {
        SndBank_GetSpecial(n, b, special);
        SndBank_Add2(special[0]);
        SndBank_Add2(special[1]);
        SndBank_Add2(0xC1);
    } else {
        if (data_0213a6cc.unk04 != 0 && n == 0x10) {
            SndBank_Add2(SndBank_GetB(0xC));
        }
        SndBank_Add2(SndBank_GetB(n));
        if (n == 9) {
            SndBank_Add2(0xC4);
        }
    }
    if (func_02088388()) {
        SndBank_Add2(SndBank_GetA(n));
    }
}

/* Room kinds 0, 1, 17 and 22. */
BOOL SndBank_IsSpecial(u32 n)
{
    return n == 0 || n == 1 || n == 0x11 || n == 0x16;
}

void SndBank_GetSpecial(u32 n, u32 b, u16 *out)
{
    u16 id;
    u8 set;

    if (func_02088388()) {
        id = 0x4A;
    } else if ((data_0213b750.flags2 & 0x20000) || data_0213a6cc.unk04 != 0) {
        id = func_0201896c(n, b);
    } else {
        if (data_0213a6cc.unk04 == 0) {
            set = data_0213a71c.stage;
        } else {
            set = data_0213b49c.variant + 7;
        }
        id = data_02123304[set][data_0213a71c.room];
    }
    out[1] = id;
    out[0] = SndBank_IsAltRoom() ? 0x49 : 0x48;
}

u32 SndBank_GetB(u32 n)
{
    if (n < 0x20) {
        switch (n) {
        case 6:
            return SndBank_IsAltRoom() ? 0x5B : 0x5A;
        case 0xA:
            return SndBank_IsAltRoom() ? 0x5F : 0x5E;
        default:
            return data_02123080[n];
        }
    }
    return 0;
}

u32 SndBank_GetA(u32 n)
{
    if (n < 0x20) {
        if (n == 6 && SndBank_IsAltRoom()) {
            return 0xC0;
        }
        return data_021230a0[n];
    }
    return 0;
}

/* Whether the current room uses its alternate sounds. */
u8 SndBank_IsAltRoom(void)
{
    if (func_02088388()) {
        return func_020883b4();
    }
    if (data_0213a6cc.unk04 == 0) {
        return (data_0212302c[data_0213a71c.stage] >> data_0213a71c.room) & 1;
    }
    return (data_02123020[data_0213b49c.variant] >> data_0213a71c.room) & 1;
}
