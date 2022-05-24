#include "createProcess.h"

//�õ���һ�����õĽ���ID
int GetNextUnusedProcessID()
{
	int nxtID = -1;
	WaitForSingleObject(usedProcessIDMutex, INFINITE);//��ס��������飬��ֹ���������ͬʱ������ɲ�ͬ��������
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		if (usedProcessID[i] == 0)
		{
			usedProcessID[i] = 1;
			nxtID = i;
			break;
		}
	}
	ReleaseSemaphore(usedProcessIDMutex, 1, NULL);//�ͷŸ���
	return nxtID;
}

int CreateMyProcess(char* processName, int fatherProcessID)//�����û�����
{
	int nowID = GetNextUnusedProcessID();//��ȡ����ID
	if (nowID == -1)//��ȡID��ʧ��
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----Error: Fail to create new process...\n");
		ReleaseSemaphore(writeMutex, 1, NULL);
		return 0;
	}
	srand(time(NULL));
	WaitForSingleObject(allPCB[nowID].processMutex, INFINITE);
	processCNT++;//����������һ
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "\n----Process %d created...\n", nowID);
	ReleaseSemaphore(writeMutex, 1, NULL);
	for (int i = 0; i < MAX_NAME; i++)
		allPCB[nowID].name[i] = processName[i];//�½�������
	allPCB[nowID].ID = nowID;//�½���ID
	allPCB[nowID].fatherProID = fatherProcessID;
	if (fatherProcessID != -1)
		allPCB[nowID].sonPro = 1;//�ý������ӽ���

	if (CPUMode == 0 || CPUMode == 2)//FCFS RR
		allPCB[nowID].priority = 0;//�̶������ȼ�
	else//����ռ�ľ�̬���ȼ�
		allPCB[nowID].priority = rand() % (PRIORITY_NUM - 1);//�����һ�����ȼ�

	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----the priority of process is %d\n", allPCB[nowID].priority);
	ReleaseSemaphore(writeMutex, 1, NULL);
	
	allPCB[nowID].eventID = 0;
	allPCB[nowID].eventTime = 0;

	allPCB[nowID].pageNum = rand() % (MAX_PAGE_NUM - 2) + 3;//�������������ڴ�ҳ�� ��СΪ3
	//���ýӿں������ڴ�ģ�������ڴ�
	memory_alloc(nowID, allPCB[nowID].pageNum, 0);
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----event total RAM is %d\n", allPCB[nowID].pageNum);
	ReleaseSemaphore(writeMutex, 1, NULL);

	allPCB[nowID].eventNum = rand() % MAX_EVENT + 1;
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----event total num is %d\n", allPCB[nowID].eventNum);
	ReleaseSemaphore(writeMutex, 1, NULL);
	int mem_cnt = 0;//��ռ�õ��ڴ�ҳ��
	for (int i = 0; i < allPCB[nowID].eventNum; i++)
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "\tevent %d:", i);
		ReleaseSemaphore(writeMutex, 1, NULL);
		if (i == 0)
			allPCB[nowID].events[i].eventType = occupyCPU;//��һ���¼�����ȥʹ��CPU
		else
			allPCB[nowID].events[i].eventType = rand() % MAX_EVENT_TYPE;//�¼�����������������������¼�
			// allPCB[nowID].events[i].eventType = 5;// ʱ��Ϊռ��IO

		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "  eventType: %d", allPCB[nowID].events[i].eventType);
		ReleaseSemaphore(writeMutex, 1, NULL);
		if (fatherProcessID != -1 && allPCB[nowID].events[i].eventType == createProcess)
			allPCB[nowID].events[i].eventType = occupyCPU;//���ӽ��̵Ĵ��������¼���Ϊ�����ռ��CPU���¼�

		if (allPCB[nowID].events[i].eventType == createProcess)//��ǰ�¼�Ϊ���������¼�
			allPCB[nowID].events[i].time = CREATE_PROCESS_TIME;//���ø��¼���Ҫ��ʱ��Ƭ
		else if (allPCB[nowID].events[i].eventType == occupyIO)//ռ��IO�¼�
		{
			allPCB[nowID].events[i].eventMsg.IDOfIO = rand() % IO_NUM;//����IO��
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "  alloc IO_num: %d", allPCB[nowID].events[i].eventMsg.IDOfIO);
			ReleaseSemaphore(writeMutex, 1, NULL);
			allPCB[nowID].events[i].time = rand() % MAX_NEED_TIME + 1; //�¼�����ʱ��Ƭ�����
		}
		else if (allPCB[nowID].events[i].eventType == heapAlloc || allPCB[nowID].events[i].eventType == stackAlloc)//�����ջ�¼�
		{
			int getMem = rand() % 100;//�¼���Ҫ�ڴ�ĸ��� С��MAX_NEED_MEMORY����Ҫ
			if (getMem < NEED_MEMORY_PERCENT && mem_cnt < allPCB[nowID].pageNum)//��Ҫ�ڴ� ���� �ڴ�û��������
			{
				allPCB[nowID].events[i].needRAM = rand() % (allPCB[nowID].pageNum - mem_cnt) + 1;//�������ڴ��С����λ��ҳ
				mem_cnt = mem_cnt + allPCB[nowID].events[i].needRAM;//��ռ�õ��ڴ�ҳ������
			}
			else
				allPCB[nowID].events[i].needRAM = 0;//����Ҫ�ڴ�
			allPCB[nowID].events[i].time = MY_ALLOC_TIME;//�����ջ��Ҫ��ʱ��Ƭ
		}
		else if (allPCB[nowID].events[i].eventType == proReadMem || allPCB[nowID].events[i].eventType == proWriteMem)//��д�ڴ�
		{
			//������ɶ�д�ڴ���Ҫ���߼�ҳ�š�ƫ��������д�ַ�������
			int start_page = rand() % allPCB[nowID].pageNum;//�߼�ҳ��
			int offset = rand() % 1024;//ҳ��ƫ����
			int remainBytes = (allPCB[nowID].pageNum - start_page - 1) * 1024 + 1023 - offset;//ʣ���ֽ�
			int len = rand() % remainBytes + 1;//��д�ĳ���
			allPCB[nowID].events[i].eventMsg.wrMsg.startPageID = start_page;
			allPCB[nowID].events[i].eventMsg.wrMsg.offset = offset;
			allPCB[nowID].events[i].eventMsg.wrMsg.len = len;
			allPCB[nowID].events[i].time = (offset + len + 1024) / 1024 * TIME_PER_PAGE;
		}
		else//occupyCPU
			allPCB[nowID].events[i].time = rand() % MAX_NEED_TIME + 1; //�¼�����ʱ��Ƭ�����
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "  needs time: %d\n", allPCB[nowID].events[i].time);
		ReleaseSemaphore(writeMutex, 1, NULL);
	}

	allPCB[nowID].CPUtime = 0;
	allPCB[nowID].startTime = time(NULL); //��¼���̴���ʱ��
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
		allPCB[Id].sonPro = 1;   //���ӽ���
	if (CPUMode == 0 || CPUMode == 2)  //FCFS RR
		allPCB[Id].priority = 0;  //�̶������ȼ�
	else  //����ռ�ľ�̬���ȼ�
		allPCB[Id].priority = rand() % (PRIORITY_NUM - 1);//�����һ�����ȼ�
	allPCB[Id].eventID = 0;
	allPCB[Id].eventTime = 0;

	fscanf(processfile, "%d", &allPCB[Id].eventNum);
	if (allPCB[Id].eventNum > MAX_EVENT) {   //����¼��������������,����ͬʱ���ٽ���
		printf("Create process %d failed,there are too many events!\n", Id);
		//����Ϣ��ӡ����־�ļ��У���δʵ��
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
		if (i == 0) {  //��֤��һ���¼���ռ��CPU
			if (allPCB[Id].events[i].eventType != occupyCPU && allPCB[Id].events[i].eventType != createProcess) {
				WaitForSingleObject(writeMutex, INFINITE);
				fprintf(logs, "Create process %d failed,the first event is wrong!\n", Id);
				ReleaseSemaphore(writeMutex, 1, NULL);
				//����Ϣ��ӡ����־�ļ��У���δʵ��
				usedProcessID[Id] = 0;
				processCNT--;
				ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
				fclose(processfile);
				return 0;
			}
			// ����ʱ��Ƭ
			// if (allPCB[Id].events[i].eventType == occupyCPU)
				// allPCB[Id].events[i].time = rand() % MAX_NEED_TIME + 1; //�¼�����ʱ��Ƭ�����
			if (allPCB[Id].events[i].eventType == createProcess)
				allPCB[Id].events[i].time = CREATE_PROCESS_TIME;
		}
		else {
			//�����������ʱ����ڴ泬������
			if (allPCB[Id].events[i].time > MAX_NEED_TIME || allPCB[Id].events[i].needRAM > MAX_PAGE_NUM) {
				WaitForSingleObject(writeMutex, INFINITE);
				fprintf(logs, "Create process %d failed,the process has too much time or memory!\n", Id);
				ReleaseSemaphore(writeMutex, 1, NULL);
				//����Ϣ��ӡ����־�ļ��У���δʵ��
				usedProcessID[Id] = 0;
				processCNT--;
				ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
				fclose(processfile);
				return 0;
			}
			//�������¼�Ϊռ��IO����֤�������
			if (allPCB[Id].events[i].eventType == occupyIO) {
				fscanf(processfile, "%d", &allPCB[Id].events[i].eventMsg.IDOfIO);
				//��֤���¼���ʹ�õ�IO�ĺ�����
				if (allPCB[Id].events[i].eventMsg.IDOfIO >= IO_NUM) {
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "Create process %d failed,the process uses illogical IO!\n", Id);
					ReleaseSemaphore(writeMutex, 1, NULL);
					//����Ϣ��ӡ����־�ļ��У���δʵ��
					usedProcessID[Id] = 0;
					processCNT--;
					fclose(processfile);
					ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
					return 0;
				}
				// allPCB[Id].events[i].time = rand() % MAX_NEED_TIME + 1;
			}
			//�������¼�Ϊ��д�ڴ棬��֤�������
			else if (allPCB[Id].events[i].eventType == proReadMem || allPCB[Id].events[i].eventType == proWriteMem) {
				int offset; //ƫ����
				fscanf(processfile, "%d %d", &allPCB[Id].pageNum, &offset);
				if (offset >= 1024) {  //��֤ƫ����Ϊ0-1023֮��
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "Create process %d failed,offset too large.\n", Id);
					ReleaseSemaphore(writeMutex, 1, NULL);
					//����ӡ��Ϣд�뵽��־�ļ���
					usedProcessID[Id] = 0;
					processCNT--;
					ReleaseSemaphore(allPCB[Id].processMutex, 1, NULL);
					fclose(processfile);
					return 0;
				}
				int start_page = rand() % allPCB[Id].pageNum;//�߼�ҳ��
				int remainBytes = (allPCB[Id].pageNum - start_page - 1) * 1024 + 1023 - offset;//ʣ���ֽ�
				int len = rand() % remainBytes + 1;//��д�ĳ���
				allPCB[Id].events[i].eventMsg.wrMsg.startPageID = start_page;
				allPCB[Id].events[i].eventMsg.wrMsg.offset = offset;
				allPCB[Id].events[i].eventMsg.wrMsg.len = len;
				allPCB[Id].events[i].time = (offset + len + 1024) / 1024 * TIME_PER_PAGE;
			}
			//�������¼�Ϊ�����ջ����֤�������
			else if (allPCB[Id].events[i].eventType == heapAlloc || allPCB[Id].events[i].eventType == stackAlloc) {
				fscanf(processfile, "%d", &allPCB[i].events[i].eventMsg.allocNum);  //��ȡҳ��
				if (allPCB[Id].events[i].eventMsg.allocNum > MAX_PAGE_NUM) {
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "Create process %d failed,apply too many pages", Id);
					ReleaseSemaphore(writeMutex, 1, NULL);
					//����ӡ��Ϣд�뵽��־�ļ���
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
	allPCB[Id].startTime = time(NULL); //��¼���̴���ʱ��
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
