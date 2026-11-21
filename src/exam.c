/*
 * exam.c - the item examine screen: the item's 3D model turned and zoomed
 * with the buttons, the screen's drawing and the item icon sprite.
 * JP (ABHJ) 0x020BD030 - 0x020BD47C.
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
#ifdef VERSION_EU
typedef struct UnkStruct_0213dd14 {
    /* 0x00 */ u8 unk00[3];
    /* 0x03 */ u8 unk03;
} UnkStruct_0213dd14;

extern UnkStruct_0213dd14 data_0213dd14;  /* JP 0x0213DD14 (only used by EU here) */
#endif
extern u32 data_0213b84c[];               /* JP 0x0213B84C: flags */


/* Turn the model with the D-pad (or touch) and zoom with L/R. */
void Exam_Input(void)
{
    s32 dir;
    u32 keys;
    s32 rotX;
    s32 rotY;
    s32 zoom;
    u16 cont;

    if (EXAM_WORK->analog == 0) {
        dir = func_02013328(1);
    } else {
        dir = func_02013340(1);
    }
    if (dir >= 0) {
        keys = 1 << dir;
        EXAM_WORK->analog = 1;
    } else {
        EXAM_WORK->analog = 0;
        keys = 0;
        cont = data_0213a6c0.cont;
        if (cont & 4) {
            keys |= 1;
        }
        if (cont & 8) {
            keys |= 2;
        }
        if (cont & 2) {
            keys |= 4;
        }
        if (cont & 1) {
            keys |= 8;
        }
        if (cont & 0x100) {
            keys |= 0x10;
        }
        if (cont & 0x200) {
            keys |= 0x20;
        }
    }
    EXAM_WORK->keys = keys;
    rotX = data_0213b414.rotX;
    rotY = data_0213b414.rotY;
    zoom = data_0213b214.unk274;
    if (keys & 1) {
        rotY += 0x400;
    }
    if (keys & 2) {
        rotY -= 0x400;
    }
    if (keys & 4) {
        rotX -= 0x400;
    }
    if (keys & 8) {
        rotX += 0x400;
    }
    if (keys & 0x10) {
        zoom -= 0x80;
    }
    if (keys & 0x20) {
        zoom += 0x80;
    }
    data_0213b414.rotX = rotX;
    data_0213b414.rotY = rotY;
    data_0213b214.unk274 = func_0200e980(zoom, 0xD00, 0x1800);
}

void ItemSprite_Draw(ItemSprite *s);

void Exam_Draw(void)
{
    ExamSprite spr;
    u32 i;
    ExamWork *w;

    if (EXAM_WORK->hideButtons == 0) {
        for (i = 0; i <= 3; i++) {
            MI_CpuClear16(&spr, sizeof(spr));
            spr.id = (u8)((EXAM_WORK->keys & data_021243cc[i]) != 0) + i * 2 + 2;
            spr.x = data_02124450[i].x;
            spr.y = data_02124450[i].y;
            spr.frame = 1;
            func_020062ac(1, &spr);
        }
    }
    EXAM_WORK->keys = 0;
    MI_CpuClear16(&spr, sizeof(spr));
    if (data_0213b214.unk24 != 0) {
        data_0213b214.unk24--;
    }
    spr.id = ((data_0213b214.unk24 & 4) != 0) + 10;
    spr.x = data_021243e4[1].x;
    spr.y = data_021243e4[1].y;
    spr.frame = 1;
    func_020062ac(1, &spr);
    ItemSprite_Draw(&EXAM_WORK->icon);
    if (data_0213b214.model != NULL) {
        if (data_0213b214.unk28 == 1) {
            if (data_0213b214.unk274 < 0x60) {
                data_0213b214.unk274 = 0;
                data_0213b214.unk28 = 2;
            } else {
                data_0213b214.unk274 -= 0xC0;
            }
            data_0213b414.rotY -= 0x1000;
            data_0213b414.rotX -= 0x2000;
        }
        func_020c3224(data_0213b46c);
        w = EXAM_WORK;
        if (w->shownMsg != w->msg) {
            w->shownMsg = w->msg;
            data_0213a71c.unk05 = 0;
            func_02092940(EXAM_WORK->shownMsg, 0, 0x13, EXAM_WORK->msgArg);
#ifdef VERSION_EU
            data_0213dd14.unk03 = 0x80;
#endif
        }
    }
}

void ItemSprite_Free(ItemSprite *s);

void Exam_FreeIcon(void)
{
    ItemSprite_Free(&EXAM_WORK->icon);
}

/* Set up the icon sprite for item icon idx. */
void ItemSprite_Init(ItemSprite *s, s32 idx)
{
    ItemIcon *icon = &data_02124438[idx];

    s->spr.tex = &s->tex;
    data_02143640.unk0E = 1;
    s->tex.slot = func_02076bc8(0x36, &s->tex);
    s->tex.pal = func_02076974(0x36);
    s->tex.unk08 = 5;
    data_02143640.unk0E = 0;
    s->spr.x = icon->x;
    s->spr.y = icon->y;
    s->spr.width = 0xFFF;
    s->spr.unk06 = 0;
    s->spr.unk08 = 0;
    s->spr.unk0A = icon->unk02;
    s->spr.unk0C = icon->unk04;
    s->spr.unk0E = s->spr.unk0A;
    s->spr.unk10 = s->spr.unk0C;
    s->spr.color = icon->color;
    s->spr.scaleX = 0x1000;
    s->spr.scaleY = 0x1000;
    s->spr.unk20 = 0;
    s->spr.unk22 = 0;
    s->spr.alpha = 0x1F;
    s->spr.palette = 0;
    s->spr.format = 3;
    s->spr.polyId = 0xC;
}

void ItemSprite_Draw(ItemSprite *s)
{
    G3d_SetMaterialSlots(s);
    func_01ffc510(s);
}

void ItemSprite_Free(ItemSprite *s)
{
    func_020769b4(&s->tex.slot);
    func_020767b0(&s->tex.pal);
}
