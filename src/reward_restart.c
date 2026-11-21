/*
 * reward_restart.c - restarting the bonus item scene's model.
 * JP (ABHJ) 0x020BE0C4 - 0x020BE11C. Not matching yet (18/22): the code is
 * right but the scene base lands in r1 instead of r2 (as in exam_take.c).
 */
#include "types.h"
#include "scd_data.h"

/* Not yet identified. JP addresses. */
extern void func_020928e8(void);
extern void func_020c31c8(u8 item);

void Reward_Restart(void)
{
    if (data_0213b214.unk28 == 2) {
        func_020928e8();
        data_0213b214.unk28 = 0;
        func_020c31c8(data_0213b214.itemId);
        data_0213b214.unk21 = 1;
        data_0213b214.unk22 = 0;
        data_0213b214.unk23 = 0;
    }
}
