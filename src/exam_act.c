/*
 * exam_act.c - the item examine screen's actions: playing the model's
 * animation, using the item and the per-state update.
 * JP (ABHJ) 0x020BD4F4 - 0x020BD664.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "game.h"
#include "mi.h"
#include "pad.h"
#include "scd_data.h"
#include "exam.h"

/* Not yet identified. JP addresses. */
extern void func_020062ac(u32 layer, ExamSprite *spr);
extern s32 func_0200e980(s32 x, s32 lo, s32 hi);  /* clamp */
extern s32 func_02013328(u32 a);
extern s32 func_02013340(u32 a);
extern void func_020767b0(u8 *pal);
extern u8 func_02076974(u32 id);
extern void func_020769b4(u8 *slot);
extern u8 func_02076bc8(u32 id, TexRef *ref);
extern void func_02076d18(SceneModel *model, u32 a, u32 b);
extern void func_02092940(u16 msg, u32 a, u32 b, u32 arg);
extern void func_020c31c8(u8 item);
extern void func_020c3224(void *a);
extern void func_020c4258(u32 a);
extern void func_01ffc510(ItemSprite *s);
extern void G3d_SetMaterialSlots(ItemSprite *s);

extern ItemStackInfo data_0211d738[];     /* JP 0x0211D738 */
extern u8 data_0211d690[];                /* JP 0x0211D690: examine message by index */
extern u8 data_021243cc[];                /* JP 0x021243CC: key bit per button */
extern ExamPos data_021243e4[];           /* JP 0x021243E4 */
extern VoidFunc data_021243f0[];          /* JP 0x021243F0: by data_0213b214.unk21 */
extern ItemIcon data_02124438[];          /* JP 0x02124438 */
extern ExamPos data_02124450[];           /* JP 0x02124450: button positions */
extern Pad data_0213a6c0;                 /* JP 0x0213A6C0 */
extern UnkStruct_0213b414 data_0213b414;  /* JP 0x0213B414 */
extern u8 data_0213b46c[];                /* JP 0x0213B46C (data_0213b214.model) */
extern u32 data_0213b84c[];               /* JP 0x0213B84C: flags */


void Exam_Update(void)
{
    EXAM_WORK->hideButtons = 1;
    data_021243f0[data_0213b214.unk21]();
}

void Exam_StartModel(void)
{
    func_02076d18(data_0213b214.model, 0x1E, 0);
    data_0213b214.unk21 = 1;
}

/* Play the model's animation; at the end show the examine message. */
void Exam_Animate(void)
{
    u8 idx;

    data_0213b214.model->anim->frame += 0x1000;
    if (data_0213b214.model->anim->frame >= 0x28000) {
        idx = data_0211d738[data_0213b214.itemId].examine;
        if (data_0213b214.itemId == 0x3E) {
            Flag_Set(data_0213b84c, idx);
        }
        EXAM_WORK->msg = data_0211d690[idx];
        EXAM_WORK->msgArg = 0;
        data_0213b214.unk25 = 1;
        data_0213b214.unk21 = 2;
    }
}

void Exam_Use(void)
{
    if (data_0213a71c.unk05 & 0x80) {
        return;
    }
    switch (data_0213b214.itemId) {
    case 0x3F:
    case 0x40:
        data_0213b214.unk28 = 1;
        data_0213b214.unk21 = 3;
        break;
    default:
        func_020c4258(2);
        break;
    }
}
