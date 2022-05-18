#include "processStauts.h"

void showAllProcess();
void showSingleProcess(int pid);

void showAllProcess()
{
	for (int i = 0; i < MAX_PROCESS; i++)
	{
		WaitForSingleObject(allPCB[i].processMutex, INFINITE);
		if (usedProcessID[i])
			showSingleProcess(i);
		ReleaseSemaphore(allPCB[i].processMutex, 1, NULL);
	}
}

void showSingleProcess(int pid)
{
	PCB pcb = allPCB[pid];
	printf("---------PROCESS %d---------\n", pcb.ID);
	printf("name: %s\n", pcb.name);
	if (pcb.fatherProID != -1)
		printf("fatherProID: %d\n", pcb.fatherProID);
	printf("priority: %d\n", pcb.priority);
	printf("nowState: %d\n", pcb.nowState);
	printf("eventID: %d\n", pcb.eventID);
	printf("eventTime: %d\n", pcb.eventTime);
	printf("eventNum: %d\n", pcb.eventNum);
	printf("all events:\n");
	for (int i = 0; i < pcb.eventNum; i++)
	{
        printf("\tevent %d : ", i);
        Event event = pcb.events[i];
        switch (event.eventType)
        {
          case occupyCPU:
            printf("occupyCPU  time: %d  needRAM: %d\n", event.time, event.needRAM);
            break;
          case occupyIO:
            printf("occupyIO  time: %d  needRAM: %d  IDofIO: %d\n", event.time, event.needRAM, event.eventMsg.IDOfIO);
            break;
          case createProcess:
            printf("createProcess  time: %d  needRAM: %d\n", event.time, event.needRAM);
            break;
          case proReadMem:
            printf("proReadMemory  time: %d  needRAM: %d  startPageID: %d  pageNum: %d\n", event.time, event.needRAM, event.eventMsg.wrMsg.startPageID, event.eventMsg.allocNum);
            break;
          case proWriteMem:
            printf("proWriteMemory  time: %d  needRAM: %d  startPageID: %d  pageNum: %d\n", event.time, event.needRAM, event.eventMsg.wrMsg.startPageID, event.eventMsg.allocNum);
            break;
          case heapAlloc:
            printf("heapAlloc  time: %d  needRAM: %d  pageNum: %d\n", event.time, event.needRAM, event.eventMsg.allocNum);
            break;
          case stackAlloc:
            printf("stackAlloc  time: %d  needRAM: %d  pageNum: %d\n", event.time, event.needRAM, event.eventMsg.allocNum);
            break;
          default:
            printf("event type error!\n");
            break;
        }
	}
}