#ifndef BHDS_MTX_H
#define BHDS_MTX_H

#include "types.h"

/* 3x3 fixed-point (20.12) matrix. */
typedef struct MtxFx33 {
    s32 m[3][3];
} MtxFx33;

extern s32 func_01ff8000(s32 idx);  /* ITCM, Thumb: sine */
extern s32 func_01ff8010(s32 idx);  /* ITCM, Thumb: cosine */
extern void func_020fd810(MtxFx33 *a, MtxFx33 *b, MtxFx33 *out);  /* probably MTX_Concat33 */

void Scd_MakeRotation(s16 *angles, MtxFx33 *out);  /* JP 0x020B4CBC */

#endif /* BHDS_MTX_H */
