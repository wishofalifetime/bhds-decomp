/*
 * exam_scene.c - the item examine screen as a scene: setup, controls,
 * zooming the model in, and checking whether the model has been turned
 * to reveal something.
 * JP (ABHJ) 0x020BD664 - 0x020BDC48.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "exam.h"
#include "game.h"
#include "hw_regs.h"
#include "pad.h"
#include "scd_data.h"

typedef struct UnkStruct_0213a71c_6 {
    /* 0x00 */ u8 unk00[6];
    /* 0x06 */ u8 unk06;
} UnkStruct_0213a71c_6;

/* Not yet identified. JP addresses. */
extern void func_020038b8(void *a);
extern void func_02005fcc(void *a);
extern void func_0200f6e8(u32 a);
extern s32 func_02013328(u32 a);
extern void func_020135b8(u32 a, u32 b, void *c);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern u16 func_02075028(u8 item);
extern void func_020c31c8(u8 item, u32 a);
extern void func_020c36cc(void *a, VoidFunc draw, VoidFunc free, u32 b);
extern void func_020c4188(u32 a, u32 b);
extern void func_020c4258(u32 a);
extern void func_020f4658(u32 a, u32 b, u32 c, u32 d, u32 e, u32 f);
extern void Snd_LoadBank0C(void);
extern void Snd_Play0(u32 id);

extern ItemStackInfo data_0211d738[];     /* JP 0x0211D738 */
extern u8 data_0211d690[];                /* JP 0x0211D690: examine message by index */
extern u8 data_021243d0[];                /* JP 0x021243D0 */
extern VoidFunc data_021243d8[];          /* JP 0x021243D8: by data_0213b214.unk21 */
extern ExamPos data_021243e4[];           /* JP 0x021243E4 */
extern u8 data_02124400[];                /* JP 0x02124400: tries << 4 | range set */
extern u8 data_02124410[];                /* JP 0x02124410 */
extern ExamPos data_02124450[];           /* JP 0x02124450 */
extern ExamRange data_0212446c[][2];      /* JP 0x0212446C: (x, y) ranges */
extern Pad data_0213a6c0;                 /* JP 0x0213A6C0 */
extern u8 data_0213a708;                  /* JP 0x0213A708: nonzero while fading */
extern UnkStruct_0213b414 data_0213b414;  /* JP 0x0213B414 */
extern u32 data_0213b784[];               /* JP 0x0213B784: flags */
extern u32 data_0213b84c[];               /* JP 0x0213B84C: flags */
extern u8 data_0213b230[];                /* JP 0x0213B230 */

#define DATA_0213A71C ((UnkStruct_0213a71c_6 *)&data_0213a71c)

void Exam_Init(void)
{
    func_0200f6e8(9);
    func_020038b8(data_02124410);
    func_02005fcc(data_021243d0);
    func_020c36cc(data_0213b230, Exam_Draw, Exam_FreeIcon, 0x60);
    Snd_LoadBank0C();
    func_020135b8(0, 0, data_021243e4);
    func_020135b8(1, 0, data_02124450);
    func_020c31c8(data_0213b214.itemId, 1);
    data_0213b214.unk264 = -50;
    data_0213b214.unk274 = 0x14C;
    data_0213b414.unk24 = 0x7FFF;
    func_020f4658(1, 0, 2, 0, 0x1F, 0);
    ItemSprite_Init(&EXAM_WORK->icon, 0);
    REG_DISPCNT = (REG_DISPCNT & ~0x1F00) | 0x1D00;
    func_020c4258(1);
    func_0206fb04(0, 2, 0, 0x100);
}

void Exam_Wait(void)
{
    switch (data_0213b214.unk21) {
    case 0:
        data_0213b214.unk21 = 1;
        data_0213b214.speed = 0x14;
        data_0213b214.unk24 = 0x28;
        data_0213b214.unk28 = 1;
        break;
    case 1:
    default:
        data_0213b214.speed--;
        if (data_0213b214.speed == 0) {
            func_020c4188(0x19, 0);
        }
        break;
    }
}

/* Zoom the model in while spinning it, then start the screen. */
void Exam_ZoomIn(void)
{
    if (data_0213b214.unk21 == 0) {
        data_0213b214.unk21 = 1;
        data_0213b414.rotX = -0x1000;
        data_0213b414.rotY = -0x800;
        data_0213b214.unk274 = 0x14C;
    } else if (data_0213b214.unk274 >= 0xD00) {
        func_020c4258(2);
        data_0213b414.rotX = 0;
        data_0213b414.rotY = 0;
        data_0213b214.unk274 = 0xD00;
    } else {
        data_0213b414.rotX += 0x1000;
        data_0213b414.rotY += 0x800;
        data_0213b214.unk274 += 0x60;
    }
}

void Exam_Main(void)
{
    if (data_0213a708 == 0) {
        data_021243d8[data_0213b214.unk21]();
    }
}

/* Turn the model, or confirm (examine) / cancel (leave). */
void Exam_Control(void)
{
    s32 r;
    u8 act;

    EXAM_WORK->hideButtons = 0;
    EXAM_WORK->msg = func_02075028(data_0213b214.itemId) + 0xE900;
    EXAM_WORK->msgArg = 0x8200;
    r = func_02013328(0);
    act = 0;
    switch (r) {
    case 0:
        act = 1;
        break;
    case 1:
        act = 2;
        break;
    default:
        if (data_0213a6c0.trg & 0x10) {
            act = 1;
        } else if (data_0213a6c0.trg & 0x40) {
            act = 2;
        }
        break;
    }
    switch (act) {
    case 0:
        Exam_Input();
        break;
    case 1:
        data_0213b214.unk21 = 1;
        break;
    case 2:
        Snd_Play0(2);
        func_020c4258(4);
        break;
    }
}

BOOL Exam_CheckAngle(u16 angle);

/*
 * Examine the item. Some items reveal something only when the model is
 * turned into one of a few angle ranges.
 */
void Exam_Check(void)
{
    u8 info = data_0211d738[data_0213b214.itemId].examine;
    u16 msg;
    u8 b;
    u8 ok;
    u8 n;

    if (!(info & 0x80)) {
        b = data_02124400[info];
        if (b & 0xF0) {
            EXAM_WORK->ranges = data_0212446c[b & 0xF];
            n = b >> 4;
            do {
                ok = 1;
                ok |= Exam_CheckAngle(data_0213b414.rotX) << 1;
                ok |= Exam_CheckAngle(data_0213b414.rotY) << 2;
            } while (--n != 0 && ok != 7);
            if (ok != 7) {
                goto other;
            }
            switch (data_0213b214.itemId) {
            case 0x3E:
            case 0x3F:
            case 0x40:
                if (data_0213b214.unk25 == 0) {
                    func_020c4258(3);
                    return;
                }
                break;
            }
            Flag_Set(data_0213b84c, info);
            msg = data_0211d690[info];
        } else {
            Flag_Set(data_0213b84c, info);
            msg = data_0211d690[info];
        }
    } else {
        switch (data_0213b214.itemId) {
        case 5:
        case 7:
        case 8:
        case 9:
            if (INV_SLOTS[data_0213b214.slot][1] == 0) {
                goto other;
            }
            DATA_0213A71C->unk06 = data_0213b214.itemId + 9;
            msg = 0xEF;
            break;
        case 0x13:
            if (Flag_Test(data_0213b784, 0x13) || Flag_Test(data_0213b784, 0)) {
                msg = 0xF0;
                break;
            }
        default:
        other:
            msg = data_0213b214.itemId | 0xE800;
            break;
        }
    }
    EXAM_WORK->msg = msg;
    EXAM_WORK->msgArg = 0;
    EXAM_WORK->hideButtons = 1;
    data_0213b214.unk21 = 2;
}

/* Is the angle inside the next range? */
BOOL Exam_CheckAngle(u16 angle)
{
    BOOL ret = TRUE;
    ExamRange *r = EXAM_WORK->ranges;
    u16 width = r->width;
    u16 center = r->center;

    EXAM_WORK->ranges = r + 1;
    if (width != 0 && (u16)(width + (angle - center)) > width * 2) {
        ret = FALSE;
    }
    return ret;
}

void Exam_Cancel(void)
{
    if (!(data_0213a71c.unk05 & 0x80)) {
        data_0213b214.unk21 = 0;
    }
}
