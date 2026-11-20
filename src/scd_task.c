/*
 * scd_task.c - room script task pool and runner.
 * JP (ABHJ) 0x020B4E9C - 0x020B5564: the task pool and runner, the
 * per-frame setup, startup script execution and item/model lookups.
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
#include "game.h"

/* Not yet identified. JP addresses. */
extern void func_02056ca4(ScdTask *task);
extern void func_02056ce0(ScdTask *task);
extern BOOL func_020707e8(u32 a);
extern void func_02105ae0(u32 value, void *dst, u32 size);  /* probably MIi_CpuClear32 */

extern void func_01ff87b4(void *a, void *b);  /* ITCM */
extern u16 func_0200e408(u32 a, u32 b, u8 c);
extern void func_020a4d88(void *player);
extern BOOL func_020ad2f4(u32 a, u32 b);

/* Per-item data, 4 bytes per item. JP 0x021228BC */
typedef struct ItemInfo {
    /* 0x00 */ u16 model;
    /* 0x02 */ u8 marker;        /* 0xFF: no marker */
    /* 0x03 */ u8 height;        /* marker height in 32-unit steps */
} ItemInfo;

typedef struct UnkStruct_0214df40 {
    /* 0x00 */ u8 unk00[0x74];
    /* 0x74 */ s16 timer;
    /* 0x76 */ u16 unk76;
} UnkStruct_0214df40;

typedef struct UnkStruct_0214ca40 {
    /* 0x00 */ u8 unk00[0x3C];
    /* 0x3C */ u16 unk3C;
} UnkStruct_0214ca40;

extern ItemInfo data_021228bc[];             /* JP 0x021228BC */
extern u16 **data_02122630[];                /* JP 0x02122630: model ids by stage, room */
extern u32 data_0213b764[];                  /* JP 0x0213B764: flags */
extern u32 data_0213b794[];                  /* JP 0x0213B794: flags */
extern u32 data_0213b7a4[];                  /* JP 0x0213B7A4: flags */
extern u8 data_02144584[];                   /* JP 0x02144584: the player */
extern u8 data_021445c6[];                   /* JP 0x021445C6 */
extern u8 data_0213dca4[];                   /* JP 0x0213DCA4 */
extern UnkStruct_0214df40 data_0214df40;     /* JP 0x0214DF40 */
extern UnkStruct_0214ca40 data_0214ca40;     /* JP 0x0214CA40 */

typedef BOOL (*ScdTaskCond)(void *target);

void Scd_UpdateTimer(void);
void Scd_RunScripts(u8 *script);
extern ScdTaskCond data_02122368[];  /* JP 0x02122368: run condition by task type */

typedef struct ActorFlags {
    /* 0x00 */ u8 unk00[0x90];
    /* 0x90 */ u8 unk90;
} ActorFlags;

/* Model id for an object kind in the current stage and room. */
u16 Scd_GetRoomModel(u8 kind)
{
    return data_02122630[data_0213a71c.stage][data_0213a71c.room][kind & 0x3F];
}

u16 Item_GetModel(u8 item)
{
    return data_021228bc[item].model;
}

/* Special marker model for item 0x42; one higher outside JP (see obj.h). */
#ifdef VERSION_JP
#define ITEM_MARKER_SPECIAL 0xC5
#else
#define ITEM_MARKER_SPECIAL 0xC6
#endif

/* Marker shown over item; 0x32 means none. */
u32 Item_GetMarker(u8 item, u8 *pc)
{
    if (item == 0x42) {
        if (!Flag_Test(data_0213b794, 8)) {
            if (!Flag_Test(data_0213b764, 0x2D) && (pc[1] & 0x7F) == 6) {
                return ITEM_MARKER_SPECIAL;
            }
            if (!Flag_Test(data_0213b764, 0x2E) && (pc[1] & 0x7F) == 7) {
                return ITEM_MARKER_SPECIAL;
            }
        } else {
            if (!Flag_Test(data_0213b764, 0x23) && (pc[1] & 0x7F) == 3) {
                return ITEM_MARKER_SPECIAL;
            }
            if (!Flag_Test(data_0213b764, 0x24) && (pc[1] & 0x7F) == 4) {
                return ITEM_MARKER_SPECIAL;
            }
        }
    }
    return data_021228bc[item].marker == 0xFF ? 0x32 : 0xB;
}

u8 Item_GetMarkerId(u8 item)
{
    return data_021228bc[item].marker;
}

u8 Item_GetHeight(u8 item)
{
    return data_021228bc[item].height;
}

/* Per-frame script setup, then run the startup scripts at script. */
void Scd_FrameStart(u8 *script)
{
    func_01ff87b4(data_021445c6, data_0213dca4);
    gScd.savedPos[0] = data_02143640.unkF74[0];
    gScd.savedPos[1] = data_02143640.unkF74[1];
    gScd.savedPos[2] = data_02143640.unkF74[2];
    if ((gScd.unk74 & 0x101) == 0x101) {
        Scd_UpdateTimer();
        if (data_02144540.unkD8 < 0) {
            data_0213a71c.unk20 &= 3;
            data_0213a71c.unk22 &= 3;
            data_0213b214.unk03 = 4;
        }
    }
    if (Flag_Test(data_0213b7a4, 0x3E)) {
        data_0214ca40.unk3C = data_0214df40.unk76;
    }
    Scd_RunScripts(script);
}

/* Run each length-prefixed script in the list to completion. */
void Scd_RunScripts(u8 *script)
{
    u16 len;

    gScd.stackPtr = gScd.stack;
    gScd.depth = 0;
    for (; gScd.pc = script, (len = *(u16 *)script) != 0; script += len) {
        gScd.pc += 2;
        for (;;) {
            while (Scd_ExecOpcode(*gScd.pc)) {
            }
            if (gScd.depth == 0) {
                break;
            }
            gScd.stackPtr--;
            gScd.pc = *gScd.stackPtr;
            gScd.depth--;
        }
    }
}

/* Count down data_0214df40.timer while flag 0x3E is set. */
void Scd_UpdateTimer(void)
{
    if (Flag_Test(data_0213b7a4, 0x3E)) {
        data_0214df40.timer--;
        if (data_0214df40.timer > 0) {
            data_0214df40.unk76 = func_0200e408(0x7FFF, 0x3C6C, data_0214df40.timer / 22);
        } else {
            u16 t;

            if (!(data_02144640.unk04 & 2)) {
                data_02144640.unk04 |= 2;
                data_02144640.unkAE = 0x96;
            }
            if (data_0213b214.unk01 == 2) {
                func_020a4d88(data_02144584);
            }
            t = -data_0214df40.timer;
            if (t % 40 == 0) {
                if ((u8)(t / 40) > 3) {
                    data_0214df40.timer = -0x78;
                }
                if (func_020ad2f4(0xFF, 2)) {
                    data_02144640.unkE3 |= 1;
                    data_0213b750.flags |= 0x1000000;
                    gScd.unk74 &= ~0x100;
                }
            }
        }
    }
}

void Scd_SetTestValue(u8 value)
{
    gScd.testValue = value;
}

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
