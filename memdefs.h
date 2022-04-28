#pragma once

#include <assert.h>

#define PAGE_SIZE 4096
#define NUM_PAGE 4096
#define MAX_PROCESS 16
#define MAX_IN_PROCESS 4

typedef struct {
	unsigned int frame : 12;
	unsigned int P : 1; // presence bit: whether a page table entry can be used for translation
	unsigned int V : 1; // valid bit
	//unsigned int D : 1; // dirty bit
} pg_t;

typedef unsigned char mem_t;
