/*
 * file_list.c - the files screen: building the grid of files that have
 * been found, and picking up a new file.
 * JP (ABHJ) 0x020BE714 - 0x020BE970. Not matching yet (136/151): the
 * code is right but some locals land in different registers.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"

/* A file that can be read. 0xC bytes (see file_page.c). */
typedef struct FileEntry {
    /* 0x00 */ u8 item;
    /* 0x01 */ u8 pages;
    /* 0x02 */ u16 msgBase;
    /* 0x04 */ void *gfx;
    /* 0x08 */ u8 unk08[4];
} FileEntry;

/* Scene work area (data_0213b214.work). */
typedef struct FileWork {
    /* 0x000 */ u8 grid[3][6];
    /* 0x012 */ u8 row;
    /* 0x013 */ s8 sel;
    /* 0x014 */ u8 unk014[0x834 - 0x14];
    /* 0x834 */ u8 unk834;
} FileWork;

#define FILE_WORK ((FileWork *)data_0213b214.work)

/* Not yet identified. JP addresses. */
extern void func_02003a0c(u32 a);
extern void func_0206fb04(u32 a, u32 b, u32 c, u32 d);
extern void func_020c4258(u32 a);
extern void Bgm_SetSlot(u32 i);
extern void Scd_ClearOp18Flag(u8 *pc);
extern void Scd_RemoveItem(UnkEntry_02147d0c *entry);
extern void Snd_Play0(u32 id);

extern const u8 data_02124678[];                /* JP 0x02124678: (file, cell) pairs */
extern const u8 data_02124698[];                /* JP 0x02124698: (file, cell) pairs, second order */
extern const FileEntry data_021246bc[];         /* JP 0x021246BC */
extern u32 data_0213b764[];               /* JP 0x0213B764: flags */
extern u32 data_0213b848[];               /* JP 0x0213B848: files found */

/* Fill the grid with the files found so far; take a file being picked up. */
void File_Build(void)
{
    u8 id;
    const u8 *order;
    u32 file;
    u8 row;
    u8 cell;
    u8 col;
    u8 k;
    u8 i;
    s8 sel;
    u8 list[0x12];
    u8 *pc;

    if (data_0213b750.flags2 & 0x20000) {
        order = data_02124698;
    } else {
        order = data_02124678;
    }
    for (i = 0; i < 0x12; i++) {
        list[i] = 0xFF;
    }
    for (i = 0; i < 0x11; i++) {
        file = ((const u8 (*)[2])order)[i][0];
        cell = ((const u8 (*)[2])order)[i][1];
        if (i == 3) {
            if (list[cell] == 0xFF) {
                list[cell] = Flag_Test(data_0213b848, data_021246bc[3].item - 0x54) ? 3 : -1;
            }
        } else {
            if (!Flag_Test(data_0213b848, data_021246bc[file].item - 0x54)) {
                file = -1;
            }
            list[cell] = file;
        }
    }
    k = 0;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 6; col++) {
            FILE_WORK->grid[row][col] = list[k];
            k++;
        }
    }
    sel = -1;
    if (data_0213b750.flags & 0x100) {
        pc = data_02144640.pickup->script;
        id = pc[8];
        data_0213a71c.unk06 = id;
        if (data_0213b214.unk04[1] != 0 && id == 0x5F && !Flag_Test(data_0213b764, 0x62)) {
            FILE_WORK->unk834 = 1;
            Bgm_SetSlot(0);
        }
        for (i = 0; i < 0x11; i++) {
            if (id == data_021246bc[i].item) {
                sel = i;
            }
        }
        FILE_WORK->sel = sel;
        Flag_Set(data_0213b848, id - 0x54);
        Scd_RemoveItem(data_02144640.pickup);
        Scd_ClearOp18Flag(pc);
        data_0213a71c.unk13 = pc[8];
        func_02003a0c(1);
        Snd_Play0(3);
        func_020c4258(2);
        func_0206fb04(0, 1, 1, 0x100);
    }
}
