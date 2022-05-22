#pragma once
#include "ProStruct.h"
extern int processCNT; 						//记录被创建的进程数量
extern int usedProcessID[MAX_PROCESS];	    //进程标识符的使用情况
extern PCB allPCB[MAX_PROCESS];					//一个进程对应有一个PCB
extern int processInCPU;						//正在CPU中运行的进程ID号
extern int processInMemory;					//正在内存中运行的进程ID号
extern int processInIO[IO_NUM];				    //正在IO中运行的进程ID号
extern int CPUMode;		  					//CPU工作模式
//int DispatchCPUMode = 0;  					//CPU调度的工作模式  #######没有用到
extern int killQueue[MAX_PROCESS];					//待删进程队列
extern int killHead;                           //上述队列的队头
extern int killTail;                           //上述队列的队尾
extern int toBeKilled[MAX_PROCESS];                //判断某一ID号的进程是否在待删进程队列中
extern int curTime;                                //当前时间片
extern WaitQueue readyQueue;						//CPU就绪队列
extern WaitQueue waitIOQueue[IO_NUM];				//IO就绪队列,有多个IO设备
extern WaitQueue memoryQueue;						//内存就绪队列

extern HANDLE usedProcessIDMutex;                  //进程标识符使用情况的一个锁
extern HANDLE contCPU;                             //CPU运行所需的信号量 初始值为0
extern HANDLE contIO[IO_NUM];                      //IO运行所需的信号量 初始值为0
extern HANDLE contMemory;                          //内存控制所需的信号量 初始值为0
extern HANDLE proInCPUMutex;                       //进程在CPU中运行需要的互斥量，强制删除CPU中的进程也要这个互斥量，目的是停止CPU中进程的执行
extern HANDLE proInIOMutex[IO_NUM];                //进程在IO中运行需要的互斥量，强制删除IO中的进程也要这个互斥量，目的是停止IO中进程的执行
extern HANDLE proInMemMutex;                       //进程在内存中运行需要的互斥量，强制删除内存中的进程也要这个互斥量，目的是停止内存中进程的执行
extern HANDLE timeLockForCPU;                      //CPU需要获取的时间片信号量 初始值为0
extern HANDLE timeLockForMemory;					//virMem运行需要等待的信号量
extern HANDLE timeLockForIO[IO_NUM];				//VirIO需要等待的信号量
extern HANDLE breakCPU;                            //CPU调度信号量 初始值为1
extern HANDLE breakIO[IO_NUM];                     //IO调度信号量 初始值为1
extern HANDLE breakMemory;                         //内存调度信号量 初始值为1
extern HANDLE killMutex;                           //强制杀死进程需要获取的信号量
extern HANDLE killQueueEmpty;                      //即将被杀死的进程队列的剩余空间，初始为MAX_PROCESS
extern HANDLE killQueueFull;                       //即将被杀死的进程队列的进程个数，初始为0
extern HANDLE killQueueMutex;                      //即将被杀死的进程队列的锁
extern HANDLE printMutex;						   //输出锁，防止输出乱序
extern HANDLE writeMutex;
extern FILE* logs;