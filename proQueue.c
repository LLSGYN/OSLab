#include "proQueue.h"

//从当前就绪队列中删除进程
void KillProFromQueue(WaitQueue* curQueue, int ID)//参数：该进程所在的就绪队列 该进程的ID
{
	int priority_num = allPCB[ID].priority;//该进程的优先级
	if (allPCB[ID].events[allPCB[ID].eventID].eventType != occupyCPU && allPCB[ID].events[allPCB[ID].eventID].eventType != createProcess)
		priority_num = 0;//当进程的事件不是CPU相关的时候，不论进程有没有优先级，该进程都在优先级为0的相关就绪队列中
	int pos = -1;//该进程在就绪队列中的位置
	for (int i = curQueue->head[priority_num]; i != curQueue->tail[priority_num]; i = (i + 1) % MAX_PROCESS)//找待删进程在就绪队列中的位置，从队列头开始遍历
	{
		if (curQueue->waitProcessID[priority_num][i] == ID)
		{
			pos = i;
			break;
		}
	}

	//printf("queue test1\n");
	WaitForSingleObject(curQueue->queueMutex[priority_num], INFINITE);//接下来的操作要修改就绪队列，防止同一优先级的多个进程同时修改该就绪队列
	//printf("queue test2\n");
	if (pos != -1)
	{
		for (int i = pos; i != curQueue->head[priority_num]; i = (i - 1 + MAX_PROCESS) % MAX_PROCESS)//将待删进程前面的进程位置向后移
			curQueue->waitProcessID[priority_num][i] = curQueue->waitProcessID[priority_num][(i - 1 + MAX_PROCESS) % MAX_PROCESS];

		curQueue->head[priority_num] = (curQueue->head[priority_num] + 1) % MAX_PROCESS;//当前队列的队头向后移一个位置

		ReleaseSemaphore(curQueue->queueEmpty[priority_num], 1, NULL);//空余空间加一
		WaitForSingleObject(curQueue->queueFull[priority_num], 0);//队列中进程数减一
		WaitForSingleObject(curQueue->totalCnt, 0);//队列中进程总数减一

	}
	ReleaseSemaphore(curQueue->queueMutex[priority_num], 1, NULL);//修改完毕，释放该互斥量
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "-----Remove process %d successfully.\n", ID);//debug信息，暂时在屏幕上打印，后续输到log文件中
	ReleaseSemaphore(writeMutex, 1, NULL);

}

void AddProcessToQueue(WaitQueue* curQueue, int ID)//将进程加入到指定就绪队列中
{
	int priority_num = allPCB[ID].priority;//该进程的优先级
	if (allPCB[ID].events[allPCB[ID].eventID].eventType != occupyCPU && allPCB[ID].events[allPCB[ID].eventID].eventType != createProcess)
		priority_num = 0;//当进程的事件不是CPU相关的时候，不论进程有没有优先级，该进程都在优先级为0的相关就绪队列中

	WaitForSingleObject(curQueue->queueEmpty[priority_num], INFINITE);//获取空余空间
	WaitForSingleObject(curQueue->queueMutex[priority_num], INFINITE);//获取该优先级下的就绪队列的使用权

	int pos = curQueue->tail[priority_num];//队尾
	curQueue->waitProcessID[priority_num][pos] = ID;//将该进程添加进去
	pos = (pos + 1) % MAX_PROCESS;//队尾后移
	curQueue->tail[priority_num] = pos;

	ReleaseSemaphore(curQueue->queueFull[priority_num], 1, NULL);//队列中多了一个进程
	ReleaseSemaphore(curQueue->totalCnt, 1, NULL);//队列中进程总数加一
	ReleaseSemaphore(curQueue->queueMutex[priority_num], 1, NULL);//修改完毕，释放该互斥量

	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "-----Add process %d successfully.\n", ID);//debug信息,暂时在屏幕上打印，后续输到log文件中
	ReleaseSemaphore(writeMutex, 1, NULL);
}

