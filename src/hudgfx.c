/*
 * hudgfx.c - room setup hook and sprite draw helper.
 * JP (ABHJ) 0x020B8D9C - 0x020B8DF4.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"

typedef struct UnkStruct_0213a900 {
    /* 0x00 */ u8 unk00[0x1E];
    /* 0x1E */ u16 slotMask;
} UnkStruct_0213a900;

/* Not yet identified. JP addresses. */
extern void func_01ffc510(void *sprite);  /* ITCM */
extern void func_020aedd8(void);
extern void Bgm_ClearSlots(void);
extern void G3d_SetMaterialSlots(void *material);

extern UnkStruct_0213a900 data_0213a900;  /* JP 0x0213A900 */

/* Set a sprite's material and draw it. */
void Hud_DrawSprite(void *sprite)
{
    G3d_SetMaterialSlots(sprite);
    func_01ffc510(sprite);
}

void Hud_ResetRoomMusic(void)
{
    data_0213a71c.unk03 = 0x1F;
    Bgm_ClearSlots();
    data_0213a900.slotMask = 0xFF;
    func_020aedd8();
}
