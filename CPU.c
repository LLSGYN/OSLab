#include "CPU.h"
#include "createProcess.h"
#include "dfVar.h"
#include "proQueue.h"

/*CPUMode 默认为0 调度策略为FCFS
              为1 调度策略为非抢占的静态优先级
              为2 调度策略为RR
*/

//模拟CPU的运行
DWORD WINAPI VirCPU(LPVOID lpParamter)
{
#ifdef DEBUG
	printf("CPU start...\n");
	if (CPUMode == 0)
		printf("CPU mode is FCFS...\n");
	else if (CPUMode == 1)
		printf("CPU mode is non-preemptive priority-based...\n");
	else if (CPUMode == 2)
		printf("CPU mode is RR...\n");
#endif
	if (CPUMode > 2)
	{
		printf("CPU mode error...\n");
		exit(0);
	}
	WaitForSingleObject(contCPU, INFINITE);//等待CPU调度完成，进行CPU的运行
#ifdef DEBUG
	printf("****************get contCPU.\n");
#endif
	while (1)
	{
		WaitForSingleObject(timeLockForCPU, INFINITE);//获取时间片
#ifdef DEBUG
		printf("*************get time successfully.\n");
#endif
		WaitForSingleObject(proInCPUMutex, INFINITE);//获取CPU运行权限，在强制杀死进程的时候可能会争夺该信号量
		WaitForSingleObject(allPCB[processInCPU].processMutex, INFINITE);

		if (processInCPU != -1)//有进程正在执行
		{
			int curEID = allPCB[processInCPU].eventID;//当前正在进行的事件编号
			allPCB[processInCPU].CPUtime++;//该进程占用的CPU时间增加
			allPCB[processInCPU].eventTime++;//该进程当前事件运行的时间增加
			if (allPCB[processInCPU].eventTime == allPCB[processInCPU].events[curEID].time)//当前事件执行完成，需要切换为下一个事件
			{
				if (allPCB[processInCPU].events[allPCB[processInCPU].eventID].eventType == createProcess) //若事件类型为创建新进程
				{
					char name[MAX_NAME] = "A new process";//新进程名
					//调用创建进程函数进行创建
					//############################
					CreateMyProcess(name, processInCPU);//创建进程
				}
				else if (allPCB[processInCPU].events[allPCB[processInCPU].eventID].eventType == compile)//若事件类型为文件相关的操作
				{
					//进行相应的操作
					//############################
				}
				ReleaseSemaphore(proInCPUMutex, 1, NULL);//释放CPU运行权限信号量，在UpdateEvent之前释放，防止发生死锁

				WaitForSingleObject(killMutex, INFINITE);//保证更新进程信息的时候该进程不可以被强制销毁
				UpdateEvent(processInCPU);//更新此进程的事件信息
				ReleaseSemaphore(allPCB[processInCPU].processMutex, 1, NULL);
				ReleaseSemaphore(killMutex, 1, NULL);//释放killMutex

				ReleaseSemaphore(breakCPU, 1, NULL);//CPU中断，等待CPU重新调度
				WaitForSingleObject(contCPU, INFINITE);//等待CPU调度完成后，继续CPU的运行
			}
			else//当前事件未执行完成
			{
				/*
				分两种情况：若为RR调度，则将该进程替换
				若不为RR，则需要重新获取信号量继续执行
				*/
				if (CPUMode == 0 || CPUMode == 1)//FCFS、优先级
				{
					//空操作
				}
				else if (CPUMode == 2)//RR
				{
					printf("----Move process %d from queue CPU.\n", processInCPU);
					KillProFromQueue(&readyQueue, processInCPU);//将进程从当前队列删除
					printf("----Insert process %d into queue CPU.\n", processInCPU);
					AddProcessToQueue(&readyQueue, processInCPU);//将进程加到当前队列尾部
					allPCB[processInCPU].nowState = ready;

					ReleaseSemaphore(breakCPU, 1, NULL);//CPU中断，等待CPU重新调度
					WaitForSingleObject(contCPU, INFINITE);//等待CPU调度完成后，继续CPU的运行
				}

				ReleaseSemaphore(proInCPUMutex, 1, NULL);//释放CPU运行权限信号量
				ReleaseSemaphore(allPCB[processInCPU].processMutex, 1, NULL);
			}
		}
		else//没有进程正在执行
		{
			ReleaseSemaphore(proInCPUMutex, 1, NULL);//释放CPU运行权限信号量
			ReleaseSemaphore(allPCB[processInCPU].processMutex, 1, NULL);
			ReleaseSemaphore(breakCPU, 1, NULL);//CPU中断，等待CPU重新调度
			WaitForSingleObject(contCPU, INFINITE);//等待CPU调度完成后，继续CPU的运行
		}
	}
}

//模拟CPU的调度
DWORD WINAPI DispatchCPU(LPVOID lpParamter)
{
#ifdef DEBUG
	printf("Dispatch CPU start...\n");
#endif
	if (CPUMode == 0 || CPUMode == 2)//FCFS RR,优先级默认为0,调度处于队头的进程运行
	{
		while (1)
		{
			WaitForSingleObject(breakCPU, INFINITE);//等待CPU中断以进行CPU调度
			WaitForSingleObject(readyQueue.queueFull[0], INFINITE);//等待，直到就绪队列中有进程
			ReleaseSemaphore(readyQueue.queueFull[0], 1, NULL);//上一行代码将就绪队列的queueFull减一，在这里重新加回去。这两行代码的目的是确保就绪队列中有进程
			
			WaitForSingleObject(readyQueue.queueMutex[0], INFINITE);//获得就绪队列的访问权限
			int subcript = readyQueue.head[0];//队头
			processInCPU = readyQueue.waitProcessID[0][subcript];//被选中的进程ID
			allPCB[processInCPU].nowState = run;//该进程状态为运行态
			ReleaseSemaphore(readyQueue.queueMutex[0], 1, NULL);//释放该就绪队列的访问权限

			ReleaseSemaphore(contCPU, 1, NULL);//调度完成，告知CPU继续运行
#ifdef DEBUG
			printf("**************CPU dispatch successfully...\n");
#endif

		}
	}
	else if (CPUMode == 1)//非抢占的静态优先级
	{
		while (1)
		{
			WaitForSingleObject(breakCPU, INFINITE);//等待CPU中断以进行CPU调度

			WaitForSingleObject(readyQueue.totalCnt, INFINITE);//等待，直到就绪队列中有进程
			ReleaseSemaphore(readyQueue.totalCnt, 1, NULL);//这两行代码的目的是确保就绪队列中有进程

			for (int i = 0; i < PRIORITY_NUM; i++)
			{
				WaitForSingleObject(readyQueue.queueMutex[i], INFINITE);//获取当前优先级下就绪队列的使用权
				if (readyQueue.head[i] == readyQueue.tail[i])//当前优先级下的就绪队列为空
					ReleaseSemaphore(readyQueue.queueMutex[i], 1, NULL);//释放访问权限，接着访问下一个队列
				else
				{
					int subcript = readyQueue.head[i];//队头
					processInCPU = readyQueue.waitProcessID[i][subcript];//被选中的进程ID
					allPCB[processInCPU].nowState = run;//该进程状态改为运行态
					ReleaseSemaphore(readyQueue.queueMutex[i], 1, NULL);//释放该就绪队列的访问权限

					ReleaseSemaphore(contCPU, 1, NULL);//调度完成，告知CPU继续运行
					break;
				}
			}
		}
	}
}