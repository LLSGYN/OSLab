#include "killProcess.h"
#include "dfVar.h"
#include "fsapi.h"

void DestoryProcess(int ID)//��Ȼ���ٵĽ���ID
{
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		if (allPCB[i].fatherProID == ID)//��Ϊ��ɾ���̵��ӽ���
		{
			memory_free(i);
			allPCB[i].pageNum = 0;
		}
	}
	WaitForSingleObject(killQueueMutex, INFINITE);//��ȡ��ɾ���̶��е�Ȩ��
	allPCB[ID].fatherProID = -1;

	if (toBeKilled[ID] == 0)//���ý�����δ���ڴ�ɾ���̶�����
	{
		WaitForSingleObject(killQueueEmpty, INFINITE);//�ȴ���ֱ���ö����п���ռ�
		toBeKilled[ID] = 1;//���ı�־λ
		killQueue[killTail] = ID;//���ý��̷ŵ��˶�����
		killTail = (killTail + 1) % MAX_PROCESS;//��β����
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----Process %d will be killed!\n", ID);//��ӡdebug��Ϣ
		ReleaseSemaphore(writeMutex, 1, NULL);
		ReleaseSemaphore(killQueueFull, 1, NULL);//�ö����н�������һ
	}

	ReleaseSemaphore(killQueueMutex, 1, NULL);//�ͷŸö��еķ���Ȩ��
}

void KillProcess(int ID)//ǿ������ָ���Ľ���
{
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "----start kill process %d...\n", ID);
	ReleaseSemaphore(writeMutex, 1, NULL);
	WaitForSingleObject(killMutex, INFINITE);//���ǿ�����ٽ��̵�Ȩ��
	WaitForSingleObject(allPCB[ID].processMutex, INFINITE);
	int nowEID = allPCB[ID].eventID;//�����ٽ����������е��¼���
	int nowEtype = allPCB[ID].events[nowEID].eventType;//�����ٽ����������е��¼�������
	switch (nowEtype)
	{
	case createProcess:
	case occupyCPU:
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----kill process is in CPU queue...\n");//��ɾ���̴���CPU��������
		ReleaseSemaphore(writeMutex, 1, NULL);
		WaitForSingleObject(proInCPUMutex, INFINITE);//��ֹCPU������
		if (processInCPU == ID)//����ɾ������������
		{
			processInCPU = -1;//ֹͣ����
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "----CPU released...\n");
			ReleaseSemaphore(writeMutex, 1, NULL);
		}
		ReleaseSemaphore(killMutex, 1, NULL);//�ͷ�ǿ�����ٽ��̵�Ȩ��
		KillProFromQueue(&readyQueue, ID);//���ý��̴Ӿ�������ɾ��
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "----remove process %d from readyQueue...\n", ID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		ReleaseSemaphore(proInCPUMutex, 1, NULL);//�ͷ�CPU�����ź���
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
		ReleaseSemaphore(killMutex, 1, NULL);//�ͷŸ�Ȩ��
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
		ReleaseSemaphore(killMutex, 1, NULL);//�ͷŸ�Ȩ��
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
		for (int i = 0; i < MAX_PROCESS; i++)
		{
			WaitForSingleObject(allPCB[i].processMutex, INFINITE);
			if (allPCB[i].fatherProID == killID)//��Ϊ��ɾ���̵��ӽ���
			{
				ReleaseSemaphore(allPCB[i].processMutex, 1, NULL);
				KillProcess(i);//ǿ��ɾ���ӽ���
			}
			else
				ReleaseSemaphore(allPCB[i].processMutex, 1, NULL);
		}
		WaitForSingleObject(allPCB[killID].processMutex, INFINITE);
		int thisFather = allPCB[killID].fatherProID;//��ɾ���̵ĸ�����ID
		ReleaseSemaphore(allPCB[killID].processMutex, 1, NULL);
		if (thisFather != -1)//���ý����и�����
		{
			WaitForSingleObject(allPCB[killID].processMutex, INFINITE);
			allPCB[killID].fatherProID = 0;//�Ͼ����ӹ�ϵ
			ReleaseSemaphore(allPCB[killID].processMutex, 1, NULL);
		}
		//�ͷ�������ڴ�ռ�
		if (allPCB[killID].pageNum)
			memory_free(killID);
		usedProcessID[killID] = 0;//���˽��̱�ʶ��ID��Ϊ����
		processCNT--;//����������һ
		toBeKilled[killID] = 0;//��ǰID���Ѿ����ڴ�ɾ������
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "\nINFO:process %d is killed!\n", killID);//debug��Ϣ
		ReleaseSemaphore(writeMutex, 1, NULL);
		printf("\nINFO:process %d is killed!\n", killID);//debug��Ϣ
		printf("root@OS-LAB:%s# ", printwd());
	}
}
