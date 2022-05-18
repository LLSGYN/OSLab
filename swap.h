#pragma once

void set_replace_algo(int x);
void dbg_residents(int ID);

int resident_init(int ID, int set_size);
int page_reference(int ID, int page);
int demand_replaced(int ID, int page);
int get_frame_num(int ID);
int destroy_residents(int ID);