#include "clockpro.h"

pageptrpro CLKFrameMap[MAX_PROCESS][NUM_PAGE];   //按照页面从0到最大的信息存储
pagevectorpro eachCLKSetPerProcess[MAX_PROCESS]; //记录一个进程的信息
int resident_size[MAX_PROCESS];

void CreateCLKSetpro(int processid, int setsize) //将某一进程的驻留集初始化
{
    resident_size[processid]=setsize;
    for (int i = 0; i < setsize; i++)
    {
        eachCLKSetPerProcess[processid].frameVector[i].used = 0;
        eachCLKSetPerProcess[processid].frameVector[i].dirty = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = i;                   //对应的是进程的第几页
        CLKFrameMap[processid][i] = &eachCLKSetPerProcess[processid].frameVector[i]; //初始化页框表
    }
    eachCLKSetPerProcess[processid].maxSet = setsize;
    eachCLKSetPerProcess[processid].pointer = 0; //初始的位置指针为0
}

void ChangeUsedBitpro(int processid, int pageid) //将近期使用过的特定页面使用位置为一
{
    pageptrpro targetpage = CLKFrameMap[processid][pageid]; //在页框表中获得该页的地址
    if (targetpage == NULL)                              //该页面并未在页框中
        return;
    targetpage->used = 1; //使用位置为1
    targetpage->dirty = page_table[processid][pageid].D;
}

int ReplacePagepro(int processid, int pageid) //页面替换
{
    int pos = 0, circle = eachCLKSetPerProcess[processid].maxSet;
    int curid, replaceid; //当前指向的id,要被替换的id
    for (int times = 0; times < 2; times++)
    {
        while (pos < circle) //第一轮扫描
        {
            eachCLKSetPerProcess[processid].pointer %= eachCLKSetPerProcess[processid].maxSet;
            curid = eachCLKSetPerProcess[processid].pointer++; //当前指针的下一个位置
            if (eachCLKSetPerProcess[processid].frameVector[curid].used == 0 &&
                eachCLKSetPerProcess[processid].frameVector[curid].dirty == 0) //指针指向的使用位,脏位为0
            {
                replaceid = eachCLKSetPerProcess[processid].frameVector[curid].pageid;
                eachCLKSetPerProcess[processid].frameVector[curid].used = 1;
                eachCLKSetPerProcess[processid].frameVector[curid].dirty = 0;
                eachCLKSetPerProcess[processid].frameVector[curid].pageid = pageid;
                CLKFrameMap[processid][pageid] = &eachCLKSetPerProcess[processid].frameVector[curid];
                //将换上的页面写入页表
                CLKFrameMap[processid][replaceid] = NULL; //从页框替换下来的置为空
                return replaceid;                         //如果扫描到，直接结束循环，置换成功
            }
            pos++;
        }
        pos = 0;
        while (pos < circle) //如果没扫描到未使用过的且未修改过的页
        {
            eachCLKSetPerProcess[processid].pointer %= eachCLKSetPerProcess[processid].maxSet;
            curid = eachCLKSetPerProcess[processid].pointer++; //当前指针的下一个位置
            if (eachCLKSetPerProcess[processid].frameVector[curid].used == 0 &&
                eachCLKSetPerProcess[processid].frameVector[curid].dirty == 1) //指针指向的使用位0,脏位为1
            {
                replaceid = eachCLKSetPerProcess[processid].frameVector[curid].pageid;
                eachCLKSetPerProcess[processid].frameVector[curid].used = 1;
                eachCLKSetPerProcess[processid].frameVector[curid].dirty = 0;
                eachCLKSetPerProcess[processid].frameVector[curid].pageid = pageid;
                CLKFrameMap[processid][pageid] = &eachCLKSetPerProcess[processid].frameVector[curid];
                //将换上的页面写入页表
                CLKFrameMap[processid][replaceid] = NULL; //从页框替换下来的置为空
                return replaceid;                         //如果扫描到，直接结束循环，置换成功
            }
            else
                eachCLKSetPerProcess[processid].frameVector[curid].used = 0;
            pos++;
        }
        pos = 0;
    }
}

int ResetResidentSetpro(int processid)
//清除一个进程的内存使用信息
{
    if (share_table[processid].father != -1) {
        printf("ERROR! cannot destroy the resident set of parent process!");
        return;
    }
    int i = 0;
    //获取当前分配给该页的页框数量
    int curSetSize = eachCLKSetPerProcess[processid].maxSet;
    for (i = 0; i < curSetSize; i++) //初始化所有页框
    {
        eachCLKSetPerProcess[processid].frameVector[i].used = 0;
        eachCLKSetPerProcess[processid].frameVector[i].dirty = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = -1;
    }
    for (i = 0; i < NUM_PAGE; i++) //将该进程的页框表初始化
    {
        CLKFrameMap[processid][i] = NULL;
    }
    return 1;
}

void OutputCLKFramepro(int processid) //输出当前驻留集内容
{
    int i = eachCLKSetPerProcess[processid].pointer; //获取当前工作指针位置
    printf("resident set of process <%d>:%d\n", processid, resident_size[processid]);
    if (resident_size[processid] == 0) {
        printf("\n\n");
        return;
    }
    int finalPointer = (i + eachCLKSetPerProcess[processid].maxSet - 1) % eachCLKSetPerProcess[processid].maxSet;
    //工作指针前一位
    
    for (; i != finalPointer; i = (i + 1) % eachCLKSetPerProcess[processid].maxSet)
    {
        printf("PageID: %d, used: %d, dirty: %d\n",
               eachCLKSetPerProcess[processid].frameVector[i].pageid,
               eachCLKSetPerProcess[processid].frameVector[i].used,
               eachCLKSetPerProcess[processid].frameVector[i].dirty);
    }
    printf("PageID: %d, used: %d, dirty: %d\n",
           eachCLKSetPerProcess[processid].frameVector[i].pageid,
           eachCLKSetPerProcess[processid].frameVector[i].used,
           eachCLKSetPerProcess[processid].frameVector[i].dirty);
}
int CLK_get_frame_num_pro(int processid)
{
	return resident_size[processid];
}
// int main()
// {

//     int processid = 1, size;
//     int d, u, m;
//     char c;
//     printf("please set your setsize");
//     scanf("%d", &size);
//     CreateCLKSet(processid, size);
//     OutputCLKFrame(processid);
//     while (c != 'q')
//     {
//         printf("choose your mode:changeuse/replace/changeall");
//         scanf("%s", &c);
//         if (c == 'u')
//         {
//             printf("use which page(id and use and modify)");
//             scanf("%d%d%d", &d, &u, &m);
//             ChangeUsedBit(processid, d, u, m);
//             OutputCLKFrame(processid);
//         }
//         else if (c == 'r')
//         {
//             printf("replace which page(from 0 to %d)", NUM_PAGE - 1);
//             scanf("%d", &d);
//             ReplacePage(processid, d);
//             OutputCLKFrame(processid);
//         }
//         // else if(c=='a')
//         // {
//         //     for(int i=0;i<NUM_PAGE;i++)
//         //     {
//         //         ChangeUsedBit(processid,i,1);
//         //     }
//         //     OutputCLKFrame(processid);
//         // }
//     }

//     return 0;
// }