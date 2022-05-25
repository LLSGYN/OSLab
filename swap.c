#include "lru.h"
#include "swap.h"

#include <stdio.h>

int op = 0;
int cnt = 0;
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

/*
我现在需要实现：
一个内存页-磁盘块的双向映射关系
*/

// 查询下一个空闲磁盘块
int get_next_free_block() {
	int i = (blk_nr + 1) % block_count;
	++cnt;
	// printf("%d\n", cnt);
	while (i != blk_nr) {
		if (block_map[i] == -1) {
			blk_nr = i;
			return i;
		}
		i = (i + 1) % block_count;
	}
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "No swap space availalbe!\n");
	ReleaseSemaphore(writeMutex, 1, NULL);
	return -1;
}

// 将进程ID的page页调入内存
int swap_in(int ID, int page) {
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "[swap-in]: pid=%d, page=%d\n", ID, page);
	ReleaseSemaphore(writeMutex, 1, NULL);
	int blk = page_to_block[ID][page];
	if (blk == -1) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "FATAL! target block does not exist!\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		exit(-1);
	}
	char *buf = (char*)malloc(block_szie * sizeof(char));
	readBlock(blk, buf);
	mmu_write_frame(page_table[ID][page].frame, buf);
	// 释放该block
	block_map[blk] = -1;
	page_to_block[ID][page] = -1;
	free(buf);
	return 0;
}

// 将进程ID的page页写入磁盘
int swap_out(int ID, int page) {
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "[swap-out]: pid=%d, page=%d\n", ID, page);
	ReleaseSemaphore(writeMutex, 1, NULL);
	// 已经调入内存的块没有了对应的磁盘块，分配一个新的
	int blk = create_block(ID, page);
	if (blk == -1) {
		return -1;
	}
	char *buf = (char*)malloc(block_szie * sizeof(char));
	mmu_read_frame(page_table[ID][page].frame, buf);
	writeBlock(blk, buf);
	block_map[blk] = ID << 12 | page;
	page_to_block[ID][page] = blk;
	free(buf);
	return 0;
}

// 读入对应的内容，但是保留在磁盘中
int disk_read(char *buf, int ID, int page) 
{
	int blk = page_to_block[ID][page];
	if (blk == -1) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "FATAL! target block does not exist!\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		exit(-1);
	}
	readBlock(blk, buf);
	return 0;
}

int disk_write(char *buf, int ID, int page) 
{
	int blk = page_to_block[ID][page];
	if (blk == -1) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "FATAL! cannot create new block\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		exit(-1);
	}
	writeBlock(blk, buf);
	return 0;
}

int create_block(int ID, int page) {
	int blk = get_next_free_block();
	if (blk == -1) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "failed to swap out\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		return -1;
	}
	// 占用block，记录映射关系
	block_map[blk] = ID << 12 | page;
	page_to_block[ID][page] = blk;
	return blk;
}

int free_block(int ID, int page) {
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "[free blk] pid=%d, page=%d\n", ID, page);
	ReleaseSemaphore(writeMutex, 1, NULL);
	int blk = page_to_block[ID][page];
	if (blk == -1) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "FATAL! target block does not exist!\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		exit(-1);
	}
	block_map[blk] = -1;
	page_to_block[ID][page] = -1;
}