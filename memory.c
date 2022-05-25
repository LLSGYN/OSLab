#include "memory.h"

typedef struct FreeListNode {
	unsigned int frame_id;
	struct FreeListNode* next;
}frnode;
frnode* frhead;

share_t share_table[MAX_PROCESS];
int page_used[MAX_PROCESS];

unsigned int num_pages;
// the corresponding byte of each byte is used to mark the number of the page is currently referenced (occupied)
unsigned char mem_map[NUM_PAGE];
unsigned char dirty[NUM_PAGE];

int blk_nr;
int block_map[block_count];
short page_to_block[MAX_PROCESS][NUM_PAGE];

void mem_init() 
{
	// assume total ram is divisible by 4kb
	frhead = (frnode*)malloc(sizeof(frnode));
	frhead->next = NULL;
	for (int i = 0; i < NUM_PAGE; ++i)
	{
		frnode* cur = (frnode*)malloc(sizeof(frnode));
		cur->frame_id = i;
		cur->next = frhead->next;
		frhead->next = cur;
	}
	blk_nr = block_count - 1;
	memset(block_map, -1, sizeof block_map);
	for (int i = 0; i < MAX_PROCESS; ++i)
	{
		share_table[i].master = -1;
		share_table[i].father = -1;
		share_table[i].dr_share = -1;
	}
}

int get_next_free()
{
	if (frhead->next == NULL) { // no free frames
		return -1;
	}
	frnode* cur = frhead->next;
	int falloc = cur->frame_id;
	frhead->next = cur->next;
	free(cur);
	return falloc;
}

//void create_copy(int ID,  int page) 
//{
//	// TODO: what if no frame is available now?
//	int to_alloc = get_next_free();
//	mem_t buf[PAGE_SIZE];
//	read_memory(buf, ID, page << 10, PAGE_SIZE); // copy the original page
//	page_table[ID][page].frame = to_alloc;
//	// 刷新TLB，因为这时我们的页面关系已经改变
//	dbg_tlb();
//	flush_tlb(ID);
//	write_memory(buf, ID, page << 10, PAGE_SIZE); // now page table is modified
//}
//
void do_new_copy(int ID)
{
	int fID = share_table[ID].master;
	memory_alloc(ID, share_table[fID].n_pages, 1);
	// 把之前父进程所有的内存、外存的内容进行copy
	for (int i = 0; i < share_table[ID].n_pages; ++i)
	{
		char buf[PAGE_SIZE];
		if (page_table[fID][i].V) {
			mmu_read_frame(page_table[fID][i].frame, buf);
		}
		else {
			disk_read(buf, fID, i);
		}
		if (page_table[ID][i].V) {
			mmu_write_frame(page_table[ID][i].frame, buf);
		}
		else {
			disk_write(buf, ID, i);
		}
	}
}

int try_to_write(int ID, int page)
{
	if (page_table[ID][page].P == 0) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "error! trying to write to an invalid frame!\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		return -1;
	}
	int fID = share_table[ID].father;
	int dr = share_table[ID].dr_share;

	if (fID != -1) { 
		// 依赖别的页表，给自己复制新的
		// 创建自己的resident set
		do_new_copy(ID);

		// 修改share-link
		if (dr != -1) {
			share_table[dr].father = share_table[ID].father;
		}
		share_table[ID].father = -1;
		share_table[ID].master = -1;
	}
	else if (dr != -1) {
		// 别的正在依赖自己，复制新的
		// 给依赖自己的创建res set
		do_new_copy(dr);

		// 修改share-link
		share_table[dr].father = share_table[ID].father;
		share_table[ID].dr_share = -1;
		share_table[dr].master = -1;
		int tdr = share_table[dr].dr_share;
		while (tdr != -1) {
			share_table[tdr].master = dr;
			tdr = share_table[tdr].dr_share;
		}
	}

	dirty[page_table[ID][page].frame] = 1; // 写脏
	return 0;
}

// PAGE_SIZE = 1024
// store(block_id, char *buf)
// load(block_id, char *buf)

// handling page fault, load the target page into the process
int do_no_page(mem_t* mem, int ID, int page) {
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "[page fault] try find (%d,%d) in disk\n", ID, page);
	ReleaseSemaphore(writeMutex, 1, NULL);
	int replaced_page = demand_replaced(ID, page);
	if (replaced_page < 0) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "ERROR happened in do_no_page()!\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		return -1;
	}
	int replaced_phys = -1;
	replaced_phys = page_table[ID][replaced_page].frame;
	page_table[ID][page].P = 1;
	page_table[ID][page].V = 1;
	page_table[ID][page].frame = replaced_phys;

	if (dirty[replaced_phys]) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "swapping page %d of process %d out\n", replaced_page, ID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		swap_out(ID, replaced_page);
	}
	else {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "page is clean, nothing to do!\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		create_block(ID, replaced_page);
	}
	page_table[ID][replaced_page].V = 0;
	page_table[ID][replaced_page].frame = 0;
	swap_in(ID, page);
	page_reference(ID, page);
}

int try_to_share(int ID, int fID) // pay attention!
{
	int lst = fID;
	while (share_table[lst].dr_share != -1)
	{
		lst = share_table[lst].dr_share;
	}
	share_table[ID].father = lst;
	share_table[ID].master = fID;
	share_table[lst].dr_share = ID;
	share_table[ID].n_pages = share_table[lst].n_pages;
	share_table[ID].dr_share = -1;
	// 从此复制了父进程的page_table
	for (int i = 0; i < NUM_PAGE; ++i) {
		//page_table[fID][i].RW = 1; // to implement copy-on-write, set to read only
		page_table[ID][i] = page_table[fID][i];
		if (page_table[ID][i].V) {
			mem_map[page_table[ID][i].frame]++;
		}
		else if (page_table[ID][i].P) { // 这里可能会修改
			page_to_block[ID][i] = page_to_block[fID][i];
		}
	}
}

// TODO: allocate memory, free memory
int memory_alloc(int ID, int page_required, int realloc) // TODO: fork, share page
{
	if (ID >= MAX_PROCESS) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "Invalid pid %d\n", ID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		return -1;
	}
	if (page_required > NUM_PAGE) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "Unable to allocate %d pages\n", page_required);
		ReleaseSemaphore(writeMutex, 1, NULL);
	}
	
	int faID = allPCB[ID].fatherProID;
	share_table[ID].n_pages = page_required;
	if (faID >= 0 && !realloc) { 
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "process %d created create a subprocess %d!\n", faID, ID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		assert(ID != faID);
		try_to_share(ID, faID);
		// 对于fork的进程，应当共享一个resident set
		// 每当出现一个共享，它们的resident set大小增加一个MAX_SIZE
		//resident_add(ID, min(MAX_RESIDENTS, page_required));
	}
	else { 
		if (!realloc) {
			//puts("triggered!");
			share_table[ID].father = -1;
			share_table[ID].dr_share = -1;
			share_table[ID].master = -1;
		}
		int i = 0;
		while (i < page_required && i < MAX_RESIDENTS) {
			int alloc = get_next_free();
			if (alloc < 0) {
				break;
			}
			page_table[ID][i].frame = alloc;
			// printf("%d\n", alloc);
			mem_map[alloc]++;
			page_table[ID][i].V = 1;
			page_table[ID][i].P = 1;
			//page_table[ID][i].RW = 0;
			i++;
		}
		while (i < page_required)
		{
			page_table[ID][i].V = 0;
			page_table[ID][i].P = 1;
			//page_table[ID][i].RW = 0;
			create_block(ID, i); // 没有写入内存的，创建相应磁盘块
			i++;
		}
		resident_init(ID, min(MAX_RESIDENTS, page_required));
	}
}

void free_page(unsigned long addr)
{
	if (mem_map[addr]--) {
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "[free-page] released frame %d\n", addr);
		ReleaseSemaphore(writeMutex, 1, NULL);
		frnode* cur = (frnode*)malloc(sizeof(frnode));
		cur->frame_id = addr;
		cur->next = frhead->next;
		frhead->next = cur;
		return;
	}
	mem_map[addr] = 0;
	fprintf(stderr, "tyring to free free page!\n");
}

int memory_free(int ID) // release memory when process is terminated
{
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "[free] Trying to free proc %d\n", ID);
	ReleaseSemaphore(writeMutex, 1, NULL);
	int fID = share_table[ID].father;
	int dr = share_table[ID].dr_share;
	dbg_residents(ID);
	if (fID >= 0) {
		// 是共享父进程的页表：
		// 清空页表，删除共享链
		for (int i = 0; i < NUM_PAGE; ++i)
		{
			if (page_table[fID][i].V) {
				mem_map[page_table[fID][i].frame]--;
			}
			page_table[ID][i].frame = 0;
			page_table[ID][i].P = 0;
			page_table[ID][i].V = 0;
		}
		if (dr != -1)
			share_table[dr].father = fID;
		if (share_table[fID].dr_share != -1) {
			share_table[fID].dr_share = dr;
		}
	}
	else {
		// 自己就是父进程
		// 清空自己的页表，内存，外存
		for (int i = 0; i < NUM_PAGE; ++i) {
			if (page_table[ID][i].V == 1) {
				// 在内存里，释放这一个物理帧
				int phys_frame = page_table[ID][i].frame;
				free_page(phys_frame);
				dirty[phys_frame] = 0;
			}
			else if (page_table[ID][i].P) {
				// 有效，删除外存上的
				free_block(ID, i);
			}
			page_table[ID][i].frame = 0;
			page_table[ID][i].P = 0;
			page_table[ID][i].V = 0;
		}
		destroy_residents(ID); 
	}
	share_table[ID].father = -1;
	share_table[ID].dr_share = -1;
	share_table[ID].master = -1;
	share_table[ID].n_pages = 0;
	WaitForSingleObject(writeMutex, INFINITE);
	for (int i = 0; i < 5; ++i) {
		fprintf(logs, "id=%d, fa=%d, dr=%d\n", i, share_table[i].father, share_table[i].dr_share);
	}
	ReleaseSemaphore(writeMutex, 1, NULL);
}

void command_free()
{
	printf("\ntotal used free shared");
}

/*
 * TODO:
 * 子进程 ok
 * Free&free  
 * commit
 * clock对接
 * 脏页是设计页框还是页表？
*/
/*
int main()
{
	InitDisk();
	ram_init();
	mem_init();
	allPCB[0].fatherProID = -1;
	memory_alloc(0, 15, 0);
	flush_tlb(0);
	char buf[3000];
	write_memory(buf, 0, 11259, 2524);
	return 0;
}*/
