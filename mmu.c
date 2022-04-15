/*
This is not a part of the operating system,
but rather a virtual MMU.
*/

#include "mmu.h"
#include "memdefs.h"

#define TLB_SIZ 32
#define max(x, y) (x > y ? x : y)
#define min(x, y) (x < y ? x : y)

size_t total_memory = 0;
static mem_t *mem = NULL; // ram

typedef struct _LRUNODE {
	int key;
	struct _LRUNODE* next;
	struct _LRUNODE* prev;
} lru_node;

typedef struct {
	unsigned int frame; // target frame
	int valid; 
	lru_node* cp; // pointer to the lru-cache node
} tlb_ht;

static int qlen = 0;
static lru_node* head = NULL, *tail = NULL;
tlb_ht fb[NUM_PAGE]; // frame bucket

static int cpid;
static pg_t* ptb = NULL;

void ram_init(size_t ram_size)
{
	total_memory = ram_size;
	mem = (mem_t*)malloc(sizeof(mem_t) * ram_size);
	head = (lru_node*)malloc(sizeof(lru_node));
	head->key = -1;
	head->next = NULL;
}

// when a new process is load, TLB should be flushed
void flush_tlb(int ID, pg_t* cur_page_table)
{
	for (int i = 0; i < NUM_PAGE; ++i)
	{
		fb[i].valid = 0;
	}
	ptb = cur_page_table;
	cpid = ID;
}

int walk_table(int page)
{
	if (!ptb[page].P) {
		fprintf(stderr, "trying to access invalid address\n");
		return -1;
	}
	else {
		page_reference(cpid, page);
		if (!ptb[page].V) {
			do_no_page(mem, cpid, page);
		}
	}
}

int _TLB(int page) // return the correct frame!
{
	if (fb[page].valid) { // a TLB hit
		lru_node* cp = fb[page].cp;
		// push head op
		lru_node* temp = (lru_node*)malloc(sizeof(lru_node));
		temp->key = page, temp->next = head->next, temp->prev = head;
		head->next->prev = temp, head->next = temp;
		// delete cp
		if (cp == tail)
			tail = cp->prev;
		cp->prev->next = cp->next;
		if (cp->next)
			cp->next->prev = cp->prev;
		free(cp);
		return fb[page].frame;
	}
	else { // a TLB miss
		int target = walk_table(page);
		if (target == -1)
			return -1;
		qlen++;
		lru_node* temp = (lru_node*)malloc(sizeof(lru_node));
		temp->key = page, temp->next = head->next, temp->prev = head;
		if (head->next)
			head->next->prev = temp;
		head->next = temp;
		fb[page].cp = temp, fb[page].frame = target, fb[page].valid = 1;
		if (qlen == 1)
			tail = temp;

		if (qlen > TLB_SIZ) { // pop tail!
			lru_node* cp = tail;
			tail = cp->prev;
			tail->next = NULL;
			free(cp);
			qlen--;
		}
		return target;
	}
}

int read_memory(char** buf, int ID, addr_t addr, int len)
{
	//pg_t* page_table = GET_PAGETABLE(ID); 
	int base = GET_BASE(ID), limit = GET_LIMIT(ID);

	if (addr >> 24) {
		fprpintf(stderr, "trying to access %d\n, out of maximum valid memory!\n", addr);
		return -1;
	}
	//if (addr + len - 1 > base + limit) { // refer to the textbook, boundary detection? TODO
	//	fprintf(stderr, "memory access out of memory!\n");
	//	return -1;
	//}
	// now the memory we want to read is valid, allocate the buffer
	*buf = (char*)malloc(len);
	char rb = buf;
	addr_t from = addr, to = addr + len, wlen = 0;
	while (from < to)
	{
		int frame = _TLB(from >> 12), offset = from & 0xfff; // TODO handle invalid frame query
		/*
		* 解释一下这个min
		* 我们需要一次性读出:
		* 1. 从offset开始的整个frame
		* 2. 或者是到结束地址的长度
		*/
		int readlen = min(0x1000 - offset, to - from);
		memcpy(rb + wlen, mem + (frame << 12 | offset), readlen);
		from = ((from >> 12) + 1) << 12; // next page's start address
		wlen += readlen;
	}
	return 0;
}

int write_memory(char* wbuf, int ID, addr_t addr, int len) 
{
	pg_t* page_table = GET_PAGETABLE(ID);
	int base = GET_BASE(ID), limit = GET_LIMIT(ID);

	if (addr >> 24) {
		fprpintf(stderr, "trying to access %d\n, out of maximum valid memory!\n", addr);
		return -1;
	}
	if (addr + len - 1 > base + limit) { // refer to the textbook, boundary detection? TODO
		fprintf(stderr, "memory access out of memory!\n");
		return -1;
	}
	addr_t from = addr, to = addr + len, wlen = 0;
	while (from < to)
	{
		int frame = _TLB(from >> 12), offset = from & 0xfff; // TODO handle invalid frame query
		int writelen = min(0x1000 - offset, to - from);
		memcpy(mem + (frame << 12 | offset), wbuf + wlen, writelen);
		from = ((from >> 12) + 1) << 12; // next page's start address
		wlen += writelen;
	}
	return 0;
}
