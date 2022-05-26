#include "clockpro.h"

pageptrpro CLKFrameMap[MAX_PROCESS][NUM_PAGE];   //����ҳ���0��������Ϣ�洢
pagevectorpro eachCLKSetPerProcess[MAX_PROCESS]; //��¼һ�����̵���Ϣ
int resident_size[MAX_PROCESS];

void CreateCLKSetpro(int processid, int setsize) //��ĳһ���̵�פ������ʼ��
{
    resident_size[processid]=setsize;
    for (int i = 0; i < setsize; i++)
    {
        eachCLKSetPerProcess[processid].frameVector[i].used = 0;
        eachCLKSetPerProcess[processid].frameVector[i].dirty = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = i;                   //��Ӧ���ǽ��̵ĵڼ�ҳ
        CLKFrameMap[processid][i] = &eachCLKSetPerProcess[processid].frameVector[i]; //��ʼ��ҳ���
    }
    eachCLKSetPerProcess[processid].maxSet = setsize;
    eachCLKSetPerProcess[processid].pointer = 0; //��ʼ��λ��ָ��Ϊ0
}

void ChangeUsedBitpro(int processid, int pageid) //������ʹ�ù����ض�ҳ��ʹ��λ��Ϊһ
{
    pageptrpro targetpage = CLKFrameMap[processid][pageid]; //��ҳ����л�ø�ҳ�ĵ�ַ
    if (targetpage == NULL)                              //��ҳ�沢δ��ҳ����
        return;
    targetpage->used = 1; //ʹ��λ��Ϊ1
    targetpage->dirty = page_table[processid][pageid].D;
}

int ReplacePagepro(int processid, int pageid) //ҳ���滻
{
    int pos = 0, circle = eachCLKSetPerProcess[processid].maxSet;
    int curid, replaceid; //��ǰָ���id,Ҫ���滻��id
    for (int times = 0; times < 2; times++)
    {
        while (pos < circle) //��һ��ɨ��
        {
            eachCLKSetPerProcess[processid].pointer %= eachCLKSetPerProcess[processid].maxSet;
            curid = eachCLKSetPerProcess[processid].pointer++; //��ǰָ�����һ��λ��
            if (eachCLKSetPerProcess[processid].frameVector[curid].used == 0 &&
                eachCLKSetPerProcess[processid].frameVector[curid].dirty == 0) //ָ��ָ���ʹ��λ,��λΪ0
            {
                replaceid = eachCLKSetPerProcess[processid].frameVector[curid].pageid;
                eachCLKSetPerProcess[processid].frameVector[curid].used = 1;
                eachCLKSetPerProcess[processid].frameVector[curid].dirty = 0;
                eachCLKSetPerProcess[processid].frameVector[curid].pageid = pageid;
                CLKFrameMap[processid][pageid] = &eachCLKSetPerProcess[processid].frameVector[curid];
                //�����ϵ�ҳ��д��ҳ��
                CLKFrameMap[processid][replaceid] = NULL; //��ҳ���滻��������Ϊ��
                return replaceid;                         //���ɨ�赽��ֱ�ӽ���ѭ�����û��ɹ�
            }
            pos++;
        }
        pos = 0;
        while (pos < circle) //���ûɨ�赽δʹ�ù�����δ�޸Ĺ���ҳ
        {
            eachCLKSetPerProcess[processid].pointer %= eachCLKSetPerProcess[processid].maxSet;
            curid = eachCLKSetPerProcess[processid].pointer++; //��ǰָ�����һ��λ��
            if (eachCLKSetPerProcess[processid].frameVector[curid].used == 0 &&
                eachCLKSetPerProcess[processid].frameVector[curid].dirty == 1) //ָ��ָ���ʹ��λ0,��λΪ1
            {
                replaceid = eachCLKSetPerProcess[processid].frameVector[curid].pageid;
                eachCLKSetPerProcess[processid].frameVector[curid].used = 1;
                eachCLKSetPerProcess[processid].frameVector[curid].dirty = 0;
                eachCLKSetPerProcess[processid].frameVector[curid].pageid = pageid;
                CLKFrameMap[processid][pageid] = &eachCLKSetPerProcess[processid].frameVector[curid];
                //�����ϵ�ҳ��д��ҳ��
                CLKFrameMap[processid][replaceid] = NULL; //��ҳ���滻��������Ϊ��
                return replaceid;                         //���ɨ�赽��ֱ�ӽ���ѭ�����û��ɹ�
            }
            else
                eachCLKSetPerProcess[processid].frameVector[curid].used = 0;
            pos++;
        }
        pos = 0;
    }
}

int ResetResidentSetpro(int processid)
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
        eachCLKSetPerProcess[processid].frameVector[i].dirty = 0;
        eachCLKSetPerProcess[processid].frameVector[i].pageid = -1;
    }
    for (i = 0; i < NUM_PAGE; i++) //���ý��̵�ҳ����ʼ��
    {
        CLKFrameMap[processid][i] = NULL;
    }
    return 1;
}

void OutputCLKFramepro(int processid) //�����ǰפ��������
{
    int i = eachCLKSetPerProcess[processid].pointer; //��ȡ��ǰ����ָ��λ��
    printf("resident set of process <%d>:%d\n", processid, resident_size[processid]);
    if (resident_size[processid] == 0) {
        printf("\n\n");
        return;
    }
    int finalPointer = (i + eachCLKSetPerProcess[processid].maxSet - 1) % eachCLKSetPerProcess[processid].maxSet;
    //����ָ��ǰһλ
    
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