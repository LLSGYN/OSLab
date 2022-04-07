#include "proQueue.h"

//�ӵ�ǰ����������ɾ������
void KillProFromQueue(WaitQueue* curQueue, int ID)//�������ý������ڵľ������� �ý��̵�ID
{
	int priority_num = allPCB[ID].priority;//�ý��̵����ȼ�
	int pos;//�ý����ھ��������е�λ��
	for (int i = curQueue->head[priority_num]; i != curQueue->tail[priority_num]; i = (i + 1) % MAX_PROCESS)//�Ҵ�ɾ�����ھ��������е�λ�ã��Ӷ���ͷ��ʼ����
	{
		if (curQueue->waitProcessID[priority_num][i] == ID)
		{
			pos = i;
			break;
		}
	}

	WaitForSingleObject(curQueue->queueMutex[priority_num], INFINITE);//�������Ĳ���Ҫ�޸ľ������У���ֹͬһ���ȼ��Ķ������ͬʱ�޸ĸþ�������

	for (int i = pos; i != curQueue->head[priority_num]; i = (i - 1 + MAX_PROCESS) % MAX_PROCESS)//����ɾ����ǰ��Ľ���λ�������
		curQueue->waitProcessID[priority_num][i] = curQueue->waitProcessID[priority_num][(i - 1 + MAX_PROCESS) % MAX_PROCESS];

	curQueue->head[priority_num] = (curQueue->head[priority_num] + 1) % MAX_PROCESS;//��ǰ���еĶ�ͷ�����һ��λ��

	ReleaseSemaphore(curQueue->queueEmpty[priority_num], 1, NULL);//����ռ��һ
	WaitForSingleObject(curQueue->queueFull[priority_num], 0);//�����н�������һ
	WaitForSingleObject(curQueue->totalCnt, 0);//�����н���������һ

	ReleaseSemaphore(curQueue->queueMutex[priority_num], 1, NULL);//�޸���ϣ��ͷŸû�����

	printf("-----Remove process %d successfully.\n", ID);//debug��Ϣ����ʱ����Ļ�ϴ�ӡ�������䵽log�ļ���
}

void AddProcessToQueue(WaitQueue* curQueue, int ID)//�����̼��뵽ָ������������
{
	int priority_num = allPCB[ID].priority;//�ý��̵����ȼ�

	WaitForSingleObject(curQueue->queueEmpty[priority_num], INFINITE);//��ȡ����ռ�
	WaitForSingleObject(curQueue->queueMutex[priority_num], INFINITE);//��ȡ�����ȼ��µľ������е�ʹ��Ȩ

	int pos = curQueue->tail[priority_num];//��β
	curQueue->waitProcessID[priority_num][pos] = ID;//���ý�����ӽ�ȥ
	pos = (pos + 1) % MAX_PROCESS;//��β����
	curQueue->tail[priority_num] = pos;

	ReleaseSemaphore(curQueue->queueFull[priority_num], 1, NULL);//�����ж���һ������
	ReleaseSemaphore(curQueue->totalCnt, 1, NULL);//�����н���������һ
	ReleaseSemaphore(curQueue->queueMutex[priority_num], 1, NULL);//�޸���ϣ��ͷŸû�����

	printf("-----Add process %d successfully.\n", ID);//debug��Ϣ,��ʱ����Ļ�ϴ�ӡ�������䵽log�ļ���
}

//��һ���¼����ʱ�����ý��̴ӵ�ǰ���ڵľ�������ɾ���������뵽��һ�¼���ָ��ľ���������
void UpdateEvent(int proID)
{
	if (proID == -1)//ǿ��ɱ�����̿��ܻ�����ID��Ϊ-1
		return;
	printf("-----Process %d, job %d finish\n", proID, allPCB[proID].eventID);//debug��Ϣ
	if (allPCB[proID].eventID < allPCB[proID].eventNum - 1)//���̻����¼�ûִ��
	{
		int preEtype = allPCB[proID].events[allPCB[proID].eventID].eventType;//��ִ�н������¼�����
		allPCB[proID].eventID++;//����Ҫִ�е��¼�ID
		int curEtype = allPCB[proID].events[allPCB[proID].eventID].eventType;//������Ҫִ�е��¼�����
		allPCB[proID].eventID--;//��ʱ�˻ص��ս������¼�ID��Ŀ���ǲ�Ӱ����������ִ��
		if (preEtype == curEtype)//��������ͬ�����ô�ԭ����ɾ��
		{
			allPCB[proID].eventID++;//�¼�ID�Ÿ���
			allPCB[proID].eventTime = 0;//��ǰ�¼����е�ʱ��Ϊ0
			allPCB[proID].nowState = wait;//�ȴ�CPU���µ���
			return;//���½����¼�����
		}
	}
	//��ԭ������ɾ���˽���
	if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyIO)//��IO�������
	{
		int curQueueID = allPCB[proID].events[allPCB[proID].eventID].eventMsg.IDOfIO;
		printf("-----Move process %d from queue IO %d.\n", proID, curQueueID);
		KillProFromQueue(&waitIOQueue[curQueueID], proID);//��IO�����������Ƴ�
	}
	else if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyCPU || allPCB[proID].events[allPCB[proID].eventID].eventType == createProcess || allPCB[proID].events[allPCB[proID].eventID].eventType == compile)
	{
		//��CPU�������
		printf("-----Move process %d from queue CPU.\n", proID);
		KillProFromQueue(&readyQueue, proID);//ͬ��

	}
	else//���ڴ�������
	{
		printf("-----Move process %d from queue Memory.\n", proID);
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
		printf("-----Process %d, job %d start\n", proID, allPCB[proID].eventID);
		if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyIO)//��IO�������
		{
			int curQueueID = allPCB[proID].events[allPCB[proID].eventID].eventMsg.IDOfIO;//��ǰ�¼�Ҫʹ�õ�IO�豸��
			printf("-----Insert process %d into queue IO %d.\n", proID, curQueueID);
			AddProcessToQueue(&waitIOQueue[curQueueID], proID);//��������ӵ��뵱ǰ�¼���صľ���������
			allPCB[proID].nowState = wait;//��Ҫ��ȡIO��Դ����˵�ǰ״̬Ϊ�ȴ�
		}
		else if (allPCB[proID].events[allPCB[proID].eventID].eventType == occupyCPU || allPCB[proID].events[allPCB[proID].eventID].eventType == createProcess || allPCB[proID].events[allPCB[proID].eventID].eventType == compile)
		{
			//��CPU�������
			printf("-----Insert process %d into queue CPU.\n", proID);
			AddProcessToQueue(&readyQueue, proID);//ͬ��
			allPCB[proID].nowState = ready;//����CPU��صľ���������״̬Ϊ����

		}
		else//���ڴ�������
		{
			printf("-----Insert process %d into queue Memory.\n", proID);
			AddProcessToQueue(&memoryQueue, proID);//ͬ��
			allPCB[proID].nowState = wait;//��Ҫ��ȡ�ڴ�ķ���Ȩ�޵���Դ����˵�ǰ״̬Ϊ�ȴ�
		}
	}
}