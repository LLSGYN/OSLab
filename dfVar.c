#include "dfVar.h"

int processCNT = 0;							//��¼�������Ľ�������
int usedProcessID[MAX_PROCESS] = { 0 };	    //���̱�ʶ����ʹ�����
PCB allPCB[MAX_PROCESS];					//һ�����̶�Ӧ��һ��PCB
int processInCPU = -1;						//����CPU�����еĽ���ID��
int processInMemory = -1;					//�����ڴ������еĽ���ID��
int processInIO[IO_NUM];				    //����IO�����еĽ���ID��
int CPUMode = 0;		  					//CPU����ģʽ
//int DispatchCPUMode = 0;  					//CPU���ȵĹ���ģʽ  #######û���õ�
int killQueue[MAX_PROCESS];					//��ɾ���̶���
int killHead = 0;                           //�������еĶ�ͷ
int killTail = 0;                           //�������еĶ�β
int toBeKilled[MAX_PROCESS];                //�ж�ĳһID�ŵĽ����Ƿ��ڴ�ɾ���̶�����
int curTime;                                //��ǰʱ��Ƭ
WaitQueue readyQueue;						//CPU��������
WaitQueue waitIOQueue[IO_NUM];				//IO��������,�ж��IO�豸
WaitQueue memoryQueue;						//�ڴ��������

HANDLE usedProcessIDMutex;                  //���̱�ʶ��ʹ�������һ����
HANDLE contCPU;                             //CPU����������ź��� ��ʼֵΪ0
HANDLE contIO[IO_NUM];                      //IO����������ź��� ��ʼֵΪ0
HANDLE contMemory;                          //�ڴ����������ź��� ��ʼֵΪ0
HANDLE proInCPUMutex;                       //������CPU��������Ҫ�Ļ�������ǿ��ɾ��CPU�еĽ���ҲҪ�����������Ŀ����ֹͣCPU�н��̵�ִ��
HANDLE proInIOMutex[IO_NUM];                //������IO��������Ҫ�Ļ�������ǿ��ɾ��IO�еĽ���ҲҪ�����������Ŀ����ֹͣIO�н��̵�ִ��
HANDLE proInMemMutex;                       //�������ڴ���������Ҫ�Ļ�������ǿ��ɾ���ڴ��еĽ���ҲҪ�����������Ŀ����ֹͣ�ڴ��н��̵�ִ��
HANDLE timeLockForCPU;                      //CPU��Ҫ��ȡ��ʱ��Ƭ�ź��� ��ʼֵΪ0
HANDLE timeLockForMemory;					//virMem������Ҫ�ȴ����ź���
HANDLE timeLockForIO[IO_NUM];				//VirIO��Ҫ�ȴ����ź���
HANDLE breakCPU;                            //CPU�����ź��� ��ʼֵΪ1
HANDLE breakIO[IO_NUM];                     //IO�����ź��� ��ʼֵΪ1
HANDLE breakMemory;                         //�ڴ�����ź��� ��ʼֵΪ1
HANDLE killMutex;                           //ǿ��ɱ��������Ҫ��ȡ���ź���
HANDLE killQueueEmpty;                      //������ɱ���Ľ��̶��е�ʣ��ռ䣬��ʼΪMAX_PROCESS
HANDLE killQueueFull;                       //������ɱ���Ľ��̶��еĽ��̸�������ʼΪ0
HANDLE killQueueMutex;                      //������ɱ���Ľ��̶��е���
