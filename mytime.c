#include "mytime.h"
#include "dfVar.h"

//ģ��ʱ��Ƭ�����У��ݶ�50msΪһ��ʱ��Ƭ
DWORD WINAPI TimeRun(LPVOID lpParamter)
{
	printf("Time run start...\n");
	while (1)
	{
		curTime++;//ϵͳ���е�ʱ��Ƭ������һ
		// printf("curTime : %d\n", curTime);

		ReleaseSemaphore(timeLockForCPU, 1, NULL);//��virCPUһ��ʱ��Ƭ
		ReleaseSemaphore(timeLockForMemory, 1, NULL);//��virMemһ��ʱ��Ƭ
		for (int i = 0; i < IO_NUM; i++)//��virIOʱ��Ƭ
			ReleaseSemaphore(timeLockForIO[i], 1, NULL);
		Sleep(TIME_PIECE);//sleep 50ms
	}
}