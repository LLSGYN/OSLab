#pragma once
#include "ProStruct.h"
extern int processCNT; 						//��¼�������Ľ�������
extern int usedProcessID[MAX_PROCESS];	    //���̱�ʶ����ʹ�����
extern PCB allPCB[MAX_PROCESS];					//һ�����̶�Ӧ��һ��PCB
extern int processInCPU;						//����CPU�����еĽ���ID��
extern int processInMemory;					//�����ڴ������еĽ���ID��
extern int processInIO[IO_NUM];				    //����IO�����еĽ���ID��
extern int CPUMode;		  					//CPU����ģʽ
//int DispatchCPUMode = 0;  					//CPU���ȵĹ���ģʽ  #######û���õ�
extern int killQueue[MAX_PROCESS];					//��ɾ���̶���
extern int killHead;                           //�������еĶ�ͷ
extern int killTail;                           //�������еĶ�β
extern int toBeKilled[MAX_PROCESS];                //�ж�ĳһID�ŵĽ����Ƿ��ڴ�ɾ���̶�����
extern int curTime;                                //��ǰʱ��Ƭ
extern WaitQueue readyQueue;						//CPU��������
extern WaitQueue waitIOQueue[IO_NUM];				//IO��������,�ж��IO�豸
extern WaitQueue memoryQueue;						//�ڴ��������

extern HANDLE usedProcessIDMutex;                  //���̱�ʶ��ʹ�������һ����
extern HANDLE contCPU;                             //CPU����������ź��� ��ʼֵΪ0
extern HANDLE contIO[IO_NUM];                      //IO����������ź��� ��ʼֵΪ0
extern HANDLE contMemory;                          //�ڴ����������ź��� ��ʼֵΪ0
extern HANDLE proInCPUMutex;                       //������CPU��������Ҫ�Ļ�������ǿ��ɾ��CPU�еĽ���ҲҪ�����������Ŀ����ֹͣCPU�н��̵�ִ��
extern HANDLE proInIOMutex[IO_NUM];                //������IO��������Ҫ�Ļ�������ǿ��ɾ��IO�еĽ���ҲҪ�����������Ŀ����ֹͣIO�н��̵�ִ��
extern HANDLE proInMemMutex;                       //�������ڴ���������Ҫ�Ļ�������ǿ��ɾ���ڴ��еĽ���ҲҪ�����������Ŀ����ֹͣ�ڴ��н��̵�ִ��
extern HANDLE timeLockForCPU;                      //CPU��Ҫ��ȡ��ʱ��Ƭ�ź��� ��ʼֵΪ0
extern HANDLE timeLockForMemory;					//virMem������Ҫ�ȴ����ź���
extern HANDLE timeLockForIO[IO_NUM];				//VirIO��Ҫ�ȴ����ź���
extern HANDLE breakCPU;                            //CPU�����ź��� ��ʼֵΪ1
extern HANDLE breakIO[IO_NUM];                     //IO�����ź��� ��ʼֵΪ1
extern HANDLE breakMemory;                         //�ڴ�����ź��� ��ʼֵΪ1
extern HANDLE killMutex;                           //ǿ��ɱ��������Ҫ��ȡ���ź���
extern HANDLE killQueueEmpty;                      //������ɱ���Ľ��̶��е�ʣ��ռ䣬��ʼΪMAX_PROCESS
extern HANDLE killQueueFull;                       //������ɱ���Ľ��̶��еĽ��̸�������ʼΪ0
extern HANDLE killQueueMutex;                      //������ɱ���Ľ��̶��е���
extern HANDLE printMutex;						   //���������ֹ�������
extern HANDLE writeMutex;
extern FILE* logs;