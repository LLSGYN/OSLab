/*
This is not a part of the operating system,
but rather a virtual MMU.
*/

#include "mmu.h"
#include "memory.h"
#include "swap.h"
#include "memdefs.h"

#include <stdlib.h>

#define TLB_SIZ 4

static mem_t mem[1 << 22]; // ram

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
static lru_node* head = NULL, * tail = NULL;
tlb_ht fb[NUM_PAGE]; // frame bucket

static int cpid;
// static pg_t* ptb = NULL;

void ram_init() // check 1
{
	head = (lru_node*)malloc(sizeof(lru_node));
	head->key = -1;
	head->next = NULL;
}

// when a new process is load, TLB should be flushed
void flush_tlb(int ID) //check 1
{
	for (int i = 0; i < NUM_PAGE; ++i)
	{
		if (fb[i].valid) {
			fb[i].valid = 0;
			if (fb[i].cp)
			{
				free(fb[i].cp);
			}
		}
		fb[i].frame = 0;
	}
	head->key = -1;
	head->next = NULL;
	tail = NULL;
	cpid = ID;
	qlen = 0;
}

lru_node* _push_head(int page) // check 1
{
	lru_node* nPtr = malloc(sizeof(lru_node));
	nPtr->key = page;
	nPtr->prev = head;
	nPtr->next = head->next;
	if (nPtr->next)
		nPtr->next->prev = nPtr;
	head->next = nPtr;
	qlen++;
	return nPtr;
}

void delete_node(int page) // check 1
{
	lru_node* cur = fb[page].cp;
	cur->prev->next = cur->next;
	if (cur->next) {
		cur->next->prev = cur->prev;
	}
	else {
		tail = cur->prev;
	}
	qlen--;
	free(cur);
}

// ok，这个函数就是lru这一个page，并且维护这个TLB_SIZE的集合
void register_ref(int page) // check 1
{
	if (!fb[page].valid) {
		printf("Error! the page accessed is not valid!\n");
		exit(-1);
	}
	lru_node* tmp = _push_head(page);
	delete_node(page);
	fb[page].cp = tmp;
}

void dbg_tlb() // check 2
{
	lru_node* cur = head->next;
	while (cur)
	{
		printf("%d ", cur->key);
		cur = cur->next;
	}
	puts("");
}

// 这个函数用于tlb查询失败时，尝试查询页表或调用缺页中断的
int walk_table(int page) // check 1
{
	int id = cpid;
	while (share_table[id].father != -1)
	{
		id = share_table[id].father;
	}
	if (!page_table[id][page].P) {
		printf("%d\n", share_table[id].n_pages * 1024);
		fprintf(stderr, "trying to access invalid address\n");
		return -1;
	}
	else {
		if (!page_table[id][page].V) {
			do_no_page(mem, id, page);
			// 处理了缺页中断之后，当前页已经有效，返回这个页框号
			// 当前问题：在处理缺页中断时，谁把内容写进了物理内存？
			return page_table[id][page].frame;
			// printf("page is valid, but should be load into memory, call do_no_page()\n");
			// return -2;
		}
		else { // 是出现在页表中的
			page_reference(id, page);
			return page_table[id][page].frame;
		}
	}
}

void tlb_invalidate(int page)
{
	if (fb[page].valid == 0) {
		return;
	}
	delete_node(page);
	fb[page].valid = 0;
	fb[page].cp = NULL;
	printf("page %d is now set invalid\n", page);
}

