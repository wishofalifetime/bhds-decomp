/*
 * coll.c - room collision boxes.
 * JP (ABHJ) 0x020B5564 - 0x020B5B9C.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"
#include "obj.h"

typedef struct CollPos {
    s32 x;
    s32 y;
    s32 z;
    s32 w;
} CollPos;

typedef s32 (*CollHandler)(RoomGroupItem *item, CollPos *pos, s16 *out);

/* Not yet identified. */
extern s32 func_01ffbd98(void *pos, s32 x0, s32 z0, s32 x1, s32 z1);  /* ITCM */
extern s32 func_01ffbdf4(CollPos *pos, void *out, s16 a, s16 b);    /* ITCM */
extern s32 func_01ffbb30(RoomGroupItem *item, CollPos *pos, s16 *out);  /* ITCM */
extern s32 func_01ffba10(RoomGroupItem *item, CollPos *pos, s16 *out);  /* ITCM */

typedef struct CollMtx {
    s32 m[8];
} CollMtx;

extern void func_021188b8(void *dst, u32 size);  /* runtime: clear bytes */
extern void func_01ff84ac(u16 angle, CollMtx *m);                 /* ITCM */
extern void func_01ff83fc(CollMtx *m, void *v, CollPos *out);     /* ITCM */
extern void func_01ff834c(CollMtx *m, void *v, s16 *out);         /* ITCM */

extern Obj *data_027e000c;           /* DTCM: current object */
extern s16 data_027e0028[3];         /* DTCM */
extern CollMtx data_027e007c;        /* DTCM */
extern CollMtx data_027e00bc;        /* DTCM */
extern s32 data_027e0018;            /* DTCM */
extern CollPos data_027e0054;        /* DTCM */
extern CollHandler data_0213a480[];  /* JP 0x0213A480: handler by box type */

s32 Coll_TestBoxType(void *pos, RoomGroupItem *item, s32 margin);
u8 Coll_CheckMove(void *from, void *to);
s32 Coll_TestBoxFlags(void *pos, RoomGroupItem *item, s32 margin);
s32 Coll_Handler4(RoomGroupItem *item, CollPos *pos, s16 *out);

/*
 * Check the current object's move against the room's collision boxes,
 * running each box's handler; undo the move when it is blocked.
 */
u8 Coll_CheckMove(void *from, void *to)
{
    RoomGroupItem **groups[2];
    s16 res[2];
    CollPos pos[2];
    s16 v[2][4];
    RoomGroupTable *t;
    RoomGroupItem *item;
    u32 type;
    Obj *obj;
    u8 i;
    RoomGroupItem **group;

    func_021188b8(res, 4);
    obj = data_027e000c;
    if (obj->unk08 & 4) {
        return 0;
    }
    data_027e00bc = data_027e007c;
    func_01ff84ac(obj->unk44, &data_027e00bc);
    func_01ff83fc(&data_027e00bc, from, &pos[0]);
    func_01ff83fc(&data_027e00bc, to, &pos[1]);
    obj = data_027e000c;
    data_027e00bc = data_027e007c;
    func_01ff84ac(obj->unk4E, &data_027e00bc);
    func_01ff834c(&data_027e00bc, from, v[0]);
    func_01ff834c(&data_027e00bc, to, v[1]);
    obj = data_027e000c;
    v[0][0] += obj->initX;
    v[0][2] += obj->initZ;
    v[1][0] += obj->initX;
    v[1][2] += obj->initZ;

    i = 1;
    do {
        obj = data_027e000c;
        pos[i].x += obj->posX;
        pos[i].z += obj->posZ;
        data_027e0028[0] = 0;
        data_027e0028[1] = 0;
        data_027e0028[2] = 0;
        t = data_0214b640.room->groups;
        data_027e0018 = func_01ffbdf4(&pos[i], data_027e0028, *(s32 *)&t->unk00, t->unk02);
        groups[i] = &data_0214b640.room->groups->groups[1] + data_027e0018;
        data_027e0054 = pos[i];
        item = groups[i][-1];
        while (item < *groups[i]) {
            type = item->unk08 & 0xFF;
            if (type == 4) {
                item++;
            } else if (type - 4 <= 1) {
                item++;
            } else {
                data_027e0018 = Coll_TestBoxType(data_027e0028, item, data_027e000c->unk0C[5]);
                if (data_027e0018 != -1) {
                    data_0213a480[data_027e0018](item, &pos[i], v[i]);
                    res[i] |= (item->flags & 0x300) >> 8;
                }
                item++;
            }
        }
    } while (i-- != 0);

    if (res[0] == 0) {
        data_027e000c->initX = data_027e000c->posX;
        data_027e000c->initY = data_027e000c->posY;
        data_027e000c->initZ = data_027e000c->posZ;
        data_027e000c->unk4E = data_027e000c->unk44;
        return res[1];
    }

    data_027e0054 = pos[1];
    group = groups[1];
    item = group[-1];
    while (item < group[0]) {
        type = item->unk08 & 0xFF;
        if (type == 4) {
            item++;
        } else if (type - 4 <= 1) {
            item++;
        } else {
            if (!(Coll_TestBoxType(data_027e0028, item, data_027e000c->unk0C[5]) & 0x8000)) {
                res[1] |= (item->flags & 0x300) >> 8;
            }
            item++;
        }
    }
    if (res[1] != 0) {
        data_027e000c->posX = data_027e000c->initX;
        data_027e000c->initY = data_027e000c->posY;
        data_027e000c->posZ = data_027e000c->initZ;
        data_027e000c->unk44 = data_027e000c->unk4E;
        return 0x80;
    }
    data_027e000c->initX = data_027e000c->posX;
    data_027e000c->initY = data_027e000c->posY;
    data_027e000c->initZ = data_027e000c->posZ;
    data_027e000c->unk4E = data_027e000c->unk44;
    return 1;
}

