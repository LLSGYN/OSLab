#include "dfVar.h"
#include "proInit.h"
#include "mytime.h"
#include "CPU.h"
#include "OSShell.h"
#include "proMemory.h"
#include "proIO.h"
#include "killProcess.h"
#include "createProcess.h"

void DestorySemaphore();

int main()
{
	srand(time(NULL));
	Init();//初始化
	logs = fopen("../logs.txt", "w");
	setbuf(logs, NULL);
	// CreateMyProcess("init process1", -1);//初始进程1
	// CreateMyProcess("init process2", -1);//初始进程2
	// CreateMyProcess("init process3", -1);//初始进程3
	HANDLE timeRunThread = CreateThread(NULL, 0, TimeRun, 0, 0, NULL);               //运行时间片模拟线程
	HANDLE dispatchCPUThread = CreateThread(NULL, 0, DispatchCPU, 0, 0, NULL);       //CPU调度线程
	HANDLE virCPUThread = CreateThread(NULL, 0, VirCPU, 0, 0, NULL);                 //CPU的模拟运行
	HANDLE memoryControlThread = CreateThread(NULL, 0, MyMemoryControl, 0, 0, NULL); //模拟读写内存的线程
	HANDLE dispatchMemoryThread = CreateThread(NULL, 0, DispatchMemory, 0, 0, NULL); //内存的F调度线程
	HANDLE killThread = CreateThread(NULL, 0, MyKill, 0, 0, NULL);                   //杀死进程

	int IOID[IO_NUM];

	HANDLE dispatchIOThread[IO_NUM];
	HANDLE virIOThread[IO_NUM];
	for (int i = 0; i < IO_NUM; i++)
	{
		IOID[i] = i;
		dispatchIOThread[i] = CreateThread(NULL, 0, ioScheduling, &IOID[i], 0, NULL); //运行IO的FIFO调度线程
		virIOThread[i] = CreateThread(NULL, 0, virtualIO, &IOID[i], 0, NULL);           //运行IO的FIFO模拟运行
	}

	shell();

	//DestorySemaphore();
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "virOS shutdown...\n");
	ReleaseSemaphore(writeMutex, 1, NULL);
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

		CloseHandle(waitIOQueue[i].totalCnt);

		for (int j = 0; j < IO_NUM; j++)
		{
			CloseHandle(waitIOQueue[j].queueMutex[i]);
			CloseHandle(waitIOQueue[j].queueEmpty[i]);
			CloseHandle(waitIOQueue[j].queueFull[i]);
		}
	}
	CloseHandle(readyQueue.totalCnt);
	CloseHandle(memoryQueue.totalCnt);
	for (int i = 0; i < MAX_PROCESS; i++)
		CloseHandle(allPCB[i].processMutex);
	CloseHandle(writeMutex);
}