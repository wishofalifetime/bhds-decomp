/*
 * choice_draw.c - drawing and moving the two-sided screen's markers, and
 * leaving to the next scene.
 * JP (ABHJ) 0x020BC6CC - 0x020BCA10. Not matching yet: in Choice_Draw and
 * Choice_Start the original converts constant arguments (a u16 0 through
 * the stack, a u8 1 with and #0xFF) where our build folds them; in
 * Choice_Exit the original does not if-convert the scene assignments and
 * keeps two pool entries for func_020d6468.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "choice.h"
#include "game.h"

typedef struct UnkStruct_0213a900 {
    /* 0x00 */ u8 unk00[0x20];
    /* 0x20 */ u8 unk20;
} UnkStruct_0213a900;

/* Not yet identified. JP addresses. */
extern void func_02072354(void);
extern BOOL func_02087ad4(void);
extern void func_0208e610(void);
extern void func_020adf8c(void);
extern void func_020d6468(void);
extern void SndBank_UnloadAll(void);
extern u8 SndCh_GetStatus(u32 ch);
extern u16 data_0213a708;                 /* JP 0x0213A708: nonzero while fading */
extern UnkStruct_0213a900 data_0213a900;  /* JP 0x0213A900 */
extern void func_020062ac(u32 layer, ChoiceSprite *spr);
extern void func_02091a88(u32 a, void *b, u32 c, u32 d);
extern void func_020bf378(u32 a);
extern void func_02105aac(u16 data, void *dst, u32 size);  /* MIi_CpuClear16 */

extern ChoicePos data_02124394[];         /* JP 0x02124394: button positions */
extern u8 data_0213f168[];                /* JP 0x0213F168 */

/* NitroSDK MI_CpuFill16 / MI_CpuClear16 */
static inline void MI_CpuFill16(void *dest, u16 data, u32 size)
{
    func_02105aac(data, dest, size);
}

static inline void MI_CpuClear16(void *dest, u32 size)
{
    MI_CpuFill16(dest, 0, size);
}

static inline u32 Choice_Attr(u8 a)
{
    return (a << 24) | 0x1100;
}

/* Pick the next scene once the fade has finished. */
void Choice_Exit(void)
{
    u8 mode;

    if (data_0213a708 != 0) {
        return;
    }
    mode = data_0213a900.unk20;
    gGame.unk01 = 1;
    if (mode == 2) {
        if (SndCh_GetStatus(1) != 1) {
            SndBank_UnloadAll();
            if (data_0213b214.unk1D == 0) {
                gGame.nextScene = func_0208e610;
            } else {
                if (func_02087ad4() == 0) {
                    gGame.nextScene = func_020d6468;
                } else {
                    gGame.nextScene = func_02072354;
                }
            }
        }
    } else {
        if (data_0213b214.unk1D == 0) {
            gGame.nextScene = func_020d6468;
        } else {
            gGame.nextScene = func_020adf8c;
        }
    }
}

void Choice_Start(void)
{
    ChoiceWork *w = CHOICE_WORK;
    u8 kind = w->side[w->cur].kind;

    if (kind == 2) {
        w->unk44 = 0;
    } else {
        w->unk44 = 1;
    }
    func_020bf378(0);
    func_02091a88(kind | 0xE700, data_0213f168, 0x40, Choice_Attr(1));
}

void Choice_SetPos(s32 side, BOOL neg)
{
    s32 v;
    ChoiceSide *s;

    if (neg == FALSE) {
        v = 0x8000;
    } else {
        v = -0x8000;
    }
    s = &CHOICE_WORK->side[side];
    s->x = v;
    s->y = v;
}

void Choice_Move(void)
{
    CHOICE_WORK->side[1].x += CHOICE_WORK->side[1].vx;
    CHOICE_WORK->side[1].y += CHOICE_WORK->side[1].vy;
    CHOICE_WORK->side[0].x += CHOICE_WORK->side[0].vx;
    CHOICE_WORK->side[0].y += CHOICE_WORK->side[0].vy;
}

void Choice_Draw(void)
{
    ChoiceSprite spr;
    ChoiceWork *w;
    u8 i;
    u8 id;

    MI_CpuClear16(&spr, sizeof(spr));
    if (data_0213b214.unk1D == 0) {
        for (i = 0; i < 2; i++) {
            spr.x = data_02124394[i].x + 4;
            spr.y = data_02124394[i].y + 4;
            id = i * 2 + 0xE;
            if (CHOICE_WORK->blink[i] == 1) {
                id += Choice_Blink();
            }
            spr.id = id;
            func_020062ac(1, &spr);
        }
    }
    w = CHOICE_WORK;
    spr.frame = w->side[1].count + 1;
    if (data_0213b214.unk1E == 1) {
        spr.frame = 1;
    }
    spr.id = 1;
    spr.x = (w->side[1].x >> 12) + 0x88;
    spr.y = (w->side[1].y >> 12) + 0x60;
    func_020062ac(0, &spr);
    w = CHOICE_WORK;
    spr.frame = w->side[0].count + 1;
    if (data_0213b214.unk1E == 1) {
        spr.frame = 1;
    }
    spr.id = 0;
    spr.x = (w->side[0].x >> 12) + 0x88;
    spr.y = (w->side[0].y >> 12) + 0x60;
    func_020062ac(0, &spr);
    Choice_UpdateCursor();
}
