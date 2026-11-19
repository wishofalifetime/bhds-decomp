#ifndef BHDS_PAD_H
#define BHDS_PAD_H

#include "types.h"

/* Button state, updated once per frame. JP: 0x0213A6B4 */
typedef struct Pad {
    /* 0x00 */ u16 cont;       /* held this frame */
    /* 0x02 */ u16 trg;        /* newly pressed this frame */
    /* 0x04 */ u16 prev;       /* held last frame */
    /* 0x06 */ u8 unk06[3];
    /* 0x09 */ u8 lidClosed;
} Pad;

extern Pad gPad;

void Pad_Update(void);

#endif /* BHDS_PAD_H */
