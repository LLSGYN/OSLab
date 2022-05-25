#include "proQueue.h"

//�ӵ�ǰ����������ɾ������
void KillProFromQueue(WaitQueue* curQueue, int ID)//�������ý������ڵľ������� �ý��̵�ID
{
	int priority_num = allPCB[ID].priority;//�ý��̵����ȼ�
	if (allPCB[ID].events[allPCB[ID].eventID].eventType != occupyCPU && allPCB[ID].events[allPCB[ID].eventID].eventType != createProcess)
		priority_num = 0;//�����̵��¼�����CPU��ص�ʱ�򣬲��۽�����û�����ȼ����ý��̶������ȼ�Ϊ0����ؾ���������
	int pos = -1;//�ý����ھ��������е�λ��
	for (int i = curQueue->head[priority_num]; i != curQueue->tail[priority_num]; i = (i + 1) % MAX_PROCESS)//�Ҵ�ɾ�����ھ��������е�λ�ã��Ӷ���ͷ��ʼ����
	{
		if (curQueue->waitProcessID[priority_num][i] == ID)
		{
			pos = i;
			break;
		}
	}

	//printf("queue test1\n");
	WaitForSingleObject(curQueue->queueMutex[priority_num], INFINITE);//�������Ĳ���Ҫ�޸ľ������У���ֹͬһ���ȼ��Ķ������ͬʱ�޸ĸþ�������
	//printf("queue test2\n");
	if (pos != -1)
	{
		for (int i = pos; i != curQueue->head[priority_num]; i = (i - 1 + MAX_PROCESS) % MAX_PROCESS)//����ɾ����ǰ��Ľ���λ�������
			curQueue->waitProcessID[priority_num][i] = curQueue->waitProcessID[priority_num][(i - 1 + MAX_PROCESS) % MAX_PROCESS];

		curQueue->head[priority_num] = (curQueue->head[priority_num] + 1) % MAX_PROCESS;//��ǰ���еĶ�ͷ�����һ��λ��

		ReleaseSemaphore(curQueue->queueEmpty[priority_num], 1, NULL);//����ռ��һ
		WaitForSingleObject(curQueue->queueFull[priority_num], 0);//�����н�������һ
		WaitForSingleObject(curQueue->totalCnt, 0);//�����н���������һ

	}
	ReleaseSemaphore(curQueue->queueMutex[priority_num], 1, NULL);//�޸���ϣ��ͷŸû�����
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "-----Remove process %d successfully.\n", ID);//debug��Ϣ����ʱ����Ļ�ϴ�ӡ�������䵽log�ļ���
	ReleaseSemaphore(writeMutex, 1, NULL);

}

void AddProcessToQueue(WaitQueue* curQueue, int ID)//�����̼��뵽ָ������������
{
	int priority_num = allPCB[ID].priority;//�ý��̵����ȼ�
	if (allPCB[ID].events[allPCB[ID].eventID].eventType != occupyCPU && allPCB[ID].events[allPCB[ID].eventID].eventType != createProcess)
		priority_num = 0;//�����̵��¼�����CPU��ص�ʱ�򣬲��۽�����û�����ȼ����ý��̶������ȼ�Ϊ0����ؾ���������

	WaitForSingleObject(curQueue->queueEmpty[priority_num], INFINITE);//��ȡ����ռ�
	WaitForSingleObject(curQueue->queueMutex[priority_num], INFINITE);//��ȡ�����ȼ��µľ������е�ʹ��Ȩ

	int pos = curQueue->tail[priority_num];//��β
	curQueue->waitProcessID[priority_num][pos] = ID;//���ý�����ӽ�ȥ
	pos = (pos + 1) % MAX_PROCESS;//��β����
	curQueue->tail[priority_num] = pos;

	ReleaseSemaphore(curQueue->queueFull[priority_num], 1, NULL);//�����ж���һ������
	ReleaseSemaphore(curQueue->totalCnt, 1, NULL);//�����н���������һ
	ReleaseSemaphore(curQueue->queueMutex[priority_num], 1, NULL);//�޸���ϣ��ͷŸû�����

	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "-----Add process %d successfully.\n", ID);//debug��Ϣ,��ʱ����Ļ�ϴ�ӡ�������䵽log�ļ���
	ReleaseSemaphore(writeMutex, 1, NULL);
}

