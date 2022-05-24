#pragma once

#include "Disk.h"
#include "memdefs.h"
#include "memory.h"

void set_replace_algo(int x);
void dbg_residents(int ID);

int resident_init(int ID, int set_size);
int page_reference(int ID, int page);
int demand_replaced(int ID, int page);
int get_frame_num(int ID);
int destroy_residents(int ID);

int disk_read(char *buf, int ID, int page);
int disk_write(char *buf, int ID, int page);
int swap_in(int ID, int page);
int swap_out(int ID, int page);
int create_block(int ID, int page);
int free_block(int ID, int page);

int blk_nr;
int block_map[block_count];