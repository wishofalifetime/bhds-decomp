/*
 * sndstrm_load.c - stream buffer allocation. Not matching yet: three
 * argument setup instructions for the stream open call are ordered
 * differently.
 * JP (ABHJ) 0x020B70E4 - 0x020B7178.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "snd.h"

/* Not yet identified. JP addresses. */
extern void *func_020b82e4(u32 size);
extern void func_0200d3a4(u32 a);
extern void func_0200d3bc(u32 a, u32 b, u32 c, u32 d, void *buf, u32 size);

/* Allocate the stream buffer and open the stream; plays until target. */
BOOL SndStrm_Load(s16 target)
{
    SndStream *s = &data_0213a548;

    s->data = func_020b82e4(0xFA00);
    if (s->data == NULL) {
        return FALSE;
    }
    s->data = (void *)(((u32)s->data + 0x1F) & ~0x1F);
    func_0200d3bc(0, 2, 0, 0x105D, s->data, 0xFA00);
    func_0200d3a4(0x32);
    s->delay = 0x1E;
    s->target = target;
    s->state = 1;
    return TRUE;
}