//��һ���¼����ʱ�����ý��̴ӵ�ǰ���ڵľ�������ɾ���������뵽��һ�¼���ָ��ľ���������
void UpdateEvent(int proID)
{
	if (proID == -1)//ǿ��ɱ�����̿��ܻ�����ID��Ϊ-1
		return;
	WaitForSingleObject(writeMutex, INFINITE);
	fprintf(logs, "-----Process %d, job %d finish\n", proID, allPCB[proID].eventID);//debug��Ϣ
	ReleaseSemaphore(writeMutex, 1, NULL);
	// if (allPCB[proID].eventID < allPCB[proID].eventNum - 1)//���̻����¼�ûִ��
	// {
	// 	int preEtype = allPCB[proID].events[allPCB[proID].eventID].eventType;//��ִ�н������¼�����
	// 	allPCB[proID].eventID++;//����Ҫִ�е��¼�ID
	// 	int curEtype = allPCB[proID].events[allPCB[proID].eventID].eventType;//������Ҫִ�е��¼�����
	// 	allPCB[proID].eventID--;//��ʱ�˻ص��ս������¼�ID��Ŀ���ǲ�Ӱ����������ִ��
	// 	if (preEtype == curEtype)//��������ͬ�����ô�ԭ����ɾ��
	// 	{
	// 		allPCB[proID].eventID++;//�¼�ID�Ÿ���
	// 		allPCB[proID].eventTime = 0;//��ǰ�¼����е�ʱ��Ϊ0
	// 		allPCB[proID].nowState = wait;//�ȴ�CPU���µ���
	// 		return;//���½����¼�����
	// 	}
	// }
	//��ԭ������ɾ���˽���
	if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyIO)//��IO�������
	{
		int curQueueID = allPCB[proID].events[allPCB[proID].eventID].eventMsg.IDOfIO;
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Move process %d from queue IO %d.\n", proID, curQueueID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		KillProFromQueue(&waitIOQueue[curQueueID], proID);//��IO�����������Ƴ�
	}
	else if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyCPU || allPCB[proID].events[allPCB[proID].eventID].eventType == createProcess)
	{
		//��CPU�������
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Move process %d from queue CPU.\n", proID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		KillProFromQueue(&readyQueue, proID);//ͬ��

	}
	else//���ڴ�������
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Move process %d from queue Memory.\n", proID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		KillProFromQueue(&memoryQueue, proID);//ͬ��
	}
	allPCB[proID].eventID++;//���ڽ��е��¼���ż�1������һ�¼�����
	allPCB[proID].eventTime = 0;//��ǰ�¼����е�ʱ��Ϊ0
	if (allPCB[proID].eventID == allPCB[proID].eventNum)//���̵�ȫ���¼����н���
	{
		DestoryProcess(proID);//������Ȼ����
	}
	else//���̻�δ����
	{
		WaitForSingleObject(writeMutex, INFINITE);
		fprintf(logs, "-----Process %d, job %d start\n", proID, allPCB[proID].eventID);
		ReleaseSemaphore(writeMutex, 1, NULL);
		if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyIO)//��IO�������
		{
			int curQueueID = allPCB[proID].events[allPCB[proID].eventID].eventMsg.IDOfIO;//��ǰ�¼�Ҫʹ�õ�IO�豸��
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "-----Insert process %d into queue IO %d.\n", proID, curQueueID);
			ReleaseSemaphore(writeMutex, 1, NULL);
			AddProcessToQueue(&waitIOQueue[curQueueID], proID);//��������ӵ��뵱ǰ�¼���صľ���������
			allPCB[proID].nowState = wait;//��Ҫ��ȡIO��Դ����˵�ǰ״̬Ϊ�ȴ�
		}
		else if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyCPU || allPCB[proID].events[allPCB[proID].eventID].eventType == createProcess)
		{
			//��CPU�������
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "-----Insert process %d into queue CPU.\n", proID);
			ReleaseSemaphore(writeMutex, 1, NULL);
			AddProcessToQueue(&readyQueue, proID);//ͬ��
			allPCB[proID].nowState = ready;//����CPU��صľ���������״̬Ϊ����

		}
		else//���ڴ�������
		{
			WaitForSingleObject(writeMutex, INFINITE);
			fprintf(logs, "-----Insert process %d into queue Memory.\n", proID);
			ReleaseSemaphore(writeMutex, 1, NULL);
			AddProcessToQueue(&memoryQueue, proID);//ͬ��
			allPCB[proID].nowState = wait;//��Ҫ��ȡ�ڴ�ķ���Ȩ�޵���Դ����˵�ǰ״̬Ϊ�ȴ�
		}
	}
}