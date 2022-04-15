#include <stdlib.h>
#include <stdio.h>
#include "clock.h"
#include "memdefs.h"



pageptr CLKFrameMap[MAX_PROCESS][NUM_PAGE]; //按照页面从0到最大的信息存储
pagevector eachCLKSetPerProcess[MAX_PROCESS];//记录一个进程的信息



void CreateCLKSet(int processid, int setsize) //将某一进程的驻留集初始化
{
    for (int i = 0; i < setsize; i++)
    {
        eachCLKSetPerProcess[processid].frameVector[i].used = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = i;                   //对应的是进程的第几页
        CLKFrameMap[processid][i] = &eachCLKSetPerProcess[processid].frameVector[i]; //初始化页框表
    }
    eachCLKSetPerProcess[processid].maxSet = setsize;
    eachCLKSetPerProcess[processid].pointer = 0; //初始的位置指针为0
}

void ChangeUsedBit(int processid, int pageid) //将近期使用过的特定页面使用位置为一
{
    pageptr targetpage = CLKFrameMap[processid][pageid]; //在页框表中获得该页的地址
    if (targetpage == NULL)                              //该页面并未在页框中
        return;
    targetpage->used = 1; //使用位置为1
}

int ReplacePage(int processid, int pageid)//页面替换
{
    int curid, replaceid; //当前指向的id,要被替换的id
    while (1)
    {
        eachCLKSetPerProcess[processid].pointer %= eachCLKSetPerProcess[processid].maxSet;
        curid = eachCLKSetPerProcess[processid].pointer++;            //当前指针的下一个位置
        if (eachCLKSetPerProcess[processid].frameVector[curid].used == 1) //指针指向的使用位为1
        {
            eachCLKSetPerProcess[processid].frameVector[curid].used = 0; //认为最近未使用过
        }
        else
        {
            replaceid = eachCLKSetPerProcess[processid].frameVector[curid].pageid;
            eachCLKSetPerProcess[processid].frameVector[curid].used = 1;
            eachCLKSetPerProcess[processid].frameVector[curid].pageid = pageid;
            CLKFrameMap[processid][pageid] = &eachCLKSetPerProcess[processid].frameVector[curid];
            //将换上的页面写入页表
            CLKFrameMap[processid][replaceid] = NULL; //从页框替换下来的置为空
            return replaceid;
        }
    }
    
}

int ResetResidentSet(int processid)
//清除一个进程的内存使用信息
{
    int i = 0;
    //获取当前分配给该页的页框数量
    int curSetSize = eachCLKSetPerProcess[processid].maxSet;
    for (i = 0; i < curSetSize; i++) //初始化所有页框
    {
        eachCLKSetPerProcess[processid].frameVector[i].used = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = -1;
    }
    for (i = 0; i < NUM_PAGE; i++) //将该进程的页框表初始化
    {
        CLKFrameMap[processid][i] = NULL;
    }
    return 1;
}

void OutputCLKFrame(int processid) //输出当前驻留集内容
{
    int i = eachCLKSetPerProcess[processid].pointer; //获取当前工作指针位置
    int finalPointer = (i + eachCLKSetPerProcess[processid].maxSet - 1) % eachCLKSetPerProcess[processid].maxSet;
    //工作指针前一位
    printf("resident set of process <%d>:\n", processid);

    for (; i != finalPointer; i = (i + 1) % eachCLKSetPerProcess[processid].maxSet)
    {
        printf("PageID: %d, used: %d\n",
               eachCLKSetPerProcess[processid].frameVector[i].pageid,
               eachCLKSetPerProcess[processid].frameVector[i].used);
    }
    printf("PageID: %d, used: %d\n",
           eachCLKSetPerProcess[processid].frameVector[i].pageid,
           eachCLKSetPerProcess[processid].frameVector[i].used);
}

// int main()
// {
//     int processid=1,size;
//     int d;
//     char c;
//     printf("please set your setsize");
//     scanf("%d",&size);
//     CreateCLKSet(processid,size);
//     OutputCLKFrame(processid);
    
//     ChangeUsedBit(processid,d);
//     OutputCLKFrame(processid);
       
//     ReplacePage(processid,d);
//     OutputCLKFrame(processid);
       
//     return 0;
// }
