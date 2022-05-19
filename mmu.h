#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_MEM (1 << 24)

typedef unsigned int addr_t;

// initialize memory
extern void ram_init();
extern void dbg_tlb();

void flush_tlb(int ID);

int read_memory(char* buf, int ID, addr_t addr, int len);

int write_memory(char* wbuf, int ID, addr_t addr, int len);
