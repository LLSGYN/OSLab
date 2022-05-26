#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "memdefs.h"



typedef struct PageNode //ҳ����Ϣ�ڵ�
{
    int pageid; //ҳ��id
    int used;   //ʹ��λ 0  ����Ƿ�ʹ�ù���ҳ
} pagenode, * pageptr;

typedef struct PageNodeVector
{
    int pointer;                        //��ǰָ��ָ�������λ��
    int maxSet;                         //��ǰ��פ������ҳ����
    pagenode frameVector[NUM_PAGE]; //��ǰ���̿�ӵ�е����ҳ����
} pagevector, * pagevectorptr;

void CreateCLKSet(int processid, int setsize); //��ĳһ���̵�פ������ʼ��
void ChangeUsedBit(int processid, int pageid); //������ʹ�ù����ض�ҳ��ʹ��λ��Ϊһ
int ReplacePage(int processid, int pageid);//ҳ���滻
int ResetResidentSet(int processid);//���һ�����̵��ڴ�ʹ����Ϣ
void OutputCLKFrame(int processid); //�����ǰפ��������
int CLK_get_frame_num(int processid);//���פ������ҳ����