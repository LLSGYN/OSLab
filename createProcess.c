#include "createProcess.h"

//得到下一个可用的进程ID
int GetNextUnusedProcessID()
{
	int nxtID = -1;
	WaitForSingleObject(usedProcessIDMutex, INFINITE);//锁住下面的数组，防止被多个进程同时访问造成不同步的问题
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		if (usedProcessID[i] == 0)
		{
			usedProcessID[i] = 1;
			nxtID = i;
			break;
		}
	}
	ReleaseSemaphore(usedProcessIDMutex, 1, NULL);//释放该锁
	return nxtID;
}

int CreateMyProcess(char* processName, int fatherProcessID)//创建用户进程
{
	int nowID = GetNextUnusedProcessID();//获取进程ID
	if (nowID == -1)//获取ID号失败
	{
		printf("Fail to create new process...\n");
		return 0;
	}
	processCNT++;//进程总数加一
	printf("**********process totol count is %d\n", processCNT);
	for (int i = 0; i < MAX_NAME; i++)
		allPCB[nowID].name[i] = processName[i];//新进程名字
	allPCB[nowID].ID = nowID;//新进程ID
	allPCB[nowID].fatherProID = fatherProcessID;
	if (fatherProcessID != -1)
		allPCB[nowID].sonPro = 1;//该进程是子进程

	if (CPUMode == 0 || CPUMode == 2)//FCFS RR
		allPCB[nowID].priority = 0;//固定的优先级
	else//非抢占的静态优先级
		allPCB[nowID].priority = rand() % (PRIORITY_NUM - 1);//随机的一个优先级

	allPCB[nowID].eventID = 0;
	allPCB[nowID].eventTime = 0;

	allPCB[nowID].pageNum = rand() % (MAX_PAGE_NUM - 2) + 3;//随机生成申请的内存页数 最小为3
	//调用接口函数向内存模块申请内存
	//********************
	//********************
	//MemoryAlloc(nowID, allPCB[nowID].pageNum, allPCB[nowID].fatherProID); //调用接口函数向内存模块申请内存
	allPCB[nowID].eventNum = rand() % 1 + 1;//随机生成事件总数 ************************** rand() % MAX_EVENT
	printf("*************event total num is %d\n", allPCB[nowID].eventNum);
	int mem_cnt = 0;//被占用的内存页数
	for (int i = 0; i < allPCB[nowID].eventNum; i++)
	{
		if (i == 0)
			allPCB[nowID].events[i].eventType = occupyCPU;//第一个事件总是去使用CPU
		else
			allPCB[nowID].events[i].eventType = rand() % MAX_EVENT_TYPE;//事件类型随机，不含编译类型事件
		
		printf("*******eventType is %d\n", allPCB[nowID].events[i].eventType);
		if (fatherProcessID != -1 && allPCB[nowID].events[i].eventType == createProcess)
			allPCB[nowID].events[i].eventType = occupyCPU;//将子进程的创建进程事件视为特殊的占用CPU的事件
		
		if(allPCB[nowID].events[i].eventType == createProcess)//当前事件为创建进程事件
			allPCB[nowID].events[i].time = CREATE_PROCESS_TIME;//设置该事件需要的时间片
		else if (allPCB[nowID].events[i].eventType == occupyIO)//占用IO事件
		{
			allPCB[nowID].events[i].eventMsg.IDOfIO = rand() % IO_NUM;//分配IO号
			printf("************alloc IO_num is %d...\n", allPCB[nowID].events[i].eventMsg.IDOfIO);
			allPCB[nowID].events[i].time = rand() % MAX_NEED_TIME + 1; //事件所需时间片数随机
		}
		else if (allPCB[nowID].events[i].eventType == heapAlloc || allPCB[nowID].events[i].eventType == stackAlloc)//申请堆栈事件
		{
			int getMem = rand() % 100;//事件需要内存的概率 小于MAX_NEED_MEMORY则需要
			if (getMem < NEED_MEMORY_PERCENT && mem_cnt < allPCB[nowID].pageNum)//需要内存 而且 内存没被分配完
			{
				allPCB[nowID].events[i].needRAM = rand() % (allPCB[nowID].pageNum - mem_cnt) + 1;//随机获得内存大小（单位：页
				mem_cnt = mem_cnt + allPCB[nowID].events[i].needRAM;//被占用的内存页数增加
			}
			else
				allPCB[nowID].events[i].needRAM = 0;//不需要内存
			allPCB[nowID].events[i].time = MY_ALLOC_TIME;//申请堆栈需要的时间片
		}
		else if (allPCB[nowID].events[i].eventType == proReadMem || allPCB[nowID].events[i].eventType == proWriteMem)//读写内存
		{
			//随机生成读写内存需要的逻辑页号、偏移量、读写字符串长度
			int start_page = rand() % allPCB[nowID].pageNum;//逻辑页号
			int offset = rand() % 1024;//页内偏移量
			int remainBytes = (allPCB[nowID].pageNum - start_page - 1) * 1024 + 1023 - offset;//剩余字节
			int len = rand() % remainBytes + 1;//读写的长度
			allPCB[nowID].events[i].eventMsg.wrMsg.startPageID = start_page;
			allPCB[nowID].events[i].eventMsg.wrMsg.offset = offset;
			allPCB[nowID].events[i].eventMsg.wrMsg.len = len;
			allPCB[nowID].events[i].time = (offset + len + 1024) / 1024 * TIME_PER_PAGE;
		}
		else//occupyCPU
			allPCB[nowID].events[i].time = rand() % MAX_NEED_TIME + 1; //事件所需时间片数随机
		printf("**************this event needs %d time\n", allPCB[nowID].events[i].time);
	}

	allPCB[nowID].CPUtime = 0;
	allPCB[nowID].startTime = time(NULL); //记录进程创建时间
	allPCB[nowID].IOID = -1;              //
	allPCB[nowID].heapUsed = 0;
	allPCB[nowID].stackUsed = 0;
	printf("----Process %d created...\n", nowID);
	AddProcessToQueue(&readyQueue, nowID);
	allPCB[nowID].nowState = ready;
	return 1;
}

int CreateMyDiyProcess(char* processName, int fatherProcessID, char* processFileName)
{

}

int CreateCompileProcess(char* fileName)
{

}