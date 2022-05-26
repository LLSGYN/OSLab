#include "killProcess.h"
#include "dfVar.h"
#include "fsapi.h"

void DestoryProcess(int ID)//自然销毁的进程ID
{
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		if (allPCB[i].fatherProID == ID)//若为待删进程的子进程
		{
			memory_free(i);
			allPCB[i].pageNum = 0;
		}
	}
	WaitForSingleObject(killQueueMutex, INFINITE);//获取待删进程队列的权限
	allPCB[ID].fatherProID = -1;

	if (toBeKilled[ID] == 0)//若该进程尚未处于待删进程队列中
	{
		WaitForSingleObject(killQueueEmpty, INFINITE);//等待，直到该队列有空余空间
		toBeKilled[ID] = 1;//更改标志位
		killQueue[killTail] = ID;//将该进程放到此队列中
		killTail = (killTail + 1) % MAX_PROCESS;//队尾后移
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----Process %d will be killed!\n", ID);//打印debug信息
		ReleaseSemaphore(writeMutex, 1, NULL);
		ReleaseSemaphore(killQueueFull, 1, NULL);//该队列中进程数加一
	}

	ReleaseSemaphore(killQueueMutex, 1, NULL);//释放该队列的访问权限
}

void KillProcess(int ID)//强制销毁指定的进程
{
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----start kill process %d...\n", ID);
	ReleaseSemaphore(writeMutex, 1, NULL);
	WaitForSingleObject(killMutex, INFINITE);//获得强制销毁进程的权限
	WaitForSingleObject(allPCB[ID].processMutex, INFINITE);
	int nowEID = allPCB[ID].eventID;//待销毁进程正在运行的事件号
	int nowEtype = allPCB[ID].events[nowEID].eventType;//待销毁进程正在运行的事件的类型
	switch (nowEtype)
	{
	case createProcess:
	case occupyCPU:
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----kill process is in CPU queue...\n");//待删进程处于CPU就绪队列
		ReleaseSemaphore(writeMutex, 1, NULL);
		WaitForSingleObject(proInCPUMutex, INFINITE);//阻止CPU的运行
		if (processInCPU == ID)//若待删进程正在运行
		{
			processInCPU = -1;//停止运行
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "----CPU released...\n");
			ReleaseSemaphore(writeMutex, 1, NULL);
		}
		ReleaseSemaphore(killMutex, 1, NULL);//释放强制销毁进程的权限
		KillProFromQueue(&readyQueue, ID);//将该进程从就绪队列删除
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----remove process %d from readyQueue...\n", ID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		ReleaseSemaphore(proInCPUMutex, 1, NULL);//释放CPU互斥信号量
		break;
	}
	case occupyIO:
	{
		int ioID = allPCB[ID].events[nowEID].eventMsg.IDOfIO;
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----kill process is in IO %d queue...\n", ioID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		WaitForSingleObject(proInIOMutex[ioID], INFINITE);
		if (processInIO[ioID] == ID)
		{
			processInIO[ioID] = -1;
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "----IO%d released...\n", ioID);
			ReleaseSemaphore(writeMutex, 1, NULL);
		}
		ReleaseSemaphore(killMutex, 1, NULL);//释放该权限
		KillProFromQueue(&waitIOQueue[ioID], ID);
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----remove process %d from IOQueue%d...\n", ID, ioID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		ReleaseSemaphore(proInIOMutex[ioID], 1, NULL);
		break;
	}
	case proWriteMem:
	case proReadMem:
	case heapAlloc:
	case stackAlloc:
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----kill process is in memory queue...\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		WaitForSingleObject(proInMemMutex, INFINITE);
		ReleaseSemaphore(writeMutex, 1, NULL);
		if (processInMemory == ID)
		{
			processInMemory = -1;
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "----Memory released...\n");
			ReleaseSemaphore(writeMutex, 1, NULL);
		}
		ReleaseSemaphore(killMutex, 1, NULL);//释放该权限
		KillProFromQueue(&memoryQueue, ID);
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----remove process %d from memoryQueue...\n", ID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		ReleaseSemaphore(proInMemMutex, 1, NULL);
		break;
	}
	default:
		break;
	}
	ReleaseSemaphore(allPCB[ID].processMutex, 1, NULL);

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
		for (int i = 0; i < MAX_PROCESS; i++)
		{
			WaitForSingleObject(allPCB[i].processMutex, INFINITE);
			if (allPCB[i].fatherProID == killID)//若为待删进程的子进程
			{
				ReleaseSemaphore(allPCB[i].processMutex, 1, NULL);
				KillProcess(i);//强制删除子进程
			}
			else
				ReleaseSemaphore(allPCB[i].processMutex, 1, NULL);
		}
		WaitForSingleObject(allPCB[killID].processMutex, INFINITE);
		int thisFather = allPCB[killID].fatherProID;//待删进程的父进程ID
		ReleaseSemaphore(allPCB[killID].processMutex, 1, NULL);
		if (thisFather != -1)//若该进程有父进程
		{
			WaitForSingleObject(allPCB[killID].processMutex, INFINITE);
			allPCB[killID].fatherProID = 0;//断绝父子关系
			ReleaseSemaphore(allPCB[killID].processMutex, 1, NULL);
		}
		//释放申请的内存空间
		if (allPCB[killID].pageNum)
			memory_free(killID);
		usedProcessID[killID] = 0;//将此进程标识符ID置为空闲
		processCNT--;//进程总数减一
		toBeKilled[killID] = 0;//当前ID号已经不在待删队列中
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "\nINFO:process %d is killed!\n", killID);//debug信息
		ReleaseSemaphore(writeMutex, 1, NULL);
		printf("\nINFO:process %d is killed!\n", killID);//debug信息
		printf("root@OS-LAB:%s# ", printwd());
	}
}