//当一个事件完成时，将该进程从当前所在的就绪队列删除，并加入到下一事件所指向的就绪队列中
void UpdateEvent(int proID)
{
	if (proID == -1)//强制杀死进程可能会把这个ID置为-1
		return;
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "-----Process %d, job %d finish\n", proID, allPCB[proID].eventID);//debug信息
	ReleaseSemaphore(writeMutex, 1, NULL);
	// if (allPCB[proID].eventID < allPCB[proID].eventNum - 1)//进程还有事件没执行
	// {
	// 	int preEtype = allPCB[proID].events[allPCB[proID].eventID].eventType;//刚执行结束的事件类型
	// 	allPCB[proID].eventID++;//马上要执行的事件ID
	// 	int curEtype = allPCB[proID].events[allPCB[proID].eventID].eventType;//马上需要执行的事件类型
	// 	allPCB[proID].eventID--;//暂时退回到刚结束的事件ID，目的是不影响后续代码的执行
	// 	if (preEtype == curEtype)//若两者相同，则不用从原队列删除
	// 	{
	// 		allPCB[proID].eventID++;//事件ID号更新
	// 		allPCB[proID].eventTime = 0;//当前事件运行的时间为0
	// 		allPCB[proID].nowState = wait;//等待CPU重新调度
	// 		return;//更新进程事件结束
	// 	}
	// }
	//从原队列中删除此进程
	if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyIO)//和IO队列相关
	{
		int curQueueID = allPCB[proID].events[allPCB[proID].eventID].eventMsg.IDOfIO;
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Move process %d from queue IO %d.\n", proID, curQueueID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		KillProFromQueue(&waitIOQueue[curQueueID], proID);//从IO就绪队列中移出
	}
	else if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyCPU || allPCB[proID].events[allPCB[proID].eventID].eventType == createProcess)
	{
		//和CPU队列相关
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Move process %d from queue CPU.\n", proID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		KillProFromQueue(&readyQueue, proID);//同理

	}
	else//和内存队列相关
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Move process %d from queue Memory.\n", proID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		KillProFromQueue(&memoryQueue, proID);//同理
	}
	allPCB[proID].eventID++;//正在进行的事件编号加1，让下一事件运行
	allPCB[proID].eventTime = 0;//当前事件运行的时间为0
	if (allPCB[proID].eventID == allPCB[proID].eventNum)//进程的全部事件运行结束
	{
		DestoryProcess(proID);//进程自然销毁
	}
	else//进程还未结束
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Process %d, job %d start\n", proID, allPCB[proID].eventID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyIO)//和IO队列相关
		{
			int curQueueID = allPCB[proID].events[allPCB[proID].eventID].eventMsg.IDOfIO;//当前事件要使用的IO设备号
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "-----Insert process %d into queue IO %d.\n", proID, curQueueID);
			ReleaseSemaphore(writeMutex, 1, NULL);
			AddProcessToQueue(&waitIOQueue[curQueueID], proID);//将进程添加到与当前事件相关的就绪队列中
			allPCB[proID].nowState = wait;//需要获取IO资源，因此当前状态为等待
		}
		else if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyCPU || allPCB[proID].events[allPCB[proID].eventID].eventType == createProcess)
		{
			//和CPU队列相关
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "-----Insert process %d into queue CPU.\n", proID);
			ReleaseSemaphore(writeMutex, 1, NULL);
			AddProcessToQueue(&readyQueue, proID);//同理
			allPCB[proID].nowState = ready;//在与CPU相关的就绪队列中状态为就绪

		}
		else//和内存队列相关
		{
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "-----Insert process %d into queue Memory.\n", proID);
			ReleaseSemaphore(writeMutex, 1, NULL);
			AddProcessToQueue(&memoryQueue, proID);//同理
			allPCB[proID].nowState = wait;//需要获取内存的访问权限等资源，因此当前状态为等待
		}
	}
}