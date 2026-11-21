#ifndef BHDS_MI_H
#define BHDS_MI_H

#include "types.h"

/* NitroSDK MIi_CpuClear16 (JP 0x02105AAC). */
void MIi_CpuClear16(u16 data, void *dest, u32 size);

/*
 * Clear memory 16 bits at a time. The game's version passes the fill
 * value through a volatile local: the original code stores it to the
 * stack and reads it back before every call.
 */
static inline void MI_CpuClear16(void *dest, u32 size)
{
    vu16 data = 0;

    MIi_CpuClear16(data, dest, size);
}

#endif /* BHDS_MI_H */
