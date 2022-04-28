#include "proMemory.h"

DWORD WINAPI MyMemoryControl(LPVOID lpParamter)
{
#ifdef DEBUG
	printf("memoryControl start...\n");
#endif
	WaitForSingleObject(contMemory, INFINITE);//�ȴ��ڴ������ɺ󣬼���Memory����
	while (1)
	{
		WaitForSingleObject(timeLockForMemory, INFINITE);
		WaitForSingleObject(proInCPUMutex, INFINITE);//��ȡ�ڴ����Ȩ�ޣ���ǿ��ɱ�����̵�ʱ����ܻ�������ź���
		WaitForSingleObject(allPCB[processInMemory].processMutex, INFINITE);

		if (processInMemory != -1)//�н�����������
		{
			int curEID = allPCB[processInMemory].eventID;//��ǰ�¼�ID
			allPCB[processInMemory].eventTime++;//�¼�����ʱ���һ
			if (allPCB[processInMemory].eventTime == allPCB[processInMemory].events[curEID].time)//��ǰ�¼�����ִ�����
			{
				if (allPCB[processInMemory].events[curEID].eventType == proReadMem)//���ڴ�
				{
					int start_page = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.startPageID;//�߼�ҳ��
					int offset = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.offset;//ҳ��ƫ����
					int len = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.len;//���ڴ�ĳ���
					//########################
					//���ö��ڴ�ӿں���
				}
				else if (allPCB[processInMemory].events[curEID].eventType == proWriteMem)//д�ڴ�
				{
					int start_page = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.startPageID;//�߼�ҳ��
					int offset = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.offset;//ҳ��ƫ����
					int len = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.len;//���ڴ�ĳ���
					//########################
					//����д�ڴ溯��
				}
				else if (allPCB[processInMemory].events[curEID].eventType == heapAlloc)//�����
				{
					allPCB[processInMemory].heapUsed += allPCB[processInMemory].events[curEID].needRAM;
#ifdef DEBUG
					printf("----Process %d heap alloc is finished...\n", processInMemory);
#endif
				}
				else if (allPCB[processInMemory].events[curEID].eventType == stackAlloc)//����ջ
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
		WaitForSingleObject(breakMemory, INFINITE); //�ȴ�Memory�ж��Խ���Memory����
		WaitForSingleObject(memoryQueue.queueFull[0], INFINITE);//�ȴ���ֱ�������������ɽ���
		ReleaseSemaphore(memoryQueue.queueFull[0], 1, NULL);//��һ�д��뽫�������е�queueFull��һ�����������¼ӻ�ȥ�������д����Ŀ����ȷ�������������н���

		WaitForSingleObject(memoryQueue.queueMutex[0], INFINITE);//��þ������еķ���Ȩ��
		int subcript = memoryQueue.head[0];//��ͷ
		processInMemory = memoryQueue.waitProcessID[0][subcript];//��ѡ�еĽ���ID
		allPCB[processInMemory].nowState = run;//�ý���״̬Ϊ����̬
		ReleaseSemaphore(memoryQueue.queueEmpty[0], 1, NULL);//�ͷŸö��еķ���Ȩ��

		ReleaseSemaphore(contMemory, 1, NULL);//������ɣ���֪ģ���ڴ��������

	}
}