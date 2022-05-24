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
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----Error: Fail to create new process...\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		return 0;
	}
	srand(time(NULL));
	WaitForSingleObject(allPCB[nowID].processMutex, INFINITE);
	processCNT++;//进程总数加一
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "\n----Process %d created...\n", nowID);
	ReleaseSemaphore(writeMutex, 1, NULL);
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

	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----the priority of process is %d\n", allPCB[nowID].priority);
	ReleaseSemaphore(writeMutex, 1, NULL);
	
	allPCB[nowID].eventID = 0;
	allPCB[nowID].eventTime = 0;

	allPCB[nowID].pageNum = rand() % (MAX_PAGE_NUM - 2) + 3;//随机生成申请的内存页数 最小为3
	//调用接口函数向内存模块申请内存
	memory_alloc(nowID, allPCB[nowID].pageNum, 0);
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----event total RAM is %d\n", allPCB[nowID].pageNum);
	ReleaseSemaphore(writeMutex, 1, NULL);

	allPCB[nowID].eventNum = rand() % MAX_EVENT + 1;
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----event total num is %d\n", allPCB[nowID].eventNum);
	ReleaseSemaphore(writeMutex, 1, NULL);
	int mem_cnt = 0;//被占用的内存页数
	for (int i = 0; i < allPCB[nowID].eventNum; i++)
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "\tevent %d:", i);
		ReleaseSemaphore(writeMutex, 1, NULL);
		if (i == 0)
			allPCB[nowID].events[i].eventType = occupyCPU;//第一个事件总是去使用CPU
		else
			allPCB[nowID].events[i].eventType = rand() % MAX_EVENT_TYPE;//事件类型随机，不含编译类型事件
			// allPCB[nowID].events[i].eventType = 5;// 时间为占用IO

		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "  eventType: %d", allPCB[nowID].events[i].eventType);
		ReleaseSemaphore(writeMutex, 1, NULL);
		if (fatherProcessID != -1 && allPCB[nowID].events[i].eventType == createProcess)
			allPCB[nowID].events[i].eventType = occupyCPU;//将子进程的创建进程事件视为特殊的占用CPU的事件

		if (allPCB[nowID].events[i].eventType == createProcess)//当前事件为创建进程事件
			allPCB[nowID].events[i].time = CREATE_PROCESS_TIME;//设置该事件需要的时间片
		else if (allPCB[nowID].events[i].eventType == occupyIO)//占用IO事件
		{
			allPCB[nowID].events[i].eventMsg.IDOfIO = rand() % IO_NUM;//分配IO号
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "  alloc IO_num: %d", allPCB[nowID].events[i].eventMsg.IDOfIO);
			ReleaseSemaphore(writeMutex, 1, NULL);
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
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "  needs time: %d\n", allPCB[nowID].events[i].time);
		ReleaseSemaphore(writeMutex, 1, NULL);
	}

	allPCB[nowID].CPUtime = 0;
	allPCB[nowID].startTime = time(NULL); //记录进程创建时间
	allPCB[nowID].IOID = -1;              //
	allPCB[nowID].heapUsed = 0;
	allPCB[nowID].stackUsed = 0;
	AddProcessToQueue(&readyQueue, nowID);
	allPCB[nowID].nowState = ready;
	ReleaseSemaphore(allPCB[nowID].processMutex, 1, NULL);
	return 1;
}

