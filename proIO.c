#include "proIO.h"

DWORD WINAPI virtualIO(LPVOID paramter);
DWORD WINAPI ioScheduling(LPVOID paramter);

DWORD WINAPI virtualIO(LPVOID paramter)
{
	int ID = *((int*)paramter); // ID是io设备号
#ifdef DEBUG
	fprintf(logs, "----IO %d start...\n", ID);
	fprintf(logs, "----Scheduling strategy is FCFS...\n");
#endif
	WaitForSingleObject(contIO[ID], INFINITE);
	while (1)
	{
		int pid = processInIO[ID];
		WaitForSingleObject(timeLockForIO[ID], INFINITE); // 获取时间片
#ifdef DEBUG
		printf("io get time successfully.\n");
#endif
		WaitForSingleObject(proInIOMutex[ID], INFINITE); // 获取IO设备运行权限，在强制杀死进程的时候可能会争夺该信号量

		if (pid != -1) // 当前IO设备有进程在占用
		{
			WaitForSingleObject(killMutex, INFINITE); // 保证更新进程信息时不会被强制杀死
			WaitForSingleObject(allPCB[pid].processMutex, INFINITE); // 获取进程修改权限
			ReleaseSemaphore(killMutex, 1, NULL); // 释放killMUtex
			int curEID = allPCB[pid].eventID; // 当前正在进行的事件编号
			allPCB[pid].eventTime++;
			if (allPCB[pid].eventTime == allPCB[pid].events[curEID].time) // 当前事件执行完毕
			{
				ReleaseSemaphore(proInIOMutex[ID], 1, NULL); // 释放IO设备运行权限信号量，在UpdateEvent之前释放，防止发生死锁
				// WaitForSingleObject(killMutex, INFINITE); // 保证更新事件信息的时候该进程不可以被强制销毁
				UpdateEvent(pid); // 更新此进程的事件信息
				allPCB[pid].IOID = -1; // IO设备使用完毕
				// ReleaseSemaphore(killMutex, 1, NULL); // 释放killMutex
#ifdef DEBUG
				printf("IO finished\n");
#endif
				ReleaseSemaphore(allPCB[pid].processMutex, 1, NULL); // 释放进程管理权限
				ReleaseSemaphore(breakIO[ID], 1, NULL); // 等待IO设备进行调度管理
				WaitForSingleObject(contIO[ID], INFINITE); // IO设备完成调度管理
			}
			else
			{
				// 事件未完成，继续循环
				ReleaseSemaphore(proInIOMutex[ID], 1, NULL);
				ReleaseSemaphore(allPCB[pid].processMutex, 1, NULL);
			}
		}
		else
		{
			ReleaseSemaphore(proInIOMutex[ID], 1, NULL); // 释放IO设备运行权限信号量
			ReleaseSemaphore(breakIO[ID], 1, NULL); // IO中断，等待IO调度
			WaitForSingleObject(contIO[ID], INFINITE); // 等待IO调度完成后，继续运行IO设备
		}
	}
}

DWORD WINAPI ioScheduling(LPVOID paramter)
{
	int ID = *((int*)paramter);
#ifdef DEBUG
	fprintf(logs, "----Dispatch IO %d start...\n", ID);
#endif
	while (1)
	{
		WaitForSingleObject(breakIO[ID], INFINITE); // 等待IO中断以进行IO调度
		WaitForSingleObject(waitIOQueue[ID].queueFull[0], INFINITE); // 等待IO队列中出现进程
		ReleaseSemaphore(waitIOQueue[ID].queueFull[0], 1, NULL); // 对full信号量-1，表示将一个进程从等待队列分配给IO设备

		WaitForSingleObject(waitIOQueue[ID].queueMutex[0], INFINITE); // 获取等待队列的访问权限

		int nextPro = waitIOQueue[ID].head[0]; // 队列开始位置
		//processInIO[ID] = nextPro;
		processInIO[ID] = waitIOQueue[ID].waitProcessID[0][nextPro];
		allPCB[processInIO[ID]].IOID = ID;

		ReleaseSemaphore(waitIOQueue[ID].queueMutex[0], 1, NULL);
		ReleaseSemaphore(contIO[ID], 1, NULL);
	}
}