/* Box type (low byte of unk08) if pos is inside item shrunk by margin, else -1. */
s32 Coll_TestBoxType(void *pos, RoomGroupItem *item, s32 margin)
{
    if (func_01ffbd98(pos, item->unk00 + margin, item->unk02 + margin,
                      item->unk04 - margin, item->unk06 - margin) == 0) {
        return (s16)(item->unk08 & 0xFF);
    }
    return -1;
}

/* Set up the room's collision groups and the box handlers. */
void Coll_InitRoom(void)
{
    RoomGroupItem **p;
    RoomGroupTable *t = data_0214b640.room->groups;
    u32 n = (u32)t->groups[0];

    t->groups[0] = (RoomGroupItem *)(t + 1);
    p = &t->groups[1];
    for (data_027e0018 = 0; data_027e0018 < 4; data_027e0018++) {
        u32 count = (u32)*p;

        *p = (RoomGroupItem *)(t + 1) + n;
        n += count;
        p++;
    }
    data_0213a480[1] = func_01ffbb30;
    data_0213a480[3] = func_01ffba10;
    data_0213a480[4] = Coll_Handler4;
    data_0213a480[5] = func_01ffbb30;
}

/* Box type 4: flag the current object. */
s32 Coll_Handler4(RoomGroupItem *item, CollPos *pos, s16 *out)
{
    data_027e000c->unkC0 |= 8;
    return 1;
}

/* Flags (bits 8-9) of the boxes pos is in; 1 if any has both. */
u16 Coll_GetFlags(CollPos *pos, void *out)
{
    RoomGroupItem **group;
    RoomGroupItem *item;
    s32 result = 0;
    RoomGroupTable *t = data_0214b640.room->groups;
    s16 i = func_01ffbdf4(pos, out, *(s32 *)&t->unk00, t->unk02);

    t = data_0214b640.room->groups;
    data_027e0054 = *pos;
    group = &t->groups[1] + i;
    for (item = group[-1]; item < group[0]; item++) {
        s32 f = Coll_TestBoxFlags(out, item, 0);

        if (f != -1) {
            f = (s16)(f & 0x300);
            if (f == 0x300) {
                return 1;
            }
            result |= f;
        }
    }
    return result;
}

/* High byte of the box flags if pos is inside item shrunk by margin, else -1. */
s32 Coll_TestBoxFlags(void *pos, RoomGroupItem *item, s32 margin)
{
    if (func_01ffbd98(pos, item->unk00 + margin, item->unk02 + margin,
                      item->unk04 - margin, item->unk06 - margin) == 0) {
        return (s16)(item->flags & 0xFF00);
    }
    return -1;
}
