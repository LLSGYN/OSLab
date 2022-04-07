#include "CPU.h"
#include "createProcess.h"
#include "dfVar.h"
#include "proQueue.h"

/*CPUMode Ĭ��Ϊ0 ���Ȳ���ΪFCFS
              Ϊ1 ���Ȳ���Ϊ����ռ�ľ�̬���ȼ�
              Ϊ2 ���Ȳ���ΪRR
*/

//ģ��CPU������
DWORD WINAPI VirCPU(LPVOID lpParamter)
{
	printf("CPU start...\n");
	if (CPUMode == 0)
		printf("CPU mode is FCFS...\n");
	else if (CPUMode == 1)
		printf("CPU mode is non-preemptive priority-based...\n");
	else if (CPUMode == 2)
		printf("CPU mode is RR...\n");
	else
	{
		printf("CPU mode error...\n");
		exit(0);
	}
	WaitForSingleObject(contCPU, INFINITE);//�ȴ�CPU������ɣ�����CPU������
	printf("****************get contCPU.\n");
	while (1)
	{
		WaitForSingleObject(timeLockForCPU, INFINITE);//��ȡʱ��Ƭ
		printf("*************get time successfully.\n");
		WaitForSingleObject(proInCPUMutex, INFINITE);//��ȡCPU����Ȩ�ޣ���ǿ��ɱ�����̵�ʱ����ܻ�������ź���

		if (processInCPU != -1)//�н�������ִ��
		{
			int curEID = allPCB[processInCPU].eventID;//��ǰ���ڽ��е��¼����
			allPCB[processInCPU].CPUtime++;//�ý���ռ�õ�CPUʱ������
			allPCB[processInCPU].eventTime++;//�ý��̵�ǰ�¼����е�ʱ������
			if (allPCB[processInCPU].eventTime == allPCB[processInCPU].events[curEID].time)//��ǰ�¼�ִ����ɣ���Ҫ�л�Ϊ��һ���¼�
			{
				if (allPCB[processInCPU].events[allPCB[processInCPU].eventID].eventType == createProcess) //���¼�����Ϊ�����½���
				{
					char name[MAX_NAME] = "A new process";//�½�����
					//���ô������̺������д���
					//############################
					CreateMyProcess(name, processInCPU);//��������
				}
				else if (allPCB[processInCPU].events[allPCB[processInCPU].eventID].eventType == compile)//���¼�����Ϊ�ļ���صĲ���
				{
					//������Ӧ�Ĳ���
					//############################
				}
				ReleaseSemaphore(proInCPUMutex, 1, NULL);//�ͷ�CPU����Ȩ���ź�������UpdateEvent֮ǰ�ͷţ���ֹ��������

				WaitForSingleObject(killMutex, INFINITE);//��֤���½�����Ϣ��ʱ��ý��̲����Ա�ǿ������
				UpdateEvent(processInCPU);//���´˽��̵��¼���Ϣ
				ReleaseSemaphore(killMutex, 1, NULL);//�ͷ�killMutex

				ReleaseSemaphore(breakCPU, 1, NULL);//CPU�жϣ��ȴ�CPU���µ���
				WaitForSingleObject(contCPU, INFINITE);//�ȴ�CPU������ɺ󣬼���CPU������
			}
			else//��ǰ�¼�δִ�����
			{
				/*
				�������������ΪRR���ȣ��򽫸ý����滻
				����ΪRR������Ҫ���»�ȡ�ź�������ִ��
				*/
				if (CPUMode == 0 || CPUMode == 1)//FCFS�����ȼ�
				{
					//�ղ���
				}
				else if (CPUMode == 2)//RR
				{
					printf("----Move process %d from queue CPU.\n", processInCPU);
					KillProFromQueue(&readyQueue, processInCPU);//�����̴ӵ�ǰ����ɾ��
					printf("----Insert process %d into queue CPU.\n", processInCPU);
					AddProcessToQueue(&readyQueue, processInCPU);//�����̼ӵ���ǰ����β��
					allPCB[processInCPU].nowState = ready;

					ReleaseSemaphore(breakCPU, 1, NULL);//CPU�жϣ��ȴ�CPU���µ���
					WaitForSingleObject(contCPU, INFINITE);//�ȴ�CPU������ɺ󣬼���CPU������
				}

				ReleaseSemaphore(proInCPUMutex, 1, NULL);//�ͷ�CPU����Ȩ���ź���
			}
		}
		else//û�н�������ִ��
		{
			ReleaseSemaphore(proInCPUMutex, 1, NULL);//�ͷ�CPU����Ȩ���ź���
			ReleaseSemaphore(breakCPU, 1, NULL);//CPU�жϣ��ȴ�CPU���µ���
			WaitForSingleObject(contCPU, INFINITE);//�ȴ�CPU������ɺ󣬼���CPU������
		}
	}
}

