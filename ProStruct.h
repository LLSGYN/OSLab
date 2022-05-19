#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable : 4996)

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<windows.h>
#include<process.h>
#include<stdbool.h>

#define MAX_NAME  256  //进程名字的最大长度
#define MAX_PROCESS 16 //最大进程数
#define MAX_EVENT 15   //一个进程包含的最大事件数
// #define MAX_PAGE_NUM 32//一个进程初始申请的最大页数 有冲突，先注释掉
#define PRIORITY_NUM 5 //优先级0--4 数字越小优先级越高
#define IO_NUM 5       //IO设备的数量
#define TIME_PIECE 50  //时间片长度，单位ms
#define MAX_PAGE_NUM 16  //最大申请页数
#define CREATE_PROCESS_TIME 2 //创建进程所需时间片
#define TIME_PER_PAGE 2  //读写一页需要两个时间片
//#define HEAP_STACK_TIME 1 //申请堆栈所需时间片
#define MAX_NEED_TIME 30  //事件所能够得到的最大时间片数
#define MAX_EVENT_TYPE 7 //用户进程事件类型最大数量
#define CREATE_PROESS_TIME 3 //创建一个进程需要的时间片个数
#define NEED_MEMORY_PERCENT 50 //事件需要内存的概率
#define MY_ALLOC_TIME 2 //申请堆栈所需的时间片个数

// #define DEBUG

//事件类型
enum EventTypes {
	occupyCPU,		//占用CPU
	occupyIO,		//占用IO
	createProcess,  //创建进程
	proWriteMem,	//写内存
	proReadMem,		//读内存
	heapAlloc,	    //申请堆
	stackAlloc,		//申请栈
	// compile,		//-------编译-------
};

//进程状态
enum ProcessStates {
	ready,	//就绪
	run,	//运行
	wait,	//等待
};

typedef struct DefineWRMemory {
	int startPageID;   //逻辑页号
	int offset;        //页内偏移量
	int len;           //读写的长度
}WRMemory;

union EventMsg {
	WRMemory wrMsg;  //读写内存的信息
	int allocNum;    //申请内存的页数
	int IDOfIO;      //对应的IO设备ID
	char filename[MAX_NAME];   //---------文件名------
};

typedef struct DefineEvent {
	enum EventTypes eventType; //事件类型
	int time;				   //事件所需的时间片数
	int needRAM;			   //占用内存大小
	union EventMsg eventMsg;   //事件信息
}Event;


//进程控制块PCB
typedef struct DefinePCB {
	char name[MAX_NAME];        //进程名
	int ID;					    //进程标识符
	int fatherProID;		    //父进程标识符 初始值为-1
	int sonPro;                 //是否是子进程 1为是 0为否
	int priority;			    //优先级
	int eventID;			    //正在进行的事件编号
	int eventTime;			    //事件已经运行的时间
	int eventNum;			    //事件总数
	Event events[MAX_EVENT];    //存储进程的所有事件
	int CPUtime;			    //记录当前占用CPU的时间片数
	time_t startTime;		    //进程创建的时间
	int IOID;				    //进程正在占用的IO号 初始值为-1
	int pageNum;			    //记录该进程申请了多少内存
	int heapUsed;			    //已经使用的堆的大小（页数）
	int stackUsed;			    //已经使用的栈的大小（页数）
	enum ProcessStates nowState;//进程状态
	HANDLE processMutex;        //每个进程都有一个互斥锁  
}PCB;

typedef struct DefineQueue {
	int waitProcessID[PRIORITY_NUM][MAX_PROCESS + 1];//记录队列中的进程ID
	int head[PRIORITY_NUM];							 //队列头
	int tail[PRIORITY_NUM];							 //队列尾
	HANDLE queueMutex[PRIORITY_NUM];				 //对于多个优先级相同的进程，就绪队列在同一时间只能被其中一个进程访问
	HANDLE queueEmpty[PRIORITY_NUM];                 //某一特定的优先级下就绪队列空余位置，初始值为MAX_PROCESS
	HANDLE queueFull[PRIORITY_NUM];                  //某一特定的优先级下就绪队列中的进程个数，初始值为0
	HANDLE totalCnt;                                 //就绪队列中进程的总数，只在优先级调度中会用到，需要判断五个优先级就绪队列中是否有进程，初始值为0
}WaitQueue;
//采用循环数组的方式来实现就绪队列，但要牺牲一个数组空间，即队头和队尾相同时表示队列为空，head=(tail+1)%MAX_PROCESS时表示队列满了