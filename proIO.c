#include "proIO.h"

DWORD WINAPI virtualIO(LPVOID paramter);
DWORD WINAPI ioScheduling(LPVOID paramter);

DWORD WINAPI virtualIO(LPVOID paramter)
{
	int ID = *((int*)paramter);
	printf("IO %d start...\n", ID);
	printf("Scheduling strategy is FCFS...\n");

	WaitForSingleObject(contIO[ID], INFINITE);
	while (1)
	{
		int pid = processInIO[ID];
		WaitForSingleObject(timeLockForIO[ID], INFINITE); // ��ȡʱ��Ƭ
		printf("*************get time successfully.\n");
		WaitForSingleObject(proInIOMutex[ID], INFINITE); // ��ȡIO�豸����Ȩ�ޣ���ǿ��ɱ�����̵�ʱ����ܻ�������ź���

		if (pid != -1) // ��ǰIO�豸�н�����ռ��
		{
			WaitForSingleObject(killMutex, INFINITE); // ��֤���½�����Ϣʱ���ᱻǿ��ɱ��
			WaitForSingleObject(allPCB[pid].processMutex, INFINITE); // ��ȡ�����޸�Ȩ��
			ReleaseSemaphore(killMutex, 1, NULL); // �ͷ�killMUtex
			int curEID = allPCB[pid].eventID; // ��ǰ���ڽ��е��¼����
			allPCB[pid].eventTime++;
			if (allPCB[pid].eventTime == allPCB[pid].events[curEID].time) // ��ǰ�¼�ִ�����
			{
				ReleaseSemaphore(proInIOMutex[ID], 1, NULL); // �ͷ�IO�豸����Ȩ���ź�������UpdateEvent֮ǰ�ͷţ���ֹ��������
				// WaitForSingleObject(killMutex, INFINITE); // ��֤�����¼���Ϣ��ʱ��ý��̲����Ա�ǿ������
				UpdateEvent(pid); // ���´˽��̵��¼���Ϣ
				allPCB[pid].IOID = -1; // IO�豸ʹ�����
				// ReleaseSemaphore(killMutex, 1, NULL); // �ͷ�killMutex
				printf("IO finished\n");
				ReleaseSemaphore(allPCB[pid].processMutex, 1, NULL); // �ͷŽ��̹���Ȩ��
				ReleaseSemaphore(breakIO[ID], 1, NULL); // �ȴ�IO�豸���е��ȹ���
				WaitForSingleObject(contIO[ID], INFINITE); // IO�豸��ɵ��ȹ���
			}
			else
			{
				// �¼�δ��ɣ�����ѭ��
				ReleaseSemaphore(proInIOMutex[ID], 1, NULL);
				// ReleaseSemaphore(allPCB[processInIO[ID]].processMutex, 1, NULL);
			}
		}
		else
		{
			ReleaseSemaphore(proInIOMutex[ID], 1, NULL); // �ͷ�IO�豸����Ȩ���ź���
			ReleaseSemaphore(breakIO[ID], 1, NULL); // IO�жϣ��ȴ�IO����
			WaitForSingleObject(contIO[ID], INFINITE); // �ȴ�IO������ɺ󣬼�������IO�豸
		}
	}
}

DWORD WINAPI ioScheduling(LPVOID paramter)
{
	int ID = *((int*)paramter);
	printf("Dispatch IO %d start...\n", ID);

	while (1)
	{
		WaitForSingleObject(breakIO[ID], INFINITE); // �ȴ�IO�ж��Խ���IO����
		WaitForSingleObject(waitIOQueue[ID].queueFull[0], INFINITE); // �ȴ�IO�����г��ֽ���
		ReleaseSemaphore(waitIOQueue[ID].queueFull[0], 1, NULL); // ��full�ź���-1����ʾ��һ�����̴ӵȴ����з����IO�豸

		WaitForSingleObject(waitIOQueue[ID].queueMutex[0], INFINITE); // ��ȡ�ȴ����еķ���Ȩ��
		
		int nextPro = waitIOQueue[ID].head[0]; // ���п�ʼλ��
		processInIO[ID] = nextPro;
		allPCB[processInIO[ID]].IOID = ID;

		ReleaseSemaphore(waitIOQueue[ID].queueMutex[0], 1, NULL);
		ReleaseSemaphore(contIO[ID], 1, NULL);
	}
}