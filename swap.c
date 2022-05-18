#include "memdefs.h"
#include "lru.h"

#include <stdio.h>

int op = 0;
/*
 * demand paging algo
 * 0: LRU
 * 1: CLOCK
*/

void set_replace_algo(int x)
{
	if (x == 0 || x == 1)
		op = x;
	else
		fprintf(stderr, "invalid option!!\n");
}

int resident_init(int ID, int set_size)
{
	switch (op)
	{
	case 0:
		return LRU_init(ID, set_size);
		break;
	default:
		// return CLOCK_init(ID, set_size);
		break;
	}
}

int page_reference(int ID, int page)
{
	switch (op)
	{
	case 0:
		return LRU_refer(ID, page);
		break;
	default:
		// return CLOCK_refer(ID, page);
		break;
	}
}

int demand_replaced(int ID, int page)
{
	switch (op)
	{
	case 0:
		return LRU_demand(ID, page);
		break;
	default:
		// return CLOCK_demand(ID, page);
		break;
	}
}

int get_frame_num(int ID)
{
	switch (op)
	{
	case 0:
		return LRU_get_frame_num(ID);
		break;
	
	default:
		// return CLOCK_get_frame_num(ID);
		break;
	}
}

int destroy_residents(int ID)
{
	switch (op)
	{
	case 0:
		LRU_destroy(ID);
		break;
	
	default:
		// return CLOCK_get_frame_num(ID);
		break;
	}
}

void dbg_residents(int ID)
{
	switch (op)
	{
	case 0:
		dbg_LRU(ID);
		break;
	
	default:
		break;
	}
}
// int add_frame(int ID, int page)
// {
// 	switch (op)
// 	{
// 	case 0:
// 		return LRU_add(ID, page);
// 		break;
// 	default:
// 		return CLOCK_add(ID, page);
// 		break;
// 	}
// }