//ģ��CPU�ĵ���
DWORD WINAPI DispatchCPU(LPVOID lpParamter)
{
	printf("Dispatch CPU start...\n");
	if (CPUMode == 0 || CPUMode == 2)//FCFS RR,���ȼ�Ĭ��Ϊ0,���ȴ��ڶ�ͷ�Ľ�������
	{
		while (1)
		{
			WaitForSingleObject(breakCPU, INFINITE);//�ȴ�CPU�ж��Խ���CPU����
			WaitForSingleObject(readyQueue.queueFull[0], INFINITE);//�ȴ���ֱ�������������н���
			ReleaseSemaphore(readyQueue.queueFull[0], 1, NULL);//��һ�д��뽫�������е�queueFull��һ�����������¼ӻ�ȥ�������д����Ŀ����ȷ�������������н���
			
			WaitForSingleObject(readyQueue.queueMutex[0], INFINITE);//��þ������еķ���Ȩ��
			int subcript = readyQueue.head[0];//��ͷ
			processInCPU = readyQueue.waitProcessID[0][subcript];//��ѡ�еĽ���ID
			allPCB[processInCPU].nowState = run;//�ý���״̬Ϊ����̬
			ReleaseSemaphore(readyQueue.queueMutex[0], 1, NULL);//�ͷŸþ������еķ���Ȩ��

			ReleaseSemaphore(contCPU, 1, NULL);//������ɣ���֪CPU��������
			printf("**************CPU dispatch successfully...\n");

		}
	}
	else if (CPUMode == 1)//����ռ�ľ�̬���ȼ�
	{
		while (1)
		{
			WaitForSingleObject(breakCPU, INFINITE);//�ȴ�CPU�ж��Խ���CPU����

			WaitForSingleObject(readyQueue.totalCnt, INFINITE);//�ȴ���ֱ�������������н���
			ReleaseSemaphore(readyQueue.totalCnt, 1, NULL);//�����д����Ŀ����ȷ�������������н���

			for (int i = 0; i < PRIORITY_NUM; i++)
			{
				WaitForSingleObject(readyQueue.queueMutex[i], INFINITE);//��ȡ��ǰ���ȼ��¾������е�ʹ��Ȩ
				if (readyQueue.head[i] == readyQueue.tail[i])//��ǰ���ȼ��µľ�������Ϊ��
					ReleaseSemaphore(readyQueue.queueMutex[i], 1, NULL);//�ͷŷ���Ȩ�ޣ����ŷ�����һ������
				else
				{
					int subcript = readyQueue.head[i];//��ͷ
					processInCPU = readyQueue.waitProcessID[i][subcript];//��ѡ�еĽ���ID
					allPCB[processInCPU].nowState = run;//�ý���״̬��Ϊ����̬
					ReleaseSemaphore(readyQueue.queueMutex[i], 1, NULL);//�ͷŸþ������еķ���Ȩ��

					ReleaseSemaphore(contCPU, 1, NULL);//������ɣ���֪CPU��������
					break;
				}
			}
		}
	}
}