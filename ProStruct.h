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

#define MAX_NAME  256  //�������ֵ���󳤶�
#define MAX_PROCESS 16 //��������
#define MAX_EVENT 15   //һ�����̰���������¼���
// #define MAX_PAGE_NUM 32//һ�����̳�ʼ��������ҳ�� �г�ͻ����ע�͵�
#define PRIORITY_NUM 5 //���ȼ�0--4 ����ԽС���ȼ�Խ��
#define IO_NUM 5       //IO�豸������
#define TIME_PIECE 50  //ʱ��Ƭ���ȣ���λms
#define MAX_PAGE_NUM 16  //�������ҳ��
#define CREATE_PROCESS_TIME 2 //������������ʱ��Ƭ
#define TIME_PER_PAGE 2  //��дһҳ��Ҫ����ʱ��Ƭ
//#define HEAP_STACK_TIME 1 //�����ջ����ʱ��Ƭ
#define MAX_NEED_TIME 30  //�¼����ܹ��õ������ʱ��Ƭ��
#define MAX_EVENT_TYPE 7 //�û������¼������������
#define CREATE_PROESS_TIME 3 //����һ��������Ҫ��ʱ��Ƭ����
#define NEED_MEMORY_PERCENT 50 //�¼���Ҫ�ڴ�ĸ���
#define MY_ALLOC_TIME 2 //�����ջ�����ʱ��Ƭ����

// #define DEBUG

//�¼�����
enum EventTypes {
	occupyCPU,		//ռ��CPU
	occupyIO,		//ռ��IO
	createProcess,  //��������
	proWriteMem,	//д�ڴ�
	proReadMem,		//���ڴ�
	heapAlloc,	    //�����
	stackAlloc,		//����ջ
	// compile,		//-------����-------
};

//����״̬
enum ProcessStates {
	ready,	//����
	run,	//����
	wait,	//�ȴ�
};

typedef struct DefineWRMemory {
	int startPageID;   //�߼�ҳ��
	int offset;        //ҳ��ƫ����
	int len;           //��д�ĳ���
}WRMemory;

union EventMsg {
	WRMemory wrMsg;  //��д�ڴ����Ϣ
	int allocNum;    //�����ڴ��ҳ��
	int IDOfIO;      //��Ӧ��IO�豸ID
	char filename[MAX_NAME];   //---------�ļ���------
};

typedef struct DefineEvent {
	enum EventTypes eventType; //�¼�����
	int time;				   //�¼������ʱ��Ƭ��
	int needRAM;			   //ռ���ڴ��С
	union EventMsg eventMsg;   //�¼���Ϣ
}Event;


//���̿��ƿ�PCB
typedef struct DefinePCB {
	char name[MAX_NAME];        //������
	int ID;					    //���̱�ʶ��
	int fatherProID;		    //�����̱�ʶ�� ��ʼֵΪ-1
	int sonPro;                 //�Ƿ����ӽ��� 1Ϊ�� 0Ϊ��
	int priority;			    //���ȼ�
	int eventID;			    //���ڽ��е��¼����
	int eventTime;			    //�¼��Ѿ����е�ʱ��
	int eventNum;			    //�¼�����
	Event events[MAX_EVENT];    //�洢���̵������¼�
	int CPUtime;			    //��¼��ǰռ��CPU��ʱ��Ƭ��
	time_t startTime;		    //���̴�����ʱ��
	int IOID;				    //��������ռ�õ�IO�� ��ʼֵΪ-1
	int pageNum;			    //��¼�ý��������˶����ڴ�
	int heapUsed;			    //�Ѿ�ʹ�õĶѵĴ�С��ҳ����
	int stackUsed;			    //�Ѿ�ʹ�õ�ջ�Ĵ�С��ҳ����
	enum ProcessStates nowState;//����״̬
	HANDLE processMutex;        //ÿ�����̶���һ��������  
}PCB;

typedef struct DefineQueue {
	int waitProcessID[PRIORITY_NUM][MAX_PROCESS + 1];//��¼�����еĽ���ID
	int head[PRIORITY_NUM];							 //����ͷ
	int tail[PRIORITY_NUM];							 //����β
	HANDLE queueMutex[PRIORITY_NUM];				 //���ڶ�����ȼ���ͬ�Ľ��̣�����������ͬһʱ��ֻ�ܱ�����һ�����̷���
	HANDLE queueEmpty[PRIORITY_NUM];                 //ĳһ�ض������ȼ��¾������п���λ�ã���ʼֵΪMAX_PROCESS
	HANDLE queueFull[PRIORITY_NUM];                  //ĳһ�ض������ȼ��¾��������еĽ��̸�������ʼֵΪ0
	HANDLE totalCnt;                                 //���������н��̵�������ֻ�����ȼ������л��õ�����Ҫ�ж�������ȼ������������Ƿ��н��̣���ʼֵΪ0
}WaitQueue;
//����ѭ������ķ�ʽ��ʵ�־������У���Ҫ����һ������ռ䣬����ͷ�Ͷ�β��ͬʱ��ʾ����Ϊ�գ�head=(tail+1)%MAX_PROCESSʱ��ʾ��������