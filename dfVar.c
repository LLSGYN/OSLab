#include "dfVar.h"

int processCNT = 0;							//记录被创建的进程数量
int usedProcessID[MAX_PROCESS] = { 0 };	    //进程标识符的使用情况
PCB allPCB[MAX_PROCESS];					//一个进程对应有一个PCB
int processInCPU = -1;						//正在CPU中运行的进程ID号
int processInMemory = -1;					//正在内存中运行的进程ID号
int processInIO[IO_NUM];				    //正在IO中运行的进程ID号
int CPUMode = 0;		  					//CPU工作模式
//int DispatchCPUMode = 0;  					//CPU调度的工作模式  #######没有用到
int killQueue[MAX_PROCESS];					//待删进程队列
int killHead = 0;                           //上述队列的队头
int killTail = 0;                           //上述队列的队尾
int toBeKilled[MAX_PROCESS];                //判断某一ID号的进程是否在待删进程队列中
int curTime;                                //当前时间片
WaitQueue readyQueue;						//CPU就绪队列
WaitQueue waitIOQueue[IO_NUM];				//IO就绪队列,有多个IO设备
WaitQueue memoryQueue;						//内存就绪队列

HANDLE usedProcessIDMutex;                  //进程标识符使用情况的一个锁
HANDLE contCPU;                             //CPU运行所需的信号量 初始值为0
HANDLE contIO[IO_NUM];                      //IO运行所需的信号量 初始值为0
HANDLE contMemory;                          //内存控制所需的信号量 初始值为0
HANDLE proInCPUMutex;                       //进程在CPU中运行需要的互斥量，强制删除CPU中的进程也要这个互斥量，目的是停止CPU中进程的执行
HANDLE proInIOMutex[IO_NUM];                //进程在IO中运行需要的互斥量，强制删除IO中的进程也要这个互斥量，目的是停止IO中进程的执行
HANDLE proInMemMutex;                       //进程在内存中运行需要的互斥量，强制删除内存中的进程也要这个互斥量，目的是停止内存中进程的执行
HANDLE timeLockForCPU;                      //CPU需要获取的时间片信号量 初始值为0
HANDLE timeLockForMemory;					//virMem运行需要等待的信号量
HANDLE timeLockForIO[IO_NUM];				//VirIO需要等待的信号量
HANDLE breakCPU;                            //CPU调度信号量 初始值为1
HANDLE breakIO[IO_NUM];                     //IO调度信号量 初始值为1
HANDLE breakMemory;                         //内存调度信号量 初始值为1
HANDLE killMutex;                           //强制杀死进程需要获取的信号量
HANDLE killQueueEmpty;                      //即将被杀死的进程队列的剩余空间，初始为MAX_PROCESS
HANDLE killQueueFull;                       //即将被杀死的进程队列的进程个数，初始为0
HANDLE killQueueMutex;                      //即将被杀死的进程队列的锁
