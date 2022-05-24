#pragma once
#include "ProStruct.h"
#include "proQueue.h"
#include "memory.h"


void DestoryProcess(int ID);//自然销毁的进程
void KillProcess(int ID);//强制撤销的进程
DWORD WINAPI MyKill(LPVOID lpParam);//负责销毁待销毁队列中的进程