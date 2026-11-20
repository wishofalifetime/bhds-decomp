/*
 * scd_rotnode.c - applies a node's rotation to its joint matrix.
 * JP (ABHJ) 0x020B4DEC - 0x020B4E9C. Not matching yet: the table base and
 * index loads swap registers.
 */
#include "types.h"
#include "mtx.h"

typedef struct UnkJoint {
    u8 unk00[0x28];
    MtxFx33 rot;
    u8 unk4C[0x58 - 0x4C];
} UnkJoint;
extern UnkJoint *data_0213fc68;      /* JP 0x0213FC68 */

/* Something with a rotation applied to a 0x58-byte table entry. */
typedef struct UnkRotNode {
    /* 0x00 */ struct {
                    u8 unk00[0x14];
                    u8 *unk14;
                } *unk00;
    /* 0x04 */ u8 unk04[0xBC - 0x04];
    /* 0xBC */ u8 enabled;
    /* 0xBD */ u8 unkBD[0xC1 - 0xBD];
    /* 0xC1 */ u8 index;
    /* 0xC2 */ s16 angleY;
    /* 0xC4 */ s16 angleZ;
    /* 0xC6 */ u8 unkC6[2];
    /* 0xC8 */ MtxFx33 base;
} UnkRotNode;

/* Apply node's rotation to its entry in the 0x58-byte table. */
void Scd_UpdateRotNode(UnkRotNode *node)
{
    UnkJoint *entry;

    if (node->enabled) {
        entry = &data_0213fc68[node->unk00->unk14[1]];
        if (entry != NULL) {
            s16 angles[3];
            MtxFx33 mtx;
            UnkJoint *dst = &entry[node->index];

            angles[0] = 0;
            angles[1] = node->angleY;
            angles[2] = -node->angleZ;
            Scd_MakeRotation(angles, &mtx);
            func_020fd810(&mtx, &node->base, &mtx);
            dst->rot = mtx;
        }
    }
}
