#pragma once
#include "ProStruct.h"
#include "proQueue.h"
#include "memory.h"


void DestoryProcess(int ID);//��Ȼ���ٵĽ���
void KillProcess(int ID);//ǿ�Ƴ����Ľ���
DWORD WINAPI MyKill(LPVOID lpParam);//�������ٴ����ٶ����еĽ���