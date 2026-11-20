/*
 * main.c - game entry point and per-frame loop.
 * NitroMain is at 0x02000B88 in every region.
 * Status: matching (JP, US, EU).
 */
#include "types.h"
#include "sdk.h"
#include "pad.h"
#include "heap.h"
#include "game.h"

/* Not yet identified. JP addresses. */
extern void func_0200653c(void);
extern void func_0201354c(void);
extern void func_02013644(void);
extern void func_020708b0(void);
extern void func_02077760(void); /* first scene */
extern void func_020777a0(void);
extern void func_020777dc(void);
extern void func_02077cd8(void);
extern void func_02093198(void);
extern void Snd_Update(void);
extern void func_020d8acc(void);
extern void func_020d8d50(void);

#define BHDS_RESET_TAG 0x42484453 /* 'BHDS' */

void NitroMain(void)
{
    Main_Init();
    VBlankIntrWait();
    GX_DispOn();
    GXS_DispOn();
#ifndef VERSION_EU
    func_020777a0();
#endif
    func_02093198();
    func_020708b0();

    gGame.nextScene = func_02077760;

    while (TRUE) {
        if (gGame.unk0A != 0) {
            func_020777dc();
        }
        Pad_Update();
        Main_LidSleep();
        if (Main_CheckSoftReset()) {
            GX_DispOff();
            GXS_DispOff();
            func_020d8acc();
            OS_ResetSystem(BHDS_RESET_TAG);
        }
        func_02013644();
        func_0201354c();
        Snd_Update();
        func_020d8d50();

        gGame.frameCount++;

        if (gGame.nextScene != NULL) {
            gGame.curScene = gGame.nextScene;
            gGame.nextScene = NULL;
            gGame.unkB4 = 0;
            gGame.unkB5 = 0;
            gGame.unkB6 = 0;
            gGame.unkB7 = 0;
            func_02077cd8();
            data_02143640.unk0F = 0;
            data_02143640.unk10 = 0;
            Heap_Reset(&gHeapSmall);
            Heap_Reset(&gHeapMain);
            func_0200653c();
        }

        G3X_Reset();
        gGame.curScene();
        Fade_Update();
        Main_EndFrame();
    }
}
