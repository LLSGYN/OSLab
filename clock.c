#include <stdlib.h>
#include <stdio.h>
#include "clock.h"



pageptr CLKFrameMap[MAX_PROCESS][NUM_PAGE]; //����ҳ���0��������Ϣ�洢
pagevector eachCLKSetPerProcess[MAX_PROCESS];
int resident_size[MAX_PROCESS];


void CreateCLKSet(int processid, int setsize) //��ĳһ���̵�פ������ʼ��
{
    resident_size[processid] = setsize;
    for (int i = 0; i < setsize; i++)
    {
        eachCLKSetPerProcess[processid].frameVector[i].used = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = i;                   //��Ӧ���ǽ��̵ĵڼ�ҳ
        CLKFrameMap[processid][i] = &eachCLKSetPerProcess[processid].frameVector[i]; //��ʼ��ҳ���
    }
    eachCLKSetPerProcess[processid].maxSet = setsize;
    eachCLKSetPerProcess[processid].pointer = 0; //��ʼ��λ��ָ��Ϊ0
}

void ChangeUsedBit(int processid, int pageid) //������ʹ�ù����ض�ҳ��ʹ��λ��Ϊһ
{
    pageptr targetpage = CLKFrameMap[processid][pageid]; //��ҳ����л�ø�ҳ�ĵ�ַ
    if (targetpage == NULL)                              //��ҳ�沢δ��ҳ����
        return;
    targetpage->used = 1; //ʹ��λ��Ϊ1
}

int ReplacePage(int processid, int pageid)//ҳ���滻
{
    int curid, replaceid; //��ǰָ���id,Ҫ���滻��id
    while (1)
    {
        eachCLKSetPerProcess[processid].pointer %= eachCLKSetPerProcess[processid].maxSet;
        curid = eachCLKSetPerProcess[processid].pointer++;            //��ǰָ�����һ��λ��
        if (eachCLKSetPerProcess[processid].frameVector[curid].used == 1) //ָ��ָ���ʹ��λΪ1
        {
            eachCLKSetPerProcess[processid].frameVector[curid].used = 0; //��Ϊ���δʹ�ù�
        }
        else
        {
            replaceid = eachCLKSetPerProcess[processid].frameVector[curid].pageid;
            eachCLKSetPerProcess[processid].frameVector[curid].used = 1;
            eachCLKSetPerProcess[processid].frameVector[curid].pageid = pageid;
            CLKFrameMap[processid][pageid] = &eachCLKSetPerProcess[processid].frameVector[curid];
            //�����ϵ�ҳ��д��ҳ��
            CLKFrameMap[processid][replaceid] = NULL; //��ҳ���滻��������Ϊ��
            return replaceid;
        }
    }

}

int ResetResidentSet(int processid)
//���һ�����̵��ڴ�ʹ����Ϣ
{
    if (share_table[processid].father != -1) {
        printf("ERROR! cannot destroy the resident set of parent process!");
        return;
    }
    int i = 0;
    //��ȡ��ǰ�������ҳ��ҳ������
    int curSetSize = eachCLKSetPerProcess[processid].maxSet;
    for (i = 0; i < curSetSize; i++) //��ʼ������ҳ��
    {
        eachCLKSetPerProcess[processid].frameVector[i].used = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = -1;
    }
    for (i = 0; i < NUM_PAGE; i++) //���ý��̵�ҳ����ʼ��
    {
        CLKFrameMap[processid][i] = NULL;
    }
    return 1;
}

void OutputCLKFrame(int processid) //�����ǰפ��������
{
    int i = eachCLKSetPerProcess[processid].pointer; //��ȡ��ǰ����ָ��λ��
    printf("resident set of process <%d>:%d\n", processid, resident_size[processid]);
    if (resident_size[processid] == 0) {
        printf("\n\n");
        return;
    }
    int finalPointer = (i + eachCLKSetPerProcess[processid].maxSet - 1) % eachCLKSetPerProcess[processid].maxSet;
    //else printf("slfjsfjao");
    //����ָ��ǰһλ
    
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

int CLK_get_frame_num(int processid)
{
    return resident_size[processid];
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
//     while(c!='q')
//     {
//         printf("choose your mode:changeuse/replace/changeall");
//         scanf("%s",&c);
//         if(c=='u')
//         {
//             printf("use which page");
//             scanf("%d",&d);
//             ChangeUsedBit(processid,d);
//             OutputCLKFrame(processid);
//         }
//         else if(c=='r')
//         {
//             printf("replace which page(from 0 to %d)",NUM_PAGE-1);
//             scanf("%d",&d);
//             ReplacePage(processid,d);
//             OutputCLKFrame(processid);
//         }
//         else if(c=='a')
//         {
//             for(int i=0;i<NUM_PAGE;i++)
//             {
//                 ChangeUsedBit(processid,i);
//             }
//             OutputCLKFrame(processid);
//         }
//     }

//     return 0;
// }