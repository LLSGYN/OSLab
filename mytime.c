#include "mytime.h"
#include "dfVar.h"

//模拟时间片的运行，暂定50ms为一个时间片
DWORD WINAPI TimeRun(LPVOID lpParamter)
{
	printf("Time run start...\n");
	while (1)
	{
		curTime++;//系统运行的时间片个数加一
		// printf("curTime : %d\n", curTime);

		ReleaseSemaphore(timeLockForCPU, 1, NULL);//给virCPU一个时间片
		ReleaseSemaphore(timeLockForMemory, 1, NULL);//给virMem一个时间片
		for (int i = 0; i < IO_NUM; i++)//给virIO时间片
			ReleaseSemaphore(timeLockForIO[i], 1, NULL);
		Sleep(TIME_PIECE);//sleep 50ms
	}
}