/*
 * reward_scene.c - the bonus item scene's setup, the model zooming in and
 * moving on to the next item.
 * JP (ABHJ) 0x020BE11C - 0x020BE45C.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "exam.h"
#include "hw_regs.h"
#include "pad.h"
#include "scd_data.h"

/* Not yet identified. JP addresses. */
extern void func_020038b8(void *a);
extern void func_0200f6e8(u32 a);
extern void func_0200f770(u32 a);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern void func_02076244(void);
extern void func_0207a03c(void);
extern void func_0209fb5c(void);
extern void func_020c31c8(u8 item, u32 a);
extern void func_020c35e4(void);
extern void func_020c36cc(void *a, VoidFunc draw, VoidFunc free, u32 b);
extern void func_020f4658(u32 a, u32 b, u32 c, u32 d, u32 e, u32 f);
extern BOOL SndBank_Load(s32 slot, u32 id);
extern void Snd_Play0(u32 id);
extern void Reward_Draw(void);
extern void Reward_FreeIcon(void);
extern u8 Reward_Next(void);
extern void Reward_ShowMsg(u16 msg);

extern u8 data_0212449c[];                /* JP 0x0212449C */
extern VoidFunc data_021244b0[];          /* JP 0x021244B0: by data_0213b214.unk21 */
extern Pad data_0213a6c0;                 /* JP 0x0213A6C0 */
extern u8 data_0213a708;                  /* JP 0x0213A708: nonzero while fading */
extern UnkStruct_0213b414 data_0213b414;  /* JP 0x0213B414 */
extern u8 data_0213b230[];                /* JP 0x0213B230 */

void Reward_Update(void)
{
    data_021244b0[data_0213b214.unk21]();
}

void Reward_Init(void)
{
    func_0200f770(6);
    func_0200f6e8(9);
    func_020c36cc(data_0213b230, Reward_Draw, Reward_FreeIcon, 0x50);
    data_0213b214.unk1E = 0;
    data_0213b214.unk24 = 0;
    if (Reward_Next() == 0) {
        func_0206fb04(4, 1, 1, 0x100);
        data_0213b214.unk21 = 4;
        data_0213b214.unk22 = 0;
        data_0213b214.unk23 = 0;
    } else {
        func_0207a03c();
        func_02076244();
        func_0209fb5c();
        SndBank_Load(5, 3);
        func_020038b8(data_0212449c);
        func_020c35e4();
        func_020c31c8(data_0213b214.itemId, 1);
        data_0213b214.unk264 = -50;
        data_0213b214.unk274 = 0;
        data_0213b414.unk24 = 0x7FFF;
        func_020f4658(1, 0, 2, 0, 0x1F, 0);
        ItemSprite_Init((ItemSprite *)data_0213b214.work, 0);
        REG_DISPCNT = (REG_DISPCNT & ~0x1F00) | 0x1D00;
        func_0206fb04(0, 1, 0, 0x100);
        data_0213b214.unk21 = 1;
        data_0213b214.unk22 = 0;
        data_0213b214.unk23 = 0;
    }
}

/* Zoom the item's model in while spinning it, then show its message. */
void Reward_ZoomIn(void)
{
    if (data_0213b214.unk22 == 0) {
        data_0213b214.unk22 = 1;
        data_0213b414.rotX = -33.0 * 0x1000;
        data_0213b414.rotY = -33.0 * 0x800;
        data_0213b214.unk274 = 0x66;
    } else {
        data_0213b414.rotX += 0x1000;
        data_0213b414.rotY += 0x800;
        data_0213b214.unk274 += 144.0;
        if (4896.0 <= data_0213b214.unk274) {
            data_0213b214.unk274 = 0x1320;
            data_0213b214.unk21 = 2;
            data_0213b214.unk22 = 0;
            data_0213b214.unk23 = 0;
            Snd_Play0(data_0213b214.unk25);
            Reward_ShowMsg(data_0213b214.speed);
        }
    }
}

/* On A, go on to the next bonus item, or leave after the last one. */
void Reward_Input(void)
{
    if (data_0213a708 == 0 && data_0213b214.unk28 == 0 && !(data_0213a71c.unk05 & 0x80) &&
        (data_0213a6c0.trg & 0x10)) {
        if (Reward_Next() != 0) {
            data_0213b214.unk28 = 1;
            data_0213b214.unk21 = 3;
            data_0213b214.unk22 = 0;
            data_0213b214.unk23 = 0;
        } else {
            data_0213b214.unk28 = 1;
            data_0213b214.unk21 = 4;
            data_0213b214.unk22 = 0;
            data_0213b214.unk23 = 0;
            func_0206fb04(4, 1, 1, 0x100);
        }
    }
}
