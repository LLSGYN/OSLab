#include"dfVar.h"
#include "proInit.h"
#include "mytime.h"
#include "CPU.h"
#include "proMemory.h"
#include "proIO.h"
#include "killProcess.h"
#include "createProcess.h"

void DestorySemaphore();

int main()
{
	srand(time(NULL));
	Init();//��ʼ��
	CreateMyProcess("init process", -1);//��ʼ����
	HANDLE timeRunThread = CreateThread(NULL, 0, TimeRun, 0, 0, NULL);               //����ʱ��Ƭģ���߳�
	HANDLE dispatchCPUThread = CreateThread(NULL, 0, DispatchCPU, 0, 0, NULL);       //CPU�����߳�
	HANDLE virCPUThread = CreateThread(NULL, 0, VirCPU, 0, 0, NULL);                 //CPU��ģ������
	HANDLE memoryControlThread = CreateThread(NULL, 0, MyMemoryControl, 0, 0, NULL); //ģ���д�ڴ���߳�
	HANDLE dispatchMemoryThread = CreateThread(NULL, 0, DispatchMemory, 0, 0, NULL); //�ڴ��F�����߳�
	HANDLE killThread = CreateThread(NULL, 0, MyKill, 0, 0, NULL);                   //ɱ������
	
	//int IOID[IO_NUM];
	/*
	HANDLE dispatchIOThread[IO_NUM];
	HANDLE virIOThread[IO_NUM];
	for (int i = 0; i < IO_NUM; i++)
	{
		IOID[i] = i;
		dispatchIOThread[i] = CreateThread(NULL, 0, ioScheduling, &IOID[i], 0, NULL); //����IO��FIFO�����߳�
		virIOThread[i] = CreateThread(NULL, 0, virtualIO, &IOID[i], 0, NULL);           //����IO��FIFOģ������
	}*/

	//DestorySemaphore();
	printf("virOS shutdown...\n");
	return 0;
}

void DestorySemaphore()
{
	CloseHandle(usedProcessIDMutex);
	CloseHandle(contCPU);
	CloseHandle(contMemory);
	CloseHandle(proInCPUMutex);
	CloseHandle(proInMemMutex);
	CloseHandle(timeLockForCPU);
	CloseHandle(timeLockForMemory);
	CloseHandle(breakCPU);
	CloseHandle(breakMemory);
	CloseHandle(killMutex);
	CloseHandle(killQueueEmpty);
	CloseHandle(killQueueFull);
	CloseHandle(killQueueMutex);
	for (int i = 0; i < IO_NUM; i++)
	{
		CloseHandle(contIO[i]);
		CloseHandle(proInIOMutex[i]);
		CloseHandle(timeLockForIO[i]);
		CloseHandle(breakIO[i]);
	}
	for (int i = 0; i < PRIORITY_NUM; i++)
	{
		CloseHandle(readyQueue.queueMutex[i]);
		CloseHandle(readyQueue.queueEmpty[i]);
		CloseHandle(readyQueue.queueFull[i]);

		CloseHandle(memoryQueue.queueMutex[i]);
		CloseHandle(memoryQueue.queueEmpty[i]);
		CloseHandle(memoryQueue.queueFull[i]);

		for (int j = 0; j < IO_NUM; j++)
		{
			CloseHandle(waitIOQueue[j].queueMutex[i]);
			CloseHandle(waitIOQueue[j].queueEmpty[i]);
			CloseHandle(waitIOQueue[j].queueFull[i]);
		}
	}
}