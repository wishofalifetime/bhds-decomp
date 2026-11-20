/*
 * scd_rot.c - item removal, sequence state and the rotation matrix helper.
 * JP (ABHJ) 0x020B4C28 - 0x020B4DEC.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd.h"
#include "scd_data.h"
#include "game.h"
#include "mtx.h"

extern void func_02017be8(void *entry);

extern MtxFx33 data_0212e30c;        /* JP 0x0212E30C: identity, rotated about Z */
extern MtxFx33 data_0212e330;        /* JP 0x0212E330: identity, rotated about Y */
extern MtxFx33 data_027e0148;        /* DTCM scratch matrix */

typedef struct UnkMarker {
    u8 unk00[0xBC];
} UnkMarker;
extern UnkMarker data_02148320[];    /* JP 0x02148320 */

/* out = RotZ(angles[2]) * RotY(angles[1]); angles in 1/16 sine-table steps. */
void Scd_MakeRotation(s16 *angles, MtxFx33 *out)
{
    s32 s;
    s32 c;
    s32 a;
    MtxFx33 tmp;
    MtxFx33 rz;
    MtxFx33 ry;

    a = angles[2] << 4;
    c = func_01ff8010(a);
    s = func_01ff8000(a);
    rz = data_0212e30c;
    rz.m[0][0] = c;
    rz.m[1][1] = c;
    rz.m[0][1] = s;
    rz.m[1][0] = -s;
    data_027e0148 = rz;

    a = angles[1] << 4;
    c = func_01ff8010(a);
    s = func_01ff8000(a);
    ry = data_0212e330;
    ry.m[0][0] = c;
    ry.m[2][2] = c;
    ry.m[0][2] = s;
    ry.m[2][0] = -s;
    tmp = ry;
    func_020fd810(&data_027e0148, &tmp, out);
}

/* Set the 4-byte sequence state; marks data_02143640.unkFDA on first use. */
void Scd_SetSeq(u8 a, u8 b, u8 c, u8 d)
{
    gScd.seq[0] = a;
    gScd.seq[1] = b;
    gScd.seq[2] = c;
    gScd.seq[3] = d;
    if (data_02143640.unkFDA == 0) {
        gScd.unk245 = 1;
        data_02143640.unkFDA = 1;
    }
}

/* Remove the item placed by entry: clear it and drop its marker. */
void Scd_RemoveItem(UnkEntry_02147d0c *entry)
{
    Obj *obj = data_0214c9e8[entry->script[0xA]];

    entry->unk00 = 0;
    obj->unk08 = 0;
    if (obj->unk92 != 0) {
        func_02017be8(&data_02148320[obj->unk92]);
        obj->unk92 = 0;
    }
}
