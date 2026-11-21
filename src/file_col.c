/*
 * file_col.c - the files screen: the next and previous file in a row.
 * JP (ABHJ) 0x020BF1F8 - 0x020BF2B0. Not matching yet (19/23 each): the
 * loop counter and the row index land in swapped registers.
 *
 * Functions are defined in reverse address order (mwccarm emits functions
 * in reverse source order).
 */
#include "types.h"
#include "scd_data.h"

/* Scene work area (data_0213b214.work, see file_read.c). */
typedef struct FileWork {
    /* 0x000 */ s8 grid[3][6];
    /* 0x012 */ u8 row;
} FileWork;

#define FILE_WORK ((FileWork *)data_0213b214.work)

/* The previous file in the row, or col if there is none. */
u8 File_PrevCol(u8 col)
{
    s8 i;
    s8 *row = FILE_WORK->grid[FILE_WORK->row];

    for (i = (s8)col - 1; i >= 0; i--) {
        if (row[i] != -1) {
            return i;
        }
    }
    return col;
}

/* The next file in the row, or col if there is none. */
u8 File_NextCol(u8 col)
{
    s8 i;
    s8 *row = FILE_WORK->grid[FILE_WORK->row];

    for (i = (s8)col + 1; i < 6; i++) {
        if (row[i] != -1) {
            return i;
        }
    }
    return col;
}