int CreateMyDiyProcess(char* processName, int fatherProcessID, char* processFileName)
{
	srand(time(NULL));
	FILE* processfile = fopen(processFileName, "r");
	if (processfile == NULL) {
		printf("Process file does not exist,please check it!\n");
		return 0;
	}
	int Id = GetNextUnusedProcessID();
	if (Id == -1) {
		printf("Create process fail,sorry!");
		return 0;
	}
	WaitForSingleObject(allPCB[Id].processMutex, INFINITE);
	processCNT++;
	for (int i = 0; i < MAX_NAME; i++)
		allPCB[Id].name[i] = processName[i];
	allPCB[Id].ID = Id;
	allPCB[Id].fatherProID = fatherProcessID;
	if (allPCB[Id].fatherProID != -1)
		allPCB[Id].sonPro = 1;   //是子进程
	if (CPUMode == 0 || CPUMode == 2)  //FCFS RR
		allPCB[Id].priority = 0;  //固定的优先级
	else  //非抢占的静态优先级
		allPCB[Id].priority = rand() % (PRIORITY_NUM - 1);//随机的一个优先级
	allPCB[Id].eventID = 0;
	allPCB[Id].eventTime = 0;

	fscanf(processfile, "%d", &allPCB[Id].eventNum);
	if (allPCB[Id].eventNum > MAX_EVENT) {   //如果事件数超过最大限制,报错同时销毁进程
		printf("Create process %d failed,there are too many events!\n", Id);
		//将信息打印到日志文件中，还未实现
		usedProcessID[Id] = 0;
		processCNT--;
		ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
		fclose(processfile);
		return 0;
	}
	for (int i = 0; i < allPCB[Id].eventNum; i++) {
		fscanf(processfile, "%d %d %d", &allPCB[Id].events[i].eventType, &allPCB[Id].events[i].time, &allPCB[Id].events[i].needRAM);
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "\t event %d: eventType: %d needstime: %d\n", i, allPCB[Id].events[i].eventType, allPCB[Id].events[i].time);
		ReleaseSemaphore(writeMutex, 1, NULL);
		allPCB[Id].pageNum += allPCB[Id].events[i].needRAM;
		if (i == 0) {  //保证第一个事件是占用CPU
			if (allPCB[Id].events[i].eventType != occupyCPU && allPCB[Id].events[i].eventType != createProcess) {
				WaitForSingleObject(writeMutex, INFINITE);
				fprintf(logs, "Create process %d failed,the first event is wrong!\n", Id);
				ReleaseSemaphore(writeMutex, 1, NULL);
				//将信息打印到日志文件中，还未实现
				usedProcessID[Id] = 0;
				processCNT--;
				ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
				fclose(processfile);
				return 0;
			}
			// 分配时间片
			// if (allPCB[Id].events[i].eventType == occupyCPU)
				// allPCB[Id].events[i].time = rand() % MAX_NEED_TIME + 1; //事件所需时间片数随机
			if (allPCB[Id].events[i].eventType == createProcess)
				allPCB[Id].events[i].time = CREATE_PROCESS_TIME;
		}
		else {
			//若进程申请的时间或内存超出限制
			if (allPCB[Id].events[i].time > MAX_NEED_TIME || allPCB[Id].events[i].needRAM > MAX_PAGE_NUM) {
				WaitForSingleObject(writeMutex, INFINITE);
				fprintf(logs, "Create process %d failed,the process has too much time or memory!\n", Id);
				ReleaseSemaphore(writeMutex, 1, NULL);
				//将信息打印到日志文件中，还未实现
				usedProcessID[Id] = 0;
				processCNT--;
				ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
				fclose(processfile);
				return 0;
			}
			//若进程事件为占用IO，保证其合理性
			if (allPCB[Id].events[i].eventType == occupyIO) {
				fscanf(processfile, "%d", &allPCB[Id].events[i].eventMsg.IDOfIO);
				//保证该事件所使用的IO的合理性
				if (allPCB[Id].events[i].eventMsg.IDOfIO >= IO_NUM) {
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "Create process %d failed,the process uses illogical IO!\n", Id);
					ReleaseSemaphore(writeMutex, 1, NULL);
					//将信息打印到日志文件中，还未实现
					usedProcessID[Id] = 0;
					processCNT--;
					fclose(processfile);
					ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
					return 0;
				}
				// allPCB[Id].events[i].time = rand() % MAX_NEED_TIME + 1;
			}
			//若进程事件为读写内存，保证其合理性
			else if (allPCB[Id].events[i].eventType == proReadMem || allPCB[Id].events[i].eventType == proWriteMem) {
				int offset; //偏移量
				fscanf(processfile, "%d %d", &allPCB[Id].pageNum, &offset);
				if (offset >= 1024) {  //保证偏移量为0-1023之间
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "Create process %d failed,offset too large.\n", Id);
					ReleaseSemaphore(writeMutex, 1, NULL);
					//将打印信息写入到日志文件中
					usedProcessID[Id] = 0;
					processCNT--;
					ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
					fclose(processfile);
					return 0;
				}
				int start_page = rand() % allPCB[Id].pageNum;//逻辑页号
				int remainBytes = (allPCB[Id].pageNum - start_page - 1) * 1024 + 1023 - offset;//剩余字节
				int len = rand() % remainBytes + 1;//读写的长度
				allPCB[Id].events[i].eventMsg.wrMsg.startPageID = start_page;
				allPCB[Id].events[i].eventMsg.wrMsg.offset = offset;
				allPCB[Id].events[i].eventMsg.wrMsg.len = len;
				allPCB[Id].events[i].time = (offset + len + 1024) / 1024 * TIME_PER_PAGE;
			}
			//若进程事件为申请堆栈，保证其合理性
			else if (allPCB[Id].events[i].eventType == heapAlloc || allPCB[Id].events[i].eventType == stackAlloc) {
				fscanf(processfile, "%d", &allPCB[i].events[i].eventMsg.allocNum);  //读取页数
				if (allPCB[Id].events[i].eventMsg.allocNum > MAX_PAGE_NUM) {
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "Create process %d failed,apply too many pages", Id);
					ReleaseSemaphore(writeMutex, 1, NULL);
					//将打印信息写入到日志文件中
					usedProcessID[Id] = 0;
					processCNT--;
					ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
					fclose(processfile);
					return 0;
				}
				allPCB[Id].events[i].time = MY_ALLOC_TIME;
			}
		}
	}
	fclose(processfile);
	allPCB[Id].CPUtime = 0;
	allPCB[Id].startTime = time(NULL); //记录进程创建时间
	allPCB[Id].IOID = -1;              //
	allPCB[Id].heapUsed = 0;
	allPCB[Id].stackUsed = 0;
	printf("----Process %d created...\n", Id);
	AddProcessToQueue(&readyQueue, Id);
	allPCB[Id].nowState = ready;
	memory_alloc(Id, allPCB[Id].pageNum, 0);
	ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
	return 1;
}

// int CreateCompileProcess(char* fileName)
// {
// 
// }
