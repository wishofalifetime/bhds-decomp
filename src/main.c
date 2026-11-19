/*
 * main.c - game entry point and per-frame loop.
 * JP (ABHJ): NitroMain at 0x02000B88.
 * Status: decompiled, NOT yet verified to match (compiler not set up).
 */
#include "types.h"
#include "hw_regs.h"
#include "game.h"

/* Not yet identified. JP addresses. */
extern void func_02000662(void); /* Thumb */
extern void func_0200653c(void);
extern void func_0201354c(void);
extern void func_02013644(void);
extern void func_0206fa80(void);
extern void func_020708b0(void);
extern void func_02073b98(void);
extern void func_02077760(void); /* first scene */
extern void func_020777a0(void);
extern void func_020777dc(void);
extern void func_02077cd8(void);
extern void func_02077fe8(void);
extern void func_0207812c(void);
extern BOOL func_020781e0(void);
extern void func_02078220(void *arg);
extern void func_02078344(void);
extern void func_02093198(void);
extern void func_020b82fc(void);
extern void func_020d8acc(void);
extern void func_020d8d50(void);
extern void func_020fecb4(void);
extern void func_020fed00(void);
extern void func_021007bc(void);
extern void func_02104f78(u32 tag);

#define BHDS_TAG 0x42484453 /* 'BHDS' */

void NitroMain(void)
{
    func_02078344();
    func_02000662();
    func_020fecb4();
    REG_DB_DISPCNT |= REG_DISPCNT_MODE_ON;
    func_020777a0();
    func_02093198();
    func_020708b0();

    gGame.nextScene = func_02077760;

    while (TRUE) {
        if (gGame.unk0A != 0) {
            func_020777dc();
        }
        func_02073b98();
        func_0207812c();
        if (func_020781e0()) {
            func_020fed00();
            REG_DB_DISPCNT &= ~REG_DISPCNT_MODE_ON;
            func_020d8acc();
            func_02104f78(BHDS_TAG);
        }
        func_02013644();
        func_0201354c();
        func_020b82fc();
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
            func_02078220(data_0213ad04);
            func_02078220(data_0213ad24);
            func_0200653c();
        }

        func_021007bc();
        gGame.curScene();
        func_0206fa80();
        func_02077fe8();
    }
}
