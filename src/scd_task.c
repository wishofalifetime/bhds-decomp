/*
 * scd_task.c - room script task pool and runner.
 * JP (ABHJ) 0x020B4E9C - 0x020B513C.
 *
 * Tasks live in gScd.tasks; tasks[0] heads a doubly linked list of the
 * running ones. Each frame Scd_RunTasks runs every task whose type-specific
 * condition allows it, then frees it.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd.h"
#include "scd_data.h"

/* Not yet identified. JP addresses. */
extern void func_02056ca4(ScdTask *task);
extern void func_02056ce0(ScdTask *task);
extern BOOL func_020707e8(u32 a);
extern void func_02105ae0(u32 value, void *dst, u32 size);  /* probably MIi_CpuClear32 */

typedef BOOL (*ScdTaskCond)(void *target);
extern ScdTaskCond data_02122368[];  /* JP 0x02122368: run condition by task type */

typedef struct ActorFlags {
    /* 0x00 */ u8 unk00[0x90];
    /* 0x90 */ u8 unk90;
} ActorFlags;

/* Take a free task from the pool and append it to the running list. */
ScdTask *Scd_AllocTask(void)
{
    u8 i;

    for (i = 1; i < SCD_TASK_COUNT; i++) {
        if (gScd.tasks[i].type == 0) {
            u8 last = gScd.tasks[0].prev;

            gScd.tasks[i].prev = last;
            gScd.tasks[i].next = 0;
            gScd.tasks[last].next = i;
            gScd.tasks[0].prev = i;
            return &gScd.tasks[i];
        }
    }
    return NULL;
}

/* Unlink a task from the running list and clear it. */
void Scd_FreeTask(ScdTask *task)
{
    volatile u32 zero;

    gScd.tasks[task->prev].next = task->next;
    gScd.tasks[task->next].prev = task->prev;
    zero = 0;
    func_02105ae0(zero, task, sizeof(ScdTask));
}

BOOL Scd_CanRunTask(void *target, u8 type)
{
    return data_02122368[type](target);
}

/* Run condition for actor tasks. */
BOOL Scd_CanRunActor(ActorFlags *actor)
{
    if (!func_020707e8(0)) {
        return FALSE;
    }
    if (actor->unk90 == 0) {
        return FALSE;
    }
    return gScd.unk247 == 0;
}

/* Default run condition. */
BOOL Scd_CanRunDefault(void *target)
{
    if (func_020707e8(0) == FALSE) {
        return FALSE;
    }
    if (gScd.unk247 != 0) {
        return FALSE;
    }
    return TRUE;
}

/* Run every task whose condition allows it, then free it. */
void Scd_RunTasks(void)
{
    u8 i = gScd.tasks[0].next;

    while (i != 0) {
        u8 cur = i;

        i = gScd.tasks[cur].next;
        if (gScd.tasks[cur].type != 0 &&
            Scd_CanRunTask(gScd.tasks[cur].target, gScd.tasks[cur].type)) {
            switch (gScd.tasks[cur].type) {
            case 3:
                func_02056ce0(&gScd.tasks[cur]);
                break;
            case 4:
                func_02056ca4(&gScd.tasks[cur]);
                break;
            default:
                Scd_Step(&gScd.tasks[cur]);
                break;
            }
            Scd_FreeTask(&gScd.tasks[cur]);
        }
    }

    if (data_0213b750.flags & 0x1000000) {
        if (!(data_0213b750.flags & 0x40000)) {
            gScd.unk74 |= 0xF;
        } else {
            gScd.unk74 &= ~0xF;
        }
        gScd.seq[0] = 0;
        gScd.seq[1] = 0;
        gScd.seq[2] = 0;
        gScd.seq[3] = 0;
        gScd.unk240 = 0;
        gScd.unk245 = 0;
    }
}
