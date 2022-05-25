#include "proMemory.h"

DWORD WINAPI MyMemoryControl(LPVOID lpParamter)
{
#ifdef DEBUG
	fprintf(logs, "----memoryControl start...\n");
#endif
	WaitForSingleObject(contMemory, INFINITE);//�ȴ��ڴ������ɺ󣬼���Memory����
	while (1)
	{
		WaitForSingleObject(timeLockForMemory, INFINITE);
		WaitForSingleObject(proInMemMutex, INFINITE);//��ȡ�ڴ����Ȩ�ޣ���ǿ��ɱ�����̵�ʱ����ܻ�������ź���

		//printf("memory test1\n");
		if (processInMemory != -1)//�н�����������
		{
			int curEID = allPCB[processInMemory].eventID;//��ǰ�¼�ID
			allPCB[processInMemory].eventTime++;//�¼�����ʱ���һ
			WaitForSingleObject(killMutex, INFINITE);
			WaitForSingleObject(allPCB[processInMemory].processMutex, INFINITE);
			ReleaseSemaphore(killMutex, 1, NULL);
			if (allPCB[processInMemory].eventTime == allPCB[processInMemory].events[curEID].time)//��ǰ�¼�����ִ�����
			{
				//printf("memory test2\n");
				if (allPCB[processInMemory].events[curEID].eventType == proReadMem)//���ڴ�
				{
					int start_page = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.startPageID;//�߼�ҳ��
					int offset = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.offset;//ҳ��ƫ����
					int len = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.len;//���ڴ�ĳ���
					//########################
					//���ö��ڴ�ӿں���
					char* memBuf = (char*)malloc(sizeof(char) * len);
					flush_tlb(allPCB[processInMemory].ID);
					read_memory(memBuf, allPCB[processInMemory].ID, start_page * PAGE_SIZE + offset, len);
					free(memBuf);
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "----Process %d read memory is finished...\n", processInMemory);
					ReleaseSemaphore(writeMutex, 1, NULL);
				}
				else if (allPCB[processInMemory].events[curEID].eventType == proWriteMem)//д�ڴ�
				{
					int start_page = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.startPageID;//�߼�ҳ��
					int offset = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.offset;//ҳ��ƫ����
					int len = allPCB[processInMemory].events[curEID].eventMsg.wrMsg.len;//���ڴ�ĳ���
					//########################
					//����д�ڴ溯��
					char* memBuf = (char*)malloc(sizeof(char) * len);
					flush_tlb(allPCB[processInMemory].ID);
					write_memory(memBuf, allPCB[processInMemory].ID, start_page * PAGE_SIZE + offset, len);
					free(memBuf);
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "----Process %d write memory is finished...\n", processInMemory);
					ReleaseSemaphore(writeMutex, 1, NULL);
				}
				else if (allPCB[processInMemory].events[curEID].eventType == heapAlloc)//�����
				{
					allPCB[processInMemory].heapUsed += allPCB[processInMemory].events[curEID].needRAM;
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "----Process %d heap alloc is finished...\n", processInMemory);
					ReleaseSemaphore(writeMutex, 1, NULL);
				}
				else if (allPCB[processInMemory].events[curEID].eventType == stackAlloc)//����ջ
				{
					allPCB[processInMemory].stackUsed += allPCB[processInMemory].events[curEID].needRAM;
					WaitForSingleObject(writeMutex, INFINITE);
					fprintf(logs, "----Process %d stack alloc is finished...\n", processInMemory);
					ReleaseSemaphore(writeMutex, 1, NULL);
				}
				ReleaseSemaphore(proInMemMutex, 1, NULL);

				ReleaseSemaphore(allPCB[processInMemory].processMutex, 1, NULL);
				UpdateEvent(processInMemory);

				ReleaseSemaphore(breakMemory, 1, NULL);
				WaitForSingleObject(contMemory, INFINITE);
			}
			else
			{
				ReleaseSemaphore(allPCB[processInMemory].processMutex, 1, NULL);
				ReleaseSemaphore(proInMemMutex, 1, NULL);
			}
		}
		else
		{
			//printf("memory test4\n");
			ReleaseSemaphore(proInMemMutex, 1, NULL);
			ReleaseSemaphore(breakMemory, 1, NULL);
			WaitForSingleObject(contMemory, INFINITE);
		}
	}
}

DWORD WINAPI DispatchMemory(LPVOID lpParamter)
{
	fprintf(logs, "----Dispatch memory start...\n");
	while (1)
	{
		WaitForSingleObject(breakMemory, INFINITE); //�ȴ�Memory�ж��Խ���Memory����
		WaitForSingleObject(memoryQueue.queueFull[0], INFINITE);//�ȴ���ֱ�������������ɽ���
		ReleaseSemaphore(memoryQueue.queueFull[0], 1, NULL);//��һ�д��뽫�������е�queueFull��һ�����������¼ӻ�ȥ�������д����Ŀ����ȷ�������������н���

		WaitForSingleObject(memoryQueue.queueMutex[0], INFINITE);//��þ������еķ���Ȩ��
		int subcript = memoryQueue.head[0];//��ͷ
		processInMemory = memoryQueue.waitProcessID[0][subcript];//��ѡ�еĽ���ID
		allPCB[processInMemory].nowState = run;//�ý���״̬Ϊ����̬
		ReleaseSemaphore(memoryQueue.queueMutex[0], 1, NULL);//�ͷŸö��еķ���Ȩ��

		ReleaseSemaphore(contMemory, 1, NULL);//������ɣ���֪ģ���ڴ��������

	}
}