#ifndef BHDS_HEAP_H
#define BHDS_HEAP_H

#include "types.h"

/*
 * A memory block from the main arena with an expanded heap on top of it.
 * The heap calls look like NitroSystem's NNS_Fnd expanded-heap API
 * (not yet confirmed), so the handle and allocator are kept opaque.
 */
typedef struct HeapInfo {
    /* 0x00 */ u32 size;
    /* 0x04 */ void *mem;
    /* 0x08 */ void *heap;
    /* 0x0C */ u8 allocator[0x14];
} HeapInfo; /* size 0x20 */

extern HeapInfo gHeapSmall;  /* JP 0x0213AD04, 0x900 bytes */
extern HeapInfo gHeapMain;   /* JP 0x0213AD24, rest of the main arena */

void Heap_Reset(HeapInfo *info);  /* JP 0x02078220 */

#endif /* BHDS_HEAP_H */
