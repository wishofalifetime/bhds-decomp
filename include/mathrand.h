#ifndef BHDS_MATHRAND_H
#define BHDS_MATHRAND_H

#include "types.h"

/* NitroSDK MATHRandContext32 */
typedef struct MATHRandContext32 {
    u64 x;
    u64 mul;
    u64 add;
} MATHRandContext32;

/* NitroSDK MATH_Rand32 */
static inline u32 MATH_Rand32(MATHRandContext32 *context, u32 max)
{
    context->x = context->mul * context->x + context->add;
    if (max == 0) {
        return (u32)(context->x >> 32);
    } else {
        return (u32)(((context->x >> 32) * max) >> 32);
    }
}

extern MATHRandContext32 data_027e0064;   /* DTCM: game random context */

#endif /* BHDS_MATHRAND_H */
