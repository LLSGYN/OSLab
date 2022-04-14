#include "proInit.h"

void InitPCB()//初始化PCB
{
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		allPCB[i].IOID = -1;
		allPCB[i].fatherProID = -1;
	}
}

void InitQueue()//初始化就绪队列
{
	for (int i = 0; i < PRIORITY_NUM; i++)
	{
		readyQueue.head[i] = 0;
		readyQueue.tail[i] = 0;

		memoryQueue.head[i] = 0;
		memoryQueue.tail[i] = 0;
	}
	for (int i = 0; i < IO_NUM; i++)
	{
		for (int j = 0; j < PRIORITY_NUM; j++)
		{
			waitIOQueue[i].head[j] = 0;
			waitIOQueue[i].tail[j] = 0;
		}
	}
}

void InitSemaphore()
{
	usedProcessIDMutex = CreateSemaphore(NULL, 1, 1, NULL);
	contCPU = CreateSemaphore(NULL, 0, 1, NULL);
	for (int i = 0; i < IO_NUM; i++)
	{
		contIO[i] = CreateSemaphore(NULL, 0, 1, NULL);
		proInIOMutex[i] = CreateSemaphore(NULL, 1, 1, NULL);
		timeLockForIO[i] = CreateSemaphore(NULL, 0, 1, NULL);
		breakIO[i] = CreateSemaphore(NULL, 1, 1, NULL);
	}
	contMemory = CreateSemaphore(NULL, 0, 1, NULL);
	proInCPUMutex = CreateSemaphore(NULL, 1, 1, NULL);
	proInMemMutex = CreateSemaphore(NULL, 1, 1, NULL);
	timeLockForCPU = CreateSemaphore(NULL, 0, 1, NULL);
	timeLockForMemory = CreateSemaphore(NULL, 0, 1, NULL);
	breakCPU = CreateSemaphore(NULL, 1, 1, NULL);
	breakMemory = CreateSemaphore(NULL, 1, 1, NULL);
	killMutex = CreateSemaphore(NULL, 1, 1, NULL);
	killQueueEmpty = CreateSemaphore(NULL, MAX_PROCESS, MAX_PROCESS, NULL);
	killQueueFull = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);
	killQueueMutex = CreateSemaphore(NULL, 1, 1, NULL);
	readyQueue.totalCnt = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);
	memoryQueue.totalCnt = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);
	readyQueue.totalCnt = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);
	for (int i = 0; i < PRIORITY_NUM; i++)
	{
		readyQueue.queueMutex[i] = CreateSemaphore(NULL, 1, 1, NULL);
		readyQueue.queueEmpty[i] = CreateSemaphore(NULL, MAX_PROCESS, MAX_PROCESS, NULL);
		readyQueue.queueFull[i] = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);

		memoryQueue.queueMutex[i] = CreateSemaphore(NULL, 1, 1, NULL);
		memoryQueue.queueEmpty[i] = CreateSemaphore(NULL, MAX_PROCESS, MAX_PROCESS, NULL);
		memoryQueue.queueFull[i] = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);

		waitIOQueue[i].totalCnt = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);
		for (int j = 0; j < IO_NUM; j++)
		{
			waitIOQueue[j].queueMutex[i] = CreateSemaphore(NULL, 1, 1, NULL);
			waitIOQueue[j].queueEmpty[i] = CreateSemaphore(NULL, MAX_PROCESS, MAX_PROCESS, NULL);
			waitIOQueue[j].queueFull[i] = CreateSemaphore(NULL, 0, MAX_PROCESS, NULL);
		}
	}
}

void Init()
{
	printf("virOS starts...\n");
	printf("input CPU Mode:\n");
	scanf("%d", &CPUMode);//初始化CPU调度方式
	getchar();
	InitPCB();
	InitQueue();
	InitSemaphore();
	InitDisk();
	initRootDir();
}
