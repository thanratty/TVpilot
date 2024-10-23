#pragma once

#include "config.h"


#if (ENABLE_CONSOLE_WINDOW==1) && defined(_DEBUG)


void OpenDebugConsole();
void CloseDebugConsole();

void WriteDebugConsole(const wchar_t* str);
void DebugConsolePrintf(const wchar_t* format, ...);


#else


#define     OpenDebugConsole()          do{} while(0)
#define     CloseDebugConsole()         do{} while(0)

#define     WriteDebugConsole(x)        do{} while(0)
#define     DebugConsolePrintf(x,...)          do{} while(0)


#endif

