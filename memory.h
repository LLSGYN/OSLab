#pragma once

#include "mmu.h"
#include "swap.h"
#include "memdefs.h"
#include "dfVar.h"
#include "swap.h"

typedef struct {
    int father;
    int n_pages;
} share_t;

pg_t page_table[MAX_PROCESS][NUM_PAGE];

int try_to_write(int ID, int page);

int do_no_page(mem_t* mem, int ID, int page);

extern void mem_init();
extern int memory_alloc(int ID, int page_required);
