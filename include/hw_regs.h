#ifndef BHDS_HW_REGS_H
#define BHDS_HW_REGS_H

#include "types.h"

/* Sub engine display control (2D engine B) */
#define REG_DB_DISPCNT      (*(vu32 *)0x04001000)
#define REG_DISPCNT_MODE_ON 0x10000

#endif /* BHDS_HW_REGS_H */
