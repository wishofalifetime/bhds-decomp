/*
 * heap.c - game heap setup.
 * JP (ABHJ): Main_InitHeaps at 0x0207825C.
 * Status: NOT matching. The original keeps &gHeapSmall / &gHeapMain in
 * callee-saved registers and re-reads size/mem after storing them; this
 * version gets the same instructions but with addresses reloaded from the
 * literal pool. Probably originally the same source file as system.c.
 */
#include "types.h"
#include "heap.h"
#include "game.h"

/* Not yet identified. JP addresses. */
extern void func_02006598(void);
extern void func_020779ec(void);
extern void *func_021041b4(u32 arena, u32 size, u32 align); /* probably OS_AllocFromArenaLo */
extern void *func_02104478(u32 arena);                    /* probably OS_GetArenaLo */
extern void *func_0210448c(u32 arena);                    /* probably OS_GetArenaHi */
extern void *func_020f26a4(void *mem, u32 size, u32 opt); /* probably NNS_FndCreateExpHeapEx */
extern void func_020f2c40(void *allocator, void *heap, s32 align); /* probably NNS_FndInitAllocatorForExpHeap */

/* Allocate info->size bytes from the main arena and build a heap on them. */
static inline void Heap_Create(HeapInfo *info)
{
    info->mem = func_021041b4(0, info->size, 4);
    info->heap = func_020f26a4(info->mem, info->size, 1);
    func_020f2c40(info->allocator, info->heap, 4);
}

/* Set up the work buffer and the two game heaps from the main arena. */
void Main_InitHeaps(void)
{
    HeapInfo *info;
    void *lo;
    void *hi;

    gGame.workBuf = func_021041b4(0, 0x10000, 4);
    func_020779ec();
    func_02006598();

    info = &gHeapSmall;
    info->size = 0x900;
    Heap_Create(info);

    info = &gHeapMain;
    lo = func_02104478(0);
    hi = func_0210448c(0);
    info->size = ((u32)hi & ~0xF) - (((u32)lo + 0xF) & ~0xF);
    Heap_Create(info);
}
