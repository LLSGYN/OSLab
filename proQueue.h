#pragma once
#include "ProStruct.h"
#include "dfVar.h"
#include "killProcess.h"

void KillProFromQueue(WaitQueue* curQueue, int ID);
void AddProcessToQueue(WaitQueue* curQueue, int ID);
void UpdateEvent(int proID);