/*
 * heap.c - game heap setup.
 * JP (ABHJ): Main_InitHeaps at 0x0207825C.
 * Status: matching (JP, US, EU).
 *
 * Copy propagation must be off: the original keeps &gHeapSmall and
 * &gHeapMain in callee-saved registers instead of reloading them from
 * the literal pool. The pragma does not change any other matched file,
 * so it may have been set project-wide originally.
 */
#pragma opt_propagation off

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
    HeapInfo *small;
    HeapInfo *main;
    u32 lo;
    u32 hi;

    gGame.workBuf = func_021041b4(0, 0x10000, 4);
    func_020779ec();
    func_02006598();

    small = &gHeapSmall;
    small->size = 0x900;
    Heap_Create(small);

    /* The rest of the main arena, rounded inward to 16 bytes. */
    main = &gHeapMain;
    lo = (u32)func_02104478(0);
    hi = (u32)func_0210448c(0);
    lo = (lo + 0xF) & ~0xF;
    hi = hi & ~0xF;
    main->size = hi - lo;
    Heap_Create(main);
}
