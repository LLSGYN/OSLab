#pragma once

int LRU_init(int ID, int r_size);

int LRU_refer(int ID, int page);

int LRU_demand(int ID, int page);

int LRU_get_frame_num(int ID);

void LRU_destroy(int ID);