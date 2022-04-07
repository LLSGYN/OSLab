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
		printf("Fail to create new process...\n");
		return 0;
	}
	processCNT++;//����������һ
	printf("**********process totol count is %d\n", processCNT);
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

	allPCB[nowID].eventID = 0;
	allPCB[nowID].eventTime = 0;

	allPCB[nowID].pageNum = rand() % (MAX_PAGE_NUM - 2) + 3;//�������������ڴ�ҳ�� ��СΪ3
	//���ýӿں������ڴ�ģ�������ڴ�
	//********************
	//********************
	//MemoryAlloc(nowID, allPCB[nowID].pageNum, allPCB[nowID].fatherProID); //���ýӿں������ڴ�ģ�������ڴ�
	allPCB[nowID].eventNum = rand() % 1 + 1;//��������¼����� ************************** rand() % MAX_EVENT
	printf("*************event total num is %d\n", allPCB[nowID].eventNum);
	int mem_cnt = 0;//��ռ�õ��ڴ�ҳ��
	for (int i = 0; i < allPCB[nowID].eventNum; i++)
	{
		if (i == 0)
			allPCB[nowID].events[i].eventType = occupyCPU;//��һ���¼�����ȥʹ��CPU
		else
			allPCB[nowID].events[i].eventType = rand() % MAX_EVENT_TYPE;//�¼�����������������������¼�
		
		printf("*******eventType is %d\n", allPCB[nowID].events[i].eventType);
		if (fatherProcessID != -1 && allPCB[nowID].events[i].eventType == createProcess)
			allPCB[nowID].events[i].eventType = occupyCPU;//���ӽ��̵Ĵ��������¼���Ϊ�����ռ��CPU���¼�
		
		if(allPCB[nowID].events[i].eventType == createProcess)//��ǰ�¼�Ϊ���������¼�
			allPCB[nowID].events[i].time = CREATE_PROCESS_TIME;//���ø��¼���Ҫ��ʱ��Ƭ
		else if (allPCB[nowID].events[i].eventType == occupyIO)//ռ��IO�¼�
		{
			allPCB[nowID].events[i].eventMsg.IDOfIO = rand() % IO_NUM;//����IO��
			printf("************alloc IO_num is %d...\n", allPCB[nowID].events[i].eventMsg.IDOfIO);
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
		printf("**************this event needs %d time\n", allPCB[nowID].events[i].time);
	}

	allPCB[nowID].CPUtime = 0;
	allPCB[nowID].startTime = time(NULL); //��¼���̴���ʱ��
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