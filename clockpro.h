#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "memdefs.h"



typedef struct PageNodepro //页面信息节点
{
    int pageid; //页面id
    int used;   //使用位 0  标记是否使用过该页
    int dirty;  //脏位
} pagenodepro, *pageptrpro;

typedef struct PageNodeVectorpro
{
    int pointer;                        //当前指针指向数组的位置
    int maxSet;                         //当前此驻留集的页面数
    pagenodepro frameVector[NUM_PAGE]; //当前进程可拥有的最大页面数
} pagevectorpro, *pagevectorptrpro;

void CreateCLKSetpro(int processid, int setsize); //将某一进程的驻留集初始化
void ChangeUsedBitpro(int processid, int pageid); //将近期使用过的特定页面使用位置为一
int ReplacePagepro(int processid, int pageid);//页面替换
int ResetResidentSetpro(int processid);//清除一个进程的内存使用信息
void OutputCLKFramepro(int processid); //输出当前驻留集内容
int CLK_get_frame_num_pro(int processid);//输出驻留集中页面数
