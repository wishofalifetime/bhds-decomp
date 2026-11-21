/*
 * file_page.c - the files (documents) screen: showing a file's pages.
 * JP (ABHJ) 0x020BE970 - 0x020BEB18.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "hw_regs.h"
#include "scd_data.h"

/* A file that can be read. 0xC bytes. */
typedef struct FileEntry {
    /* 0x00 */ u8 item;          /* item id; its flag is item - 0x54 */
    /* 0x01 */ u8 pages;
    /* 0x02 */ u16 msgBase;      /* message of the first page */
    /* 0x04 */ void *gfx;
    /* 0x08 */ u8 unk08[4];
} FileEntry;

/* Scene work area (data_0213b214.work). */
typedef struct FileWork {
    /* 0x00 */ u8 grid[3][6];    /* file index per cell, 0xFF: empty */
    /* 0x12 */ u8 row;
    /* 0x13 */ u8 sel;           /* file being read */
    /* 0x14 */ u8 page;
} FileWork;

typedef struct UnkDtcm_027e01ac {
    /* 0x00 */ u8 unk00[0x10];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 unk12[0x20 - 0x12];
    /* 0x20 */ s16 unk20;
} UnkDtcm_027e01ac;

#define FILE_WORK ((FileWork *)data_0213b214.work)

/* Not yet identified. JP addresses. */
extern void func_020035e4(u32 a, void *b, u32 c, u32 d);
extern void func_020038b8(void *a);
extern void func_02003a0c(u32 a);
extern void func_02003b00(u32 a);
extern void *func_0206fe94(void *a, u32 b, u16 msg, u32 c);
extern void func_02070244(void *a);

extern u8 data_02124540[];                /* JP 0x02124540 */
extern u8 data_02124630[];                /* JP 0x02124630 */
extern FileEntry data_021246bc[];         /* JP 0x021246BC */
extern u8 data_0213a768[];                /* JP 0x0213A768 */
extern UnkDtcm_027e01ac data_027e01ac;    /* DTCM */

void File_LoadGfx(void)
{
    u8 sel;

    func_020038b8(data_02124630);
    sel = FILE_WORK->sel;
    func_020038b8(data_021246bc[sel].gfx);
    if (sel == 11) {
        func_02003a0c(1);
        func_020038b8(data_02124540);
    }
}

u8 File_PageCount(void);

/* Show the current page; file 11 also shows a picture on its last page. */
void File_ShowPage(void)
{
    u8 sel = FILE_WORK->sel;
    u8 page = FILE_WORK->page;
    void *msg = func_0206fe94(data_0213a768, 1, page + data_021246bc[sel].msgBase, 1);

    func_020035e4(2, msg, 0, 0x3F00);
    func_02070244(msg);
    if (sel == 11) {
        if (page >= File_PageCount() - 1) {
            if (!(((REG_DISPCNT & 0x1F00) >> 8) & 1)) {
                func_02003b00(2);
                data_027e01ac.unk10 = data_027e01ac.unk20;
            }
        } else {
            REG_DISPCNT = (REG_DISPCNT & ~0x1F00) | 0x1C00;
        }
    }
}

u8 File_PageCount(void)
{
    return data_021246bc[FILE_WORK->sel].pages;
}

/* Count down the blink timer; off for one frame in every three. */
BOOL File_Blink(void)
{
    BOOL ret = TRUE;

    if (data_0213b214.blink != 0) {
        if (data_0213b214.blink % 3 == 0) {
            ret = FALSE;
        }
        data_0213b214.blink--;
    } else {
        ret = FALSE;
    }
    return ret;
}
