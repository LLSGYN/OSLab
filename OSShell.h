#pragma once

#include "processStauts.h"
#include "createProcess.h"
#include "killProcess.h"
#include "fsapi.h"

extern void shell();
extern int string2Int(char string[], int start);
extern int substr(char string[], int start, char **substring);