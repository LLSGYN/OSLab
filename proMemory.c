#include "proMemory.h"

DWORD WINAPI MyMemoryControl(LPVOID lpParamter)
{
#ifdef DEBUG
	printf("memoryControl start...\n");
#endif
	WaitForSingleObject(contMemory, INFINITE);//等待内存调度完成后，继续Memory运行
	while (1)
	{
		WaitForSingleObject(timeLockForMemory, INFINITE);
		WaitForSingleObject(proInCPUMutex, INFINITE);//获取内存控制权限，在强制杀死进程的时候可能会争夺该信号量
		WaitForSingleObject(allPCB[processInMemory].processMutex, INFINITE);

		if (processInMemory != -1)//有进程正在运行
		{
			int curEID = allPCB[processInMemory].eventID;//当前事件ID
			allPCB[processInMemory].eventTime++;//事件运行时间加一
			if (allPCB[processInMemory].eventTime == allPCB[processInMemory].events[curEID].time)//当前事件即将执行完成
			{
				if (allPCB[processInMemory].events[curEID].eventType == proReadMem)//读内存
				{
					int start_page = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.startPageID;//逻辑页号
					int offset = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.offset;//页内偏移量
					int len = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.len;//读内存的长度
					//########################
					//调用读内存接口函数
				}
				else if (allPCB[processInMemory].events[curEID].eventType == proWriteMem)//写内存
				{
					int start_page = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.startPageID;//逻辑页号
					int offset = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.offset;//页内偏移量
					int len = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.len;//读内存的长度
					//########################
					//调用写内存函数
				}
				else if (allPCB[processInMemory].events[curEID].eventType == heapAlloc)//申请堆
				{
					allPCB[processInMemory].heapUsed += allPCB[processInMemory].events[curEID].needRAM;
#ifdef DEBUG
					printf("----Process %d heap alloc is finished...\n", processInMemory);
#endif
				}
				else if (allPCB[processInMemory].events[curEID].eventType == stackAlloc)//申请栈
				{
					allPCB[processInMemory].stackUsed += allPCB[processInMemory].events[curEID].needRAM;
#ifdef DEBUG
					printf("----Process %d stack alloc is finished...\n", processInMemory);
#endif
				}
				ReleaseSemaphore(proInMemMutex, 1, NULL);

				WaitForSingleObject(killMutex, INFINITE);
				UpdateEvent(processInMemory);
				ReleaseSemaphore(killMutex, 1, NULL);
				ReleaseSemaphore(allPCB[processInMemory].processMutex, 1, NULL);

				ReleaseSemaphore(breakMemory, 1, NULL);
				WaitForSingleObject(contMemory, INFINITE);
			}
			else
			{
				ReleaseSemaphore(proInMemMutex, 1, NULL);
				ReleaseSemaphore(allPCB[processInMemory].processMutex, 1, NULL);
			}
		}
		else
		{
			ReleaseSemaphore(proInMemMutex, 1, NULL);
			ReleaseSemaphore(allPCB[processInMemory].processMutex, 1, NULL);
			ReleaseSemaphore(breakMemory, 1, NULL);
			WaitForSingleObject(contMemory, INFINITE);
		}
	}
}

DWORD WINAPI DispatchMemory(LPVOID lpParamter)
{
#ifdef DEBUG
	printf("Dispatch memory start...\n");
#endif
	while (1)
	{
		WaitForSingleObject(breakMemory, INFINITE); //等待Memory中断以进行Memory调度
		WaitForSingleObject(memoryQueue.queueFull[0], INFINITE);//等待，直到就绪队列中由进程
		ReleaseSemaphore(memoryQueue.queueFull[0], 1, NULL);//上一行代码将就绪队列的queueFull减一，在这里重新加回去。这两行代码的目的是确保就绪队列中有进程

		WaitForSingleObject(memoryQueue.queueMutex[0], INFINITE);//获得就绪队列的访问权限
		int subcript = memoryQueue.head[0];//队头
		processInMemory = memoryQueue.waitProcessID[0][subcript];//被选中的进程ID
		allPCB[processInMemory].nowState = run;//该进程状态为运行态
		ReleaseSemaphore(memoryQueue.queueEmpty[0], 1, NULL);//释放该队列的访问权限

		ReleaseSemaphore(contMemory, 1, NULL);//调度完成，告知模拟内存继续运行

	}
}