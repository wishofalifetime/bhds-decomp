/*
 * choice_scene.c - the two-sided screen as a scene: setup, input, the
 * swap animation and the scene dispatchers.
 * JP (ABHJ) 0x020BCA10 - 0x020BD030.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "choice.h"
#include "game.h"
#include "hw_regs.h"
#include "pad.h"

typedef struct UnkStruct_0213a6cc {
    /* 0x00 */ u8 unk00[2];
    /* 0x02 */ u8 unk02;
} UnkStruct_0213a6cc;

typedef struct UnkStruct_0213a900 {
    /* 0x00 */ u8 unk00[0x20];
    /* 0x20 */ u8 unk20;
} UnkStruct_0213a900;

/* Not yet identified. JP addresses. */
extern void func_020038b8(void *a);
extern void func_02005fcc(void *a);
extern void func_0200f6e8(u32 a);
extern u32 func_02013340(u32 a);
extern void func_020135b8(u32 a, u32 b, void *c);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern void func_02090690(u32 a);
extern void func_020bf378(u32 a);
extern void func_020c36cc(void *a, VoidFunc draw, u32 b, u32 c);
extern void func_020c4258(u32 a);
extern s32 func_020fe688(s32 a, s32 b);   /* fixed-point divide */
extern void func_02100224(void *reg, u32 a, u32 b, u32 c, u32 d);
extern s32 func_01ff8000(u16 angle);     /* sine */
extern void Bgm_PlayJingle(u16 track);
extern BOOL SndBank_Load(s32 slot, u32 id);
extern void Snd_Play0(u32 id);

/* The US and EU versions also accept a second button to confirm. */
#ifdef VERSION_JP
#define CHOICE_CONFIRM_KEYS 0x10
#else
#define CHOICE_CONFIRM_KEYS 0x4010
#endif

extern VoidFunc data_02124370[];          /* JP 0x02124370: by data_0213b214.unk21 */
extern VoidFunc data_0212437c[];          /* JP 0x0212437C: by data_0213b214.state */
extern u8 data_02124388[];                /* JP 0x02124388 */
extern ChoicePos data_02124394[];         /* JP 0x02124394 */
extern u8 data_021243a4[];                /* JP 0x021243A4 */
extern Pad data_0213a6c0;                 /* JP 0x0213A6C0: repeat-filtered pad */
extern UnkStruct_0213a6cc data_0213a6cc;  /* JP 0x0213A6CC */
extern u16 data_0213a708;                 /* JP 0x0213A708: nonzero while fading */
extern UnkStruct_0213a900 data_0213a900;  /* JP 0x0213A900 */
extern u8 data_0213b230[];                /* JP 0x0213B230 */

void Choice_Main(void)
{
    data_0212437c[data_0213b214.state]();
}

void Choice_Init(void)
{
    func_020c36cc(data_0213b230, Choice_Draw, 0, 0x48);
    gGame.unk01 = 0;
    func_0200f6e8(4);
    func_020038b8(data_021243a4);
    func_02005fcc(data_02124388);
    func_020135b8(0, 0, data_02124394);
    SndBank_Load(4, 2);
    CHOICE_WORK->side[1].count = 0;
    CHOICE_WORK->side[0].count = 1;
    CHOICE_WORK->side[1].x = -0x8000;
    CHOICE_WORK->side[1].y = -0x8000;
    CHOICE_WORK->side[0].x = 0x8000;
    CHOICE_WORK->side[0].y = 0x8000;
    CHOICE_WORK->side[0].kind = 1;
    CHOICE_WORK->side[1].kind = 2;
    Choice_Start();
    func_02100224((void *)0x04000050, 4, 0x18, 0x10, 8);
    REG_DISPCNT = (REG_DISPCNT & ~0x1F00) | 0x1E00;
    func_020c4258(1);
    func_0206fb04(3, 2, 0, 0x80);
}

void Choice_Update(void)
{
    data_02124370[data_0213b214.unk21]();
}

