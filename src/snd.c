/*
 * snd.c - sound system startup, the sound heap, bank loading and the
 * per-frame update.
 * JP (ABHJ) 0x020B7F70 - 0x020B8324.
 *
 * Snd_Trace is an empty function called with __LINE__ of the original
 * source; the values are kept as literals because this file's line
 * numbers differ.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "snd.h"

/* Not yet identified. JP addresses. */
extern void func_0200d814(void);
extern void func_0200d820(u32 id, u32 kind);
extern void func_0200d8b4(u32 id, void *buf, u32 kind);
extern void func_0200d934(void *a, void *mem);
extern u32 func_0200d958(u32 id, u32 kind);
extern u32 func_0200d9f0(void);
extern void func_0200d9fc(void *a);
extern void func_0200da08(void);
extern u32 func_0200da14(void);
extern void func_0200da20(u32 a, void *mem);
extern void func_020f2464(void *heap);
extern void *func_020f26a4(void *mem, u32 size, u32 a);
extern void func_020f2c40(void *heap, void *mem, u32 align);
extern void func_020f2c60(void *heap, void *p);
extern void *func_020f2c80(void *heap, u32 size);
extern void func_020fc3bc(void);
extern void func_020fc3f0(void);
extern void *func_021041b4(u32 arena, u32 size, u32 align);
extern void func_02105aac(u16 value, void *dst, u32 size);  /* probably MI_CpuFill16 */
extern void Snd_Stop(u8 kind, s32 id);
extern void Snd_StopAll(void);
extern u32 Snd_GetMaxSize(u32 lo, u32 hi);
extern void SndCh_StopAll(void);
extern void SndCh_Update(void);
extern void SndSe_StopAll(void);
extern void SndSe_Update(void);

extern u8 data_02123764[];  /* JP 0x02123764 */

void Snd_Trace(u32 line);
void SndBank_Unload(s32 slot);
void Snd_LoadSeqData(u32 kind);

void Snd_Update(void)
{
    func_0200da08();
    func_020fc3bc();
    SndCh_Update();
    SndSe_Update();
    SndStrm_Update();
}

void *Snd_Alloc(u32 size)
{
    return func_020f2c80(data_0213a548.heap, size);
}

void Snd_Free(void *p)
{
    func_020f2c60(data_0213a548.heap, p);
}

void Snd_Trace(u32 line)
{
}

void Snd_InitSystem(void)
{
    volatile u16 zero = 0;

    func_02105aac(zero, &data_0213a548, sizeof(SndStream));
    data_0213a548.driverMem = func_021041b4(0, func_0200da14(), 0x20);
    func_020fc3f0();
    func_0200da20(2, data_0213a548.driverMem);
    func_0200d9fc(data_02123764);
    func_0200d934(data_02123764, func_021041b4(0, func_0200d9f0(), 0x20));
    data_0213a548.heapMem = func_020f26a4(func_021041b4(0, 0x140000, 0x20), 0x140000, 1);
    func_020f2c40(data_0213a548.heap, data_0213a548.heapMem, 0x20);
    func_020f2464(data_0213a548.heapMem);
    Snd_Trace(461);
    func_0200d814();
    Snd_Trace(465);
    Snd_LoadSeqData(0);
    Snd_LoadSeqData(1);
    Snd_LoadSeqData(2);
    Snd_LoadSeqData(6);
    Snd_LoadSeqData(3);
    Snd_LoadSeqData(4);
    Snd_LoadSeqData(5);
    Snd_Trace(476);
    data_0213a548.banks[0].buf = Snd_Alloc(Snd_GetMaxSize(0xA, 0x12));
    Snd_Trace(484);
    data_0213a548.banks[1].buf = Snd_Alloc(Snd_GetMaxSize(0x13, 0x21));
    Snd_Trace(492);
}

void Snd_LoadSeqData(u32 kind)
{
    void *p = Snd_Alloc(func_0200d958(kind, 8));

    data_0213a548.seqData[kind] = p;
    if (p != NULL) {
        func_0200d8b4(kind, p, 8);
    }
}

void Snd_StopEverything(void)
{
    Snd_Trace(551);
    SndCh_StopAll();
    SndSe_StopAll();
    Snd_StopAll();
    Snd_Trace(567);
}

/* Load bank id into slot; slots 0 and 1 reuse their own buffers. */
BOOL SndBank_Load(s32 slot, u32 id)
{
    SndBankSlot *s;
    void *buf;

    Snd_Trace(648);
    s = &data_0213a548.banks[slot];
    switch (slot) {
    case 0:
    case 1:
        if (s->id != 0 && s->buf != NULL) {
            func_0200d820(s->id, 6);
        }
        buf = s->buf;
        break;
    default:
        if (s->id == id) {
            return TRUE;
        }
        SndBank_Unload(slot);
        buf = Snd_Alloc(func_0200d958(id, 6));
        break;
    }
    Snd_Trace(677);
    s->id = id;
    s->buf = buf;
    if (buf == NULL) {
        return FALSE;
    }
    func_0200d8b4(s->id, s->buf, 6);
    return TRUE;
}

void SndBank_Unload(s32 slot)
{
    SndBankSlot *s = &data_0213a548.banks[slot];

    if (s->id != 0 && s->buf != NULL) {
        func_0200d820(s->id, 6);
        Snd_Free(s->buf);
        s->id = 0;
        s->buf = NULL;
    }
}

/* Stop every sound kind except 2 and unload slots 2-15. */
void SndBank_UnloadAll(void)
{
    u32 kind;

    Snd_Trace(792);
    for (kind = 0; kind <= 6; kind++) {
        if (kind != 2) {
            Snd_Stop(kind, -1);
        }
    }
    for (kind = 2; (s32)kind < 0x10; kind++) {
        SndBank_Unload(kind);
    }
    Snd_Trace(805);
}
