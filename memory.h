#pragma once

#include "mmu.h"
#include "swap.h"
#include "memdefs.h"
#include "dfVar.h"
#include "swap.h"
#include "Disk.h"

typedef struct {
    int father;
    int dr_share;
    int master;
    int n_pages;
} share_t;

pg_t page_table[MAX_PROCESS][NUM_PAGE];
share_t share_table[MAX_PROCESS];

extern int try_to_write(int ID, int page);
extern int do_no_page(mem_t* mem, int ID, int page);
extern void mem_init();
extern void command_free();
extern int memory_alloc(int ID, int page_required, int realloc);
extern int memory_free(int ID);

extern int blk_nr;
extern int block_map[block_count];
extern short page_to_block[MAX_PROCESS][NUM_PAGE];