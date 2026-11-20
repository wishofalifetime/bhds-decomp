/*
 * system.c - frame timing, VBlank, lid-close sleep, soft reset and heap reset.
 * JP (ABHJ) 0x02077FE8 - 0x0207825C.
 *
 * mwccarm emits functions in reverse source order, so they are defined
 * here from the last address to the first.
 */
#include "types.h"
#include "sdk.h"
#include "pad.h"
#include "heap.h"
#include "game.h"

/* Not yet identified. JP addresses. */
extern void func_0200642c(void);
extern void func_0200648c(void);
extern void func_02005898(void);
extern void func_020058d0(void);
extern void func_02056d54(void);
extern void func_020761c4(void);
extern void func_02077c9c(void);
extern void func_02077d60(void);
extern void func_02077e40(void);
extern void Snd_StopAll(void);
extern void func_020d8acc(void);
extern void func_020d8be0(void);
extern void func_020f7740(void);
extern void func_02105af4(const void *src, void *dst, u32 size);  /* probably MI_CpuCopy32 */
extern BOOL func_0210a7d0(u32 on);                        /* probably PM_SetLCDPower */
extern void func_0210a8e0(u32 trigger, u32 logic, u16 keys); /* probably PM_GoSleepMode */
extern void func_021023dc(u32 mask, VoidFunc handler);    /* probably OS_SetIrqFunction */
extern void func_020fed48(BOOL on);                       /* probably GX_VBlankIntr */
extern void func_0210224c(u32 mask);                      /* probably OS_EnableIrqMask */
extern void func_020f2698(void *heap);                    /* probably NNS_FndDestroyExpHeap */
extern void *func_020f26a4(void *mem, u32 size, u32 opt); /* probably NNS_FndCreateExpHeapEx */
extern void func_020f2c40(void *allocator, void *heap, s32 align); /* probably NNS_FndInitAllocatorForExpHeap */

extern u8 data_027e01ac[0x98];  /* DTCM; copied into gGame every frame */

#define SOFT_RESET_KEYS (PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START | PAD_BUTTON_SELECT)

/* Throw away everything in a heap by recreating it over the same memory. */
void Heap_Reset(HeapInfo *info)
{
    func_020f2698(info->heap);
    info->heap = func_020f26a4(info->mem, info->size, 1);
    func_020f2c40(info->allocator, info->heap, 4);
}

/* TRUE when L+R+Start+Select is held and one of them was just pressed. */
BOOL Main_CheckSoftReset(void)
{
    if (gGame.unk09 == 0 && (gPad.trg & SOFT_RESET_KEYS) != 0 && gPad.cont == SOFT_RESET_KEYS) {
        return TRUE;
    }
    return FALSE;
}

/* Put the system to sleep while the lid is closed, then restore the screens. */
void Main_LidSleep(void)
{
    if (gPad.lidClosed) {
        Snd_StopAll();
        func_020d8acc();
        Main_EndFrame();
        GX_DispOff();
        GXS_DispOff();
        func_0210a7d0(0);
        func_0210a8e0(0xC, 0, 0);
        Main_EndFrame();
        while (!func_0210a7d0(1)) {
        }
        Main_EndFrame();
        GX_SetPower(POWCNT1_ALL_ENGINES);
        GX_DispOn();
        GXS_DispOn();
    }
}

/* Install Main_VBlankHandler and enable the VBlank interrupt. */
void Main_InitVBlank(void)
{
    func_021023dc(OS_IE_V_BLANK, Main_VBlankHandler);
    func_020fed48(TRUE);
    func_0210224c(OS_IE_V_BLANK);
    OS_EnableIrq();
}

void Main_VBlankHandler(void)
{
    func_02077e40();
    gGame.unk02--;
    gGame.vblankCount++;
    if (gGame.unk02 < 0 && gGame.vblankWait != 0) {
        gGame.vblankWait = 0;
        func_0200648c();
    }
    func_020d8be0();
    func_020058d0();
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/* Finish the frame: swap 3D buffers, wait for VBlank, run end-of-frame updates. */
void Main_EndFrame(void)
{
    func_020f7740();
    G3_SwapBuffers(SWAP_BUFFERS_MANUAL_SORT, SWAP_BUFFERS_W_DEPTH);
    func_02105af4(data_027e01ac, gGame.unk0C, sizeof(gGame.unk0C));
    gGame.vblankWait = 1;
    func_02005898();
    while (gGame.vblankWait != 0) {
    }
    gGame.unk02 = gGame.unk01;
    func_02077c9c();
    func_020761c4();
    func_02056d54();
    func_02077d60();
    func_0200642c();
}
