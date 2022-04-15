#include "swap.h"
#include "memdefs.h"
#include "mmu.h"
#include "memory.h"

/*
 * TODO:
 * memory paging mechanism
 * 
 * physical memory allocation & management
 *	+ smiulated kernel memory occupation?
 *  + fork() calls? share page? copy-on-write?
 *  + page fault handling
 * 
 * 
*/

typedef struct FreeListNode {
	unsigned int frame_id;
	struct FreeListNode* next;
}frnode;
frnode* frhead;

pg_t page_table[MAX_PROCESS][NUM_PAGE];
int page_used[MAX_PROCESS];

unsigned int num_pages;
// the corresponding byte of each byte is used to mark the number of the page is currently referenced (occupied)
unsigned char mem_map[1 << 12];
unsigned char dirty[1 << 12];

void mem_init() 
{
	// assume total ram is divisible by 4kb
	num_pages = total_memory / PAGE_SIZE;
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

// handling page fault, load the target page into the process
int do_no_page(mem_t* mem, int ID, int page) {
	// now suppopse we use static residual size
	// if (page_used[ID] < MAX_IN_PROCESS) { // we can allocate a free frame to the process
	// 	page_used[ID]++;
	// 	int target_phys = GET_FREE_FRAME();
	// 	// update page table
	// 	page_table[ID][page].P = 1;
	// 	page_table[ID][page].V = 1;
	// 	page_table[ID][page].frame = target_phys;

	// 	// load content from disk
	// 	mem_t* buf = LOAD_PAGE_FROM_DISK(); // dirty?
	// 	memcpy(mem + target_phys * PAGE_SIZE, buf, PAGE_SIZE);
		
	// 	// maintain demand paging record
	// 	PAGE_RECORD_MAINTAIN(ID, page); // TODO
	// }
	// else { // swap out a page
	int replaced_page = demand_replaced(ID, page);
	if (replaced_page < 0)
		return; // exception
	int replaced_phys = -1;
	if (page_table[ID][replaced_page].P)
		replaced_phys = page_table[ID][replaced_page].frame;
	if (replaced_phys == -1)
		return;

	if (dirty[replaced_phys])
		STORE_PAGE_FROM_DISK(); // TODO!
	else {
		// page is clean, a disk read is not required
	}
	page_table[ID][page].P = 1;
	page_table[ID][page].V = 1;
	page_table[ID][page].frame = replaced_phys;

	// load content from disk
	mem_t* buf = LOAD_PAGE_FROM_DISK(); // dirty?
	memcpy(mem + replaced_phys * PAGE_SIZE, buf, PAGE_SIZE);
	// }
}

// TODO: allocate memory, free memory
int memory_alloc(int ID, int page_reqired) // TODO: fork, share page
{
	int palloc = 0;
	while (frhead->next && palloc < page_reqired) 
	{
		frnode* cur = frhead->next;
		mem_map[cur->frame_id]++;
		frhead->next = cur->next;
		page_table[ID][palloc].frame = cur->frame_id;
		page_table[ID][palloc].V = 1;
		page_table[ID][palloc].P = 1;
		palloc++;
		free(cur);
	}
	while (palloc < page_reqired) // no free frame now
	{
		page_table[ID][palloc].V = 0;
		page_table[ID][palloc].P = 1;
		palloc++;
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
	// case without sharing
	for (int i = 0; i < NUM_PAGE; ++i) {
		if (!page_table[ID][i].P)
			continue;
		page_table[ID][i].P = 0;
		int phys_frame = page_table[ID][i].frame;
		if (page_table[ID][i].V == 1) {
			free_page(phys_frame);
			page_table[ID][i].V = 0;
			dirty[phys_frame] = 0;
		}
	}
}

int main()
{
	ram_init(1 << 20);

	return 0;
}