#include "killProcess.h"
#include "dfVar.h"

void DestoryProcess(int ID)//自然销毁的进程ID
{
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		if (allPCB[i].fatherProID == ID)//若为待删进程的子进程
		{
			KillProcess(i);//强制删除子进程
		}
	}
	WaitForSingleObject(killQueueMutex, INFINITE);//获取待删进程队列的权限

	if (toBeKilled[ID] == 0)//若该进程尚未处于待删进程队列中
	{
		WaitForSingleObject(killQueueEmpty, INFINITE);//等待，直到该队列有空余空间
		toBeKilled[ID] = 1;//更改标志位
		killQueue[killTail] = ID;//将该进程放到此队列中
		killTail = (killTail + 1) % MAX_PROCESS;//队尾后移
		printf("----Process %d will be killed!\n", ID);//打印debug信息
		ReleaseSemaphore(killQueueFull, 1, NULL);//该队列中进程数加一
	}

	ReleaseSemaphore(killQueueMutex, 1, NULL);//释放该队列的访问权限
}

void KillProcess(int ID)//强制销毁指定的进程
{
	printf("start kill process %d...\n", ID);
	WaitForSingleObject(killMutex, INFINITE);//获得强制销毁进程的权限
	int nowEID = allPCB[ID].eventID;//待销毁进程正在运行的事件号
	int nowEtype = allPCB[ID].events[nowEID].eventType;//待销毁进程正在运行的事件的类型
	switch (nowEtype)
	{
	case createProcess:
	case compile:
	case occupyCPU:
	{
		printf("kill process is in CPU queue...\n");//待删进程处于CPU就绪队列
		WaitForSingleObject(proInCPUMutex, INFINITE);//阻止CPU的运行
		if (processInCPU == ID)//若待删进程正在运行
		{
			processInCPU = -1;//停止运行
			printf("----CPU released...\n");
		}
		ReleaseSemaphore(killMutex, 1, NULL);//释放强制销毁进程的权限
		KillProFromQueue(&readyQueue, ID);//将该进程从就绪队列删除
		printf("----remove process %d from readyQueue...\n", ID);
		ReleaseSemaphore(proInCPUMutex, 1, NULL);//释放CPU互斥信号量
		break;
	}
	case occupyIO:
	{
		int ioID = allPCB[ID].events[nowEID].eventMsg.IDOfIO;
		printf("kill process is in IO %d queue...\n", ioID);
		WaitForSingleObject(proInIOMutex[ioID], INFINITE);
		if (processInIO[ioID] == ID)
		{
			processInIO[ioID] = -1;
			printf("----IO%d released...\n", ioID);
		}
		ReleaseSemaphore(killMutex, 1, NULL);//释放该权限
		KillProFromQueue(&waitIOQueue[ioID], ID);
		printf("----remove process %d from IOQueue%d...\n", ID, ioID);
		ReleaseSemaphore(proInIOMutex[ioID], 1, NULL);
		break;
	}
	case proWriteMem:
	case proReadMem:
	case heapAlloc:
	case stackAlloc:
	{
		printf("kill process is in memory queue...\n");
		WaitForSingleObject(proInMemMutex, INFINITE);
		if (processInCPU == ID)
		{
			processInCPU = -1;
			printf("----Memory released...\n");
		}
		ReleaseSemaphore(killMutex, 1, NULL);//释放该权限
		KillProFromQueue(&memoryQueue, ID);
		printf("----remove process %d from memoryQueue...\n", ID);
		ReleaseSemaphore(proInMemMutex, 1, NULL);
		break;
	}
	default:
		break;
	}
	
	DestoryProcess(ID);//将该进程加入待删进程队列
}

DWORD WINAPI MyKill(LPVOID lpParam)
{
	while (1)
	{
		WaitForSingleObject(killQueueFull, INFINITE);//等待，直到待删进程队列中有进程
		//printf("lzy123456\n");
		WaitForSingleObject(killQueueMutex, INFINITE);//获取该队列访问权限
		int killID = killQueue[killHead];//获取队列头进程ID
		killHead = (killHead + 1) % MAX_PROCESS;//队列头后移
		ReleaseSemaphore(killQueueEmpty, 1, NULL);//该队列剩余空间加一
		ReleaseSemaphore(killQueueMutex, 1, NULL);//释放该队列访问权限
		int thisFather = allPCB[killID].fatherProID;//待删进程的父进程ID
		if (thisFather != -1)//若该进程有父进程
			allPCB[killID].fatherProID = 0;//断绝父子关系
		//释放申请的内存空间
		//*****************
		//MemoryRelease(ID);
		usedProcessID[killID] = 0;//将此进程标识符ID置为空闲
		processCNT--;//进程总数减一
		toBeKilled[killID] = 0;//当前ID号已经不在待删队列中
		printf("\nINFO:process %d is killed!\n", killID);//debug信息
	}
}
