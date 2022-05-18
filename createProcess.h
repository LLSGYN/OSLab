#pragma once
#include "dfVar.h"
#include "ProStruct.h"
#include "proQueue.h"
#include "memory.h"

int GetNextUnusedProcessID();
int CreateMyProcess(char* processName, int fatherProcessID);
int CreateMyDiyProcess(char* processName, int fatherProcessID, char* processFileName);
// int CreateCompileProcess(char* fileName);