void Choice_Input(void)
{
    u32 key;
    u32 track;

    if (data_0213a708 != 0) {
        return;
    }
    if (data_0213a6c0.trg & 0x102) {
        key = 0;
    } else if (data_0213a6c0.trg & 0x201) {
        key = 1;
    } else if (data_0213a6c0.trg & CHOICE_CONFIRM_KEYS) {
        key = 2;
    } else if (data_0213a6c0.trg & 0x20) {
        key = 3;
    } else {
        key = func_02013340(0);
    }
    switch (key) {
    case 0:
        CHOICE_WORK->dir = -1;
        CHOICE_WORK->blink[0] = 1;
        data_0213b214.unk21 = 1;
        break;
    case 1:
        CHOICE_WORK->dir = 1;
        CHOICE_WORK->blink[1] = 1;
        data_0213b214.unk21 = 1;
        break;
    case 2:
        data_0213b214.unk1D = 1;
        data_0213b214.unk21 = 2;
        data_0213a6cc.unk02 = CHOICE_WORK->unk44;
        if (data_0213a900.unk20 == 2) {
            if (data_0213a6cc.unk02 == 0) {
                track = 0x28D;
            } else {
                track = 0x28E;
            }
            Bgm_PlayJingle(track);
            func_02090690(0x1E);
            func_0206fb04(4, 1, 1, 0x80);
        } else {
            Snd_Play0(0xB);
            func_0206fb04(3, 0, 1, 0xC);
        }
        break;
    case 3:
        data_0213b214.unk1D = 0;
        data_0213b214.unk21 = 2;
        Snd_Play0(0xE);
        func_0206fb04(3, 1, 1, 0x80);
        break;
    }
}

/* Swap the two sides along an arc over 0x20 frames. */
void Choice_Swap(void)
{
    s32 d;

    switch (data_0213b214.unk22) {
    case 0:
        data_0213b214.unk22 = 1;
        data_0213b214.blink = 0xF;
        func_020bf378(0);
        Snd_Play0(10);
        CHOICE_WORK->timer = 0x20;
        CHOICE_WORK->side[1].ox = CHOICE_WORK->side[1].x;
        CHOICE_WORK->side[1].oy = CHOICE_WORK->side[1].y;
        CHOICE_WORK->side[0].ox = CHOICE_WORK->side[0].x;
        CHOICE_WORK->side[0].oy = CHOICE_WORK->side[0].y;
        CHOICE_WORK->side[1].vx = func_020fe688(CHOICE_WORK->side[0].ox - CHOICE_WORK->side[1].ox, 0x20000);
        CHOICE_WORK->side[1].vy = func_020fe688(CHOICE_WORK->side[0].oy - CHOICE_WORK->side[1].oy, 0x20000);
        CHOICE_WORK->side[0].vx = func_020fe688(CHOICE_WORK->side[1].ox - CHOICE_WORK->side[0].ox, 0x20000);
        CHOICE_WORK->side[0].vy = func_020fe688(CHOICE_WORK->side[1].oy - CHOICE_WORK->side[0].oy, 0x20000);
    case 1:
        CHOICE_WORK->timer--;
        if (CHOICE_WORK->timer != 0) {
            CHOICE_WORK->side[1].x = CHOICE_WORK->side[1].ox;
            CHOICE_WORK->side[1].y = CHOICE_WORK->side[1].oy;
            CHOICE_WORK->side[0].x = CHOICE_WORK->side[0].ox;
            CHOICE_WORK->side[0].y = CHOICE_WORK->side[0].oy;
            Choice_Move();
            CHOICE_WORK->side[1].ox = CHOICE_WORK->side[1].x;
            CHOICE_WORK->side[1].oy = CHOICE_WORK->side[1].y;
            CHOICE_WORK->side[0].ox = CHOICE_WORK->side[0].x;
            CHOICE_WORK->side[0].oy = CHOICE_WORK->side[0].y;
            d = func_01ff8000(CHOICE_WORK->timer << 10);
            d *= CHOICE_WORK->dir;
            CHOICE_WORK->side[CHOICE_WORK->cur ^ 1].x += d * 0x78;
            CHOICE_WORK->side[CHOICE_WORK->cur ^ 1].y += d * 0x50;
            CHOICE_WORK->side[CHOICE_WORK->cur].x -= d * 0x78;
            CHOICE_WORK->side[CHOICE_WORK->cur].y -= d * 0x50;
            if (CHOICE_WORK->timer == 0x10) {
                CHOICE_WORK->side[0].count = CHOICE_WORK->cur;
                CHOICE_WORK->side[1].count = CHOICE_WORK->cur ^ 1;
                data_0213b214.unk1E = 0;
            }
            if (CHOICE_WORK->timer == 0x18) {
                data_0213b214.unk1E = 1;
            }
        } else {
            CHOICE_WORK->blink[0] = 0;
            CHOICE_WORK->blink[1] = 0;
            CHOICE_WORK->cur ^= 1;
            Choice_SetPos(0, CHOICE_WORK->side[1].count);
            Choice_SetPos(1, CHOICE_WORK->side[0].count);
            Choice_Start();
            func_020c4258(1);
        }
        break;
    }
}
