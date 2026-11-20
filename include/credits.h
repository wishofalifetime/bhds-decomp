#ifndef BHDS_CREDITS_H
#define BHDS_CREDITS_H

#include "types.h"
#include "scd_data.h"

/* One scrolling column. */
typedef struct CreditsLine {
    /* 0x00 */ s32 pos;         /* 24.8 fixed */
    /* 0x04 */ u16 index;       /* next line in data_0212eb2c */
    /* 0x06 */ u8 lastRow;
    /* 0x07 */ u8 pad07;
} CreditsLine;

typedef struct CreditsWork {
    /* 0x00 */ CreditsLine lines[2];
    /* 0x10 */ s32 fade0;
    /* 0x14 */ u8 unk14[4];
    /* 0x18 */ s32 fade1;
    /* 0x1C */ u8 unk1C[4];
    /* 0x20 */ s32 fade2;
} CreditsWork;

typedef struct UnkDtcm_BgScroll {
    /* 0x00 */ u8 unk00[0x12];
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 unk14[0x22 - 0x14];
    /* 0x22 */ s16 unk22;
    /* 0x24 */ u8 unk24[0x32 - 0x24];
    /* 0x32 */ s16 unk32;
} UnkDtcm_BgScroll;

typedef struct UnkDtcm_027e01bc {
    /* 0x00 */ u8 unk00[4];
    /* 0x04 */ u16 unk04;
    /* 0x06 */ u16 size;
    /* 0x08 */ u8 dirty;
    /* 0x09 */ u8 unk09[3];
    /* 0x0C */ void *src;
} UnkDtcm_027e01bc;

#define CREDITS ((CreditsWork *)data_0213b214.work)

/* credits.c */
void Credits_Init(void);              /* JP 0x020B8B58 */
void Credits_Update(void);            /* JP 0x020B89D0 */
u32 Credits_CheckSkip(BOOL done);     /* JP 0x020B8994 */
BOOL Credits_ScrollLine(u32 which);   /* JP 0x020B88A4 */

/* credits_text.c */
void Credits_SetScroll(s32 a, s32 b); /* JP 0x020B8848 */
void Credits_Exit(void);              /* JP 0x020B87F8 */
void Credits_WriteText(u32 which, u8 x, u8 y, const void *src, u16 len, u8 palette);  /* JP 0x020B8778 */

#endif /* BHDS_CREDITS_H */
