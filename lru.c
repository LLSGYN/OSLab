#include "lru.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct LRUNODE {
	 int page_id;
	//rp_t page_info;
	struct LRUNODE* next;
	struct LRUNODE* pre;
} lrunode, *lruptr;

lruptr node_mp[MAX_PROCESS][NUM_PAGE];
lruptr head[MAX_PROCESS], tail[MAX_PROCESS];
int resident_size[MAX_PROCESS]; //, npage[MAX_PROCESS];

int push_head(int ID, int page) 
{
	lruptr nptr = malloc(sizeof(lrunode));
	if (nptr == NULL)
		return -1;
	lruptr phead = head[ID];
	nptr->next = phead->next, nptr->pre = phead;
	nptr->page_id = page;
	node_mp[ID][page] = nptr;
	if (phead->next)
		phead->next->pre = nptr;
	phead->next = nptr;
	return 0;
}

lruptr get_page_ptr(int ID, int page)
{
	return node_mp[ID][page];
}

int remove_node(int ID, int page)
{
	lruptr target = get_page_ptr(ID, page);
	if (target == NULL) {
		fprintf(stderr, "trying to delete non-existing page!\n");
		return -1;
	}
	if (target == tail[ID])
		tail[ID] = target->pre;
	else 
		target->next->pre = target->pre;
	target->pre->next = target->next;
	free(target);
	node_mp[ID][page] = NULL;
	return 0;
}

int LRU_init(int ID, int r_size) 
{
	if (r_size <= 0)
		return -1;
	// what if residnet size already > 0 ?
	resident_size[ID] = r_size;
	lruptr nhead = NULL;
	nhead = (lruptr)malloc(sizeof(lrunode));
	if (!nhead)
		return -1;
	head[ID] = nhead, tail[ID] = NULL;
	nhead->next = NULL, nhead->pre = NULL;

	lruptr cur = NULL, prev = nhead;
	for (int i = 0; i < r_size; ++i) {
		cur = (lruptr)malloc(sizeof(lrunode));
		if (!cur) {
			while (prev != NULL) {
				cur = prev;
				prev = prev->pre;
				free(cur);
			}
			return -1;
		}
		prev->next = cur;
		cur->pre = prev;
		cur->next = NULL;
		cur->page_id = i;
		node_mp[ID][i] = cur;
		tail[ID] = cur;
		prev = cur;
	}
	return 1;
}

int LRU_refer(int ID, int page)
{
	if (0 > page || page >= NUM_PAGE) {
		fprintf(stderr, "page number out of range!\n");
		return -2;
	}
	lruptr phead = head[ID];
	if (phead == NULL || phead->next == NULL)
		return -1;
	remove_node(ID, page);
	push_head(ID, page);
	return 0;
}

/*
 * swaps in a page,
 * if no extra space is available,
 * return the swapped out page
*/
int LRU_demand(int ID, int page)
{
	if (0 > page || page >= NUM_PAGE) {
		fprintf(stderr, "page number out of range!\n");
		return -2;
	}
	lruptr phead = head[ID];
	if (phead == NULL || resident_size[ID] == 0)
		return -2;
	push_head(ID, page);

	int target_page = tail[ID]->page_id;
	remove_node(ID, target_page);
	//lruptr t = tail[ID];
	//tail[ID] = t->pre;
	//tail[ID]->next = NULL;
	//free(t);
	return target_page;
}

void LRU_destroy(int ID)
{
	if (allPCB[ID].fatherProID != -1) {
		printf("ERROR! cannot destroy the resident set of parent process!");
		return;
	}
	lruptr phead = head[ID], cur = phead->next;
	while (cur) {
		lruptr nxt = cur->next;
		node_mp[ID][cur->page_id] = NULL;
		free(cur);
		cur = nxt;
	}
	tail[ID] = head[ID] = NULL;
	free(phead);
	resident_size[ID] = 0;
}

void dbg_LRU(int ID)
{
	while (share_table[ID].father != -1)
	{
		ID = share_table[ID].father;
	}
	printf("------DEBUG PROCESS %d------\n", ID);
	printf("resident set info: %d\n", resident_size[ID]);
	if (resident_size[ID] == 0) {
		printf("\n\n");
		return;
	}
	lruptr cur = head[ID]->next;
	// printf("head=%d, tail=%d\n", head[ID]->next->page_info.page, tail[ID]->page_id);
	printf("context:\n");
	while (cur) {
		printf("<%d> ", cur->page_id);
		cur = cur->next;
	}
	printf("\n--------------------------\n\n");
}

// return the number of frames in memory
int LRU_get_frame_num(int ID)
{
	return resident_size[ID];
}

//void LRU_add(int ID, int set_size)
//{
//	int faID = ID;
//	while (allPCB[faID].fatherProID != -1)
//	{
//		faID = allPCB[faID].fatherProID;
//	}
//	assert(tail[faID] != NULL);
//	for (int i = 0; i < set_size; ++i)
//	{
//		lruptr nPtr = malloc(sizeof(lrunode));
//		if (nPtr == NULL) {
//			printf("fail to allocate memory!\n");
//			return;
//		}
//		tail[faID]->next = nPtr;
//		nPtr->pre = tail[faID];
//		nPtr->page_info = (rp_t){ -1, -1 };
//		nPtr->next = NULL;
//		resident_size[faID]++;
//		tail[faID] = nPtr;
//	}
//	resident_size[ID] = resident_size[faID];
//}

/*int main() {
	// testing LRU
	LRU_init(3, 4);
	dbg_LRU(3);
	LRU_refer(3, 0);
	dbg_LRU(3);
	LRU_refer(3, 3);
	dbg_LRU(3);
	LRU_refer(3, 3);
	dbg_LRU(3);
	printf("%d\n", LRU_demand(3, 1234));
	dbg_LRU(3);
	printf("%d\n", LRU_demand(3, 5678));
	dbg_LRU(3);
	return 0;
}*/