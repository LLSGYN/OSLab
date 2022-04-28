#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int addr_t;

// total ram given by configuration
extern size_t total_memory;

// initialize memory
extern void ram_init(size_t ram_size);

int read_memory(char** buf, int ID, addr_t addr, int len);

int write_memory(char* wbuf, int ID, addr_t addr, int len);
