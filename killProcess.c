#include "killProcess.h"
#include "dfVar.h"

void DestoryProcess(int ID)//��Ȼ���ٵĽ���ID
{
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		if (allPCB[i].fatherProID == ID)//��Ϊ��ɾ���̵��ӽ���
		{
			KillProcess(i);//ǿ��ɾ���ӽ���
		}
	}
	WaitForSingleObject(killQueueMutex, INFINITE);//��ȡ��ɾ���̶��е�Ȩ��

	if (toBeKilled[ID] == 0)//���ý�����δ���ڴ�ɾ���̶�����
	{
		WaitForSingleObject(killQueueEmpty, INFINITE);//�ȴ���ֱ���ö����п���ռ�
		toBeKilled[ID] = 1;//���ı�־λ
		killQueue[killTail] = ID;//���ý��̷ŵ��˶�����
		killTail = (killTail + 1) % MAX_PROCESS;//��β����
		printf("----Process %d will be killed!\n", ID);//��ӡdebug��Ϣ
		ReleaseSemaphore(killQueueFull, 1, NULL);//�ö����н�������һ
	}

	ReleaseSemaphore(killQueueMutex, 1, NULL);//�ͷŸö��еķ���Ȩ��
}

void KillProcess(int ID)//ǿ������ָ���Ľ���
{
	printf("start kill process %d...\n", ID);
	WaitForSingleObject(killMutex, INFINITE);//���ǿ�����ٽ��̵�Ȩ��
	int nowEID = allPCB[ID].eventID;//�����ٽ����������е��¼���
	int nowEtype = allPCB[ID].events[nowEID].eventType;//�����ٽ����������е��¼�������
	switch (nowEtype)
	{
	case createProcess:
	case compile:
	case occupyCPU:
	{
		printf("kill process is in CPU queue...\n");//��ɾ���̴���CPU��������
		WaitForSingleObject(proInCPUMutex, INFINITE);//��ֹCPU������
		if (processInCPU == ID)//����ɾ������������
		{
			processInCPU = -1;//ֹͣ����
			printf("----CPU released...\n");
		}
		ReleaseSemaphore(killMutex, 1, NULL);//�ͷ�ǿ�����ٽ��̵�Ȩ��
		KillProFromQueue(&readyQueue, ID);//���ý��̴Ӿ�������ɾ��
		printf("----remove process %d from readyQueue...\n", ID);
		ReleaseSemaphore(proInCPUMutex, 1, NULL);//�ͷ�CPU�����ź���
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
		ReleaseSemaphore(killMutex, 1, NULL);//�ͷŸ�Ȩ��
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
		ReleaseSemaphore(killMutex, 1, NULL);//�ͷŸ�Ȩ��
		KillProFromQueue(&memoryQueue, ID);
		printf("----remove process %d from memoryQueue...\n", ID);
		ReleaseSemaphore(proInMemMutex, 1, NULL);
		break;
	}
	default:
		break;
	}
	
	DestoryProcess(ID);//���ý��̼����ɾ���̶���
}

DWORD WINAPI MyKill(LPVOID lpParam)
{
	while (1)
	{
		WaitForSingleObject(killQueueFull, INFINITE);//�ȴ���ֱ����ɾ���̶������н���
		//printf("lzy123456\n");
		WaitForSingleObject(killQueueMutex, INFINITE);//��ȡ�ö��з���Ȩ��
		int killID = killQueue[killHead];//��ȡ����ͷ����ID
		killHead = (killHead + 1) % MAX_PROCESS;//����ͷ����
		ReleaseSemaphore(killQueueEmpty, 1, NULL);//�ö���ʣ��ռ��һ
		ReleaseSemaphore(killQueueMutex, 1, NULL);//�ͷŸö��з���Ȩ��
		int thisFather = allPCB[killID].fatherProID;//��ɾ���̵ĸ�����ID
		if (thisFather != -1)//���ý����и�����
			allPCB[killID].fatherProID = 0;//�Ͼ����ӹ�ϵ
		//�ͷ�������ڴ�ռ�
		//*****************
		//MemoryRelease(ID);
		usedProcessID[killID] = 0;//���˽��̱�ʶ��ID��Ϊ����
		processCNT--;//����������һ
		toBeKilled[killID] = 0;//��ǰID���Ѿ����ڴ�ɾ������
		printf("\nINFO:process %d is killed!\n", killID);//debug��Ϣ
	}
}