int _TLB(int page) // check 1
{
	// printf("Trying to find the frame for page %d\n", page);
	puts("----------------------------------------------");
	printf("[TLB] Looking for phys frame of page %d, pid=%d\n", page, cpid);
	if (fb[page].valid) { // a TLB hit
		register_ref(page);
		page_reference(cpid, page);
		return fb[page].frame;
	}
	else { // a TLB miss
		// printf("Oops, a TLB miss!\n");
		printf("[TLB] TLB miss...\n");
		int target = walk_table(page);
		if (target == -1)
			return -1;
		//else if (target == -2) {
		//	target = do_no_page(mem, cpid, page);
		//}
		lru_node* nPtr = _push_head(page);
		fb[page].cp = nPtr, fb[page].frame = target, fb[page].valid = 1;
		if (qlen == 1)
			tail = nPtr;
		if (qlen > TLB_SIZ) { // pop tail!
			lru_node* cp = tail;
			tail = cp->prev;
			tail->next = NULL;
			fb[cp->key].valid = 0;
			fb[cp->key].cp = NULL;
			free(cp);
			qlen--;
		}
		return target;
	}
}

int read_memory(char* buf, int ID, addr_t addr, int len)
{
	if (addr >> 22) {
		fprintf(stderr, "trying to access %d\n, out of maximum valid memory!\n", addr);
		return -1;
	}
	// now the memory we want to read is valid, allocate the buffer
	char* rb = buf;
	addr_t from = addr, to = addr + len, wlen = 0;
	if (to >= share_table[ID].n_pages * PAGE_SIZE) {
		printf("memory out of boundary!\n");
		printf("stop reading memory");
		return -1;
	}
	while (from < to)
	{

		int frame = _TLB(from >> 10), offset = from & 0x3ff; // TODO handle invalid frame query
		/*
		* 解释一下这个min
		* 我们需要一次性读出:
		* 1. 从offset开始的整个frame
		* 2. 或者是到结束地址的长度
		*/
		int readlen = min(0x400 - offset, to - from);
		memcpy(rb + wlen, mem + (frame << 10 | offset), readlen);
		from = ((from >> 10) + 1) << 10; // next page's start address
		wlen += readlen;
	}
	return 0;
}

int write_memory(char* wbuf, int ID, addr_t addr, int len) // check 1
{
	puts("----------------------------------------------");
	if (addr >> 22) {
		fprintf(stderr, "trying to access %d\n, out of maximum valid memory!\n", addr);
		return -1;
	}
	addr_t from = addr, to = addr + len, wlen = 0;
	printf("[write_mem] st=%d, ed=%d\n", from, to);
	if (to >= share_table[ID].n_pages * PAGE_SIZE) {
		printf("memory out of boundary!\n");
		printf("stop writing memory");
		return -1;
	}
	while (from < to)
	{
		try_to_write(cpid, from >> 10);
		int frame = _TLB(from >> 10), offset = from & 0x3ff; // TODO handle invalid frame query
		int writelen = min(0x400 - offset, to - from);
		memcpy(mem + (frame << 10 | offset), wbuf + wlen, writelen);
		from = ((from >> 10) + 1) << 10; // next page's start address
		wlen += writelen;
	}
	return 0;
}

int mmu_read_frame(int frame, char* buf)
{
	if (frame >= 4096) {
		return -1;
	}
	memcpy(buf, mem + (frame << 10), PAGE_SIZE);
	return 0;
}

int mmu_write_frame(int frame, char* buf)
{
	if (frame >= 4096) {
		return -1;
	}
	memcpy(mem + (frame << 10), buf, PAGE_SIZE);
	return 0;
}

/*
int main() {
	ram_init();
	flush_tlb(1);
	printf("NOW CHECKING PROC %d\n", cpid);
	char *buf = NULL;
	buf = malloc(16);
	strcpy(buf, "hello world!");

	for (int i = 0; i < 10; ++i) {
		page_table[1][i].frame = i;
		page_table[1][i].P = page_table[1][i].V = 1;
		page_table[1][i].RW = 1;
	}
	write_memory(buf, 1, 4093, 13);
	dbg_tlb();
	printf("%s\n", mem + 4093);
	return 0;

	// 测试TLB_LRU正确性
	printf("%d\n", _TLB(0));
	printf("%d\n", _TLB(2));
	printf("%d\n", _TLB(3));
	printf("%d\n", _TLB(5));
	printf("%d\n", _TLB(7));
	printf("%d\n", _TLB(0));
	dbg_tlb();
	// register_ref(4096);
	return 0;
}
*/