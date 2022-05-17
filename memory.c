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

void mem_init() 
{
	// assume total ram is divisible by 4kb
	num_pages = TOTAL_MEM / PAGE_SIZE;
	frhead = (frnode*)malloc(sizeof(frnode));
	frhead->next = NULL;
	for (int i = 0; i < num_pages; ++i)
	{
		frnode* cur = (frnode*)malloc(sizeof(frnode));
		cur->frame_id = i;
		cur->next = frhead->next;
		frhead->next = cur;
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

int create_copy(int ID,  int page) 
{
	// TODO: what if no frame is available now?
	int to_alloc = get_next_free();
	mem_t buf[PAGE_SIZE];
	read_memory(buf, ID, page << 12, PAGE_SIZE); // copy the original page
	page_table[ID][page].frame = to_alloc;
	// TODO: to write to the frame we want, a TLB flush is required
	write_memory(buf, ID, page << 12, PAGE_SIZE); // now page table is modified
}

int try_to_write(int ID, int page)
{
	printf("trying to write page %d of prodexx %d\n", page, ID);
	if (page_table[ID][page].P == 0) {
		printf("error! trying to write to an invalid frame!\n");
		return -1;
	}
	if (page_table[ID][page].RW == 1) { // not shared
		page_table[ID][page].RW = 0;
		int cur_frame = page_table[ID][page].frame;
		if (mem_map[cur_frame] != 1) {
			int new_alloc = create_copy(ID, page);
			page_table[ID][page].frame = new_alloc;
			mem_map[cur_frame]--;
		}
	}
	return 0;
}

// PAGE_SIZE = 1024
// store(block_id, char *buf)
// load(block_id, char *buf)

// handling page fault, load the target page into the process
int do_no_page(mem_t* mem, int ID, int page) {
	int replaced_page = demand_replaced(ID, page);
	if (replaced_page < 0)
		return -1; // exception
	int replaced_phys = -1;
	if (page_table[ID][replaced_page].P)
		replaced_phys = page_table[ID][replaced_page].frame;
	if (replaced_phys == -1)
		return -1;

	if (dirty[replaced_phys])
		printf("STORE PAGE TO DISK()\n");
		// STORE_PAGE_FROM_DISK(); // TODO!
	else {
		// page is clean, a disk read is not required
	}
	page_table[ID][page].P = 1;
	page_table[ID][page].V = 1;
	page_table[ID][page].frame = replaced_phys;

	// load content from disk
	printf("LOAD PAGE FROM DISK()\n");
	// mem_t* buf = LOAD_PAGE_FROM_DISK(); // dirty?
	// memcpy(mem + replaced_phys * PAGE_SIZE, buf, PAGE_SIZE);
	// }
	page_reference(ID, page);
}

int try_to_share(int ID, int fID)
{
	share_table[ID].father = fID;
	// share_table[ID].ref = 1;
	// memcpy(page_table[ID], page_table[faID], sizeof(pg_t) * NUM_PAGE); // ����������ҳ��
	for (int i = 0; i < NUM_PAGE; ++i) {
		page_table[ID][i] = page_table[fID][i];
		page_table[ID][i].RW = page_table[fID][i].RW = 1; // to implement copy-on-write, set to read only
		if (page_table[ID][i].V) {
			mem_map[page_table[ID][i].frame]++;
		}
	}
}

// TODO: allocate memory, free memory
int memory_alloc(int ID, int page_required) // TODO: fork, share page
{
	printf("Trying to allocate %d pages to pid %d\n", page_required, ID);
	if (ID >= MAX_PROCESS) {
		printf("Invalid pid %d\n", ID);
		return -1;
	}
	if (page_required > NUM_PAGE) {
		printf("Unable to allocate %d\n", page_required);
	}
	
	int faID = allPCB[ID].fatherProID;
	share_table[ID].n_pages = page_required;
	if (faID >= 0) { // ���������ӽ���
		try_to_share(ID, faID);
	}
	else { // ���´����Ľ��̷����ڴ棬�޸�����
		share_table[ID].father = -1;

		int i = 0;
		while (i < page_required && i < MAX_RESIDENTS) {
			int alloc = get_next_free();
			if (alloc < 0) {
				break;
			}
			page_table[ID][i].frame = alloc;
			mem_map[alloc]++;
			page_table[ID][i].V = 1;
			page_table[ID][i].P = 1;
			page_table[ID][i].RW = 0;
			i++;
		}
		while (i < page_required) // no free frame now
		{
			page_table[ID][i].V = 0;
			page_table[ID][i].P = 1;
			page_table[ID][i].RW = 0;
			i++;
		}
		resident_init(ID, min(MAX_RESIDENTS, page_required));
	}
}

void free_page(unsigned long addr)
{
	if (mem_map[addr]--) {
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
	printf("Trying to free memory of pid %d\n", ID);
	if (share_table[ID].father >= 0) {
		for (int i = 0; i < NUM_PAGE; ++i)
		{
			if (page_table[ID][i].V) {
				mem_map[page_table[ID][i].frame]--;
			}
			page_table[ID][i].frame = 0;
			page_table[ID][i].P = 0;
			page_table[ID][i].V = 0;
		}
	}
	else {
		for (int i = 0; i < NUM_PAGE; ++i) {
			page_table[ID][i].P = 0;
			int phys_frame = page_table[ID][i].frame;
			if (page_table[ID][i].V == 1) {
				free_page(phys_frame);
				page_table[ID][i].V = 0;
				dirty[phys_frame] = 0;
			}
		}
		destroy_residents(ID); 
	}
	share_table[ID].father = -1;
	share_table[ID].n_pages = 0;
}

// int main()
// {
// 	ram_init();
// 	mem_init();
// 	allPCB[1].fatherProID = -1;
// 	memory_alloc(1, 50);
// 	flush_tlb(1);
// 	dbg_residents(1);
// 	// return 0;
// 	char buf[16] = "hello world!";
// 	write_memory(buf, 1, 17000, 13);
// 	// char buf1[16] = "another text";
// 	// write_memory(buf1, 1, 10000, 13);
// 	// dbg_residents(1);
// 	char nb[16];
// 	read_memory(nb, 1, 17000, 13);
// 	puts(nb);
// 	// read_memory(nb, 1, 10000, 13);
// 	// puts(nb);
// 	dbg_residents(1);

// 	return 0;
// }