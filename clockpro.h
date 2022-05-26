#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "memdefs.h"



typedef struct PageNodepro //ҳ����Ϣ�ڵ�
{
    int pageid; //ҳ��id
    int used;   //ʹ��λ 0  ����Ƿ�ʹ�ù���ҳ
    int dirty;  //��λ
} pagenodepro, *pageptrpro;

typedef struct PageNodeVectorpro
{
    int pointer;                        //��ǰָ��ָ�������λ��
    int maxSet;                         //��ǰ��פ������ҳ����
    pagenodepro frameVector[NUM_PAGE]; //��ǰ���̿�ӵ�е����ҳ����
} pagevectorpro, *pagevectorptrpro;

void CreateCLKSetpro(int processid, int setsize); //��ĳһ���̵�פ������ʼ��
void ChangeUsedBitpro(int processid, int pageid); //������ʹ�ù����ض�ҳ��ʹ��λ��Ϊһ
int ReplacePagepro(int processid, int pageid);//ҳ���滻
int ResetResidentSetpro(int processid);//���һ�����̵��ڴ�ʹ����Ϣ
void OutputCLKFramepro(int processid); //�����ǰפ��������
int CLK_get_frame_num_pro(int processid);//���פ������ҳ����
