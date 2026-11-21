/*
 * choice.c - a two-sided screen: two markers that move by their speed,
 * show a count each, and two blinking buttons. The cursor and blink timer.
 * JP (ABHJ) 0x020BC624 - 0x020BC6CC.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "choice.h"

typedef struct UnkDtcm_027e01ac {
    /* 0x00 */ u8 unk00[0x20];
    /* 0x20 */ s16 x;
    /* 0x22 */ s16 y;
} UnkDtcm_027e01ac;

extern UnkDtcm_027e01ac data_027e01ac;    /* DTCM */

/* Put the cursor on the side with the higher count. */
void Choice_UpdateCursor(void)
{
    ChoiceWork *w = CHOICE_WORK;
    u8 i = w->side[1].count > w->side[0].count;

    data_027e01ac.x = (w->side[i].x >> 12) - 0x10;
    data_027e01ac.y = (w->side[i].y >> 12) - 8;
}

/* Count down the blink timer; off for one frame in every ten. */
BOOL Choice_Blink(void)
{
    BOOL ret = TRUE;

    if (data_0213b214.blink != 0) {
        if (data_0213b214.blink % 10 == 0) {
            ret = FALSE;
        }
        data_0213b214.blink--;
    } else {
        ret = FALSE;
    }
    return ret;
}
