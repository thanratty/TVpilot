#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "debugConsole.h"



#define		CONSOLE_BUFFER_SIZE		1024




#if (ENABLE_CONSOLE_WINDOW==1) && defined (_DEBUG)


STATIC HANDLE		hDebugConsole	= INVALID_HANDLE_VALUE;
STATIC wchar_t		pBuffer[ CONSOLE_BUFFER_SIZE ];


void WriteDebugConsole(const wchar_t* str)
{
DWORD dwCharsWritten;

	if (hDebugConsole)
	{
		int len = wcslen(str);
		WriteConsole(hDebugConsole, str, len, &dwCharsWritten, NULL);
	}
}


void DebugConsolePrintf(const wchar_t* format, ...)
{
	// This should be allocated by now, but just in case.....
	if (pBuffer)
	{
		va_list args;
		va_start(args, format);
		vswprintf_s(pBuffer, CONSOLE_BUFFER_SIZE, format, args);
		WriteDebugConsole(pBuffer);
		va_end(args);
	}
}


void OpenDebugConsole()
{
	// Create a new console window
	if (0 == AllocConsole()) {
		AfxMessageBox(L"Can't create debug console. Debug output disabled.", MB_OK);
		return;
	}

	hDebugConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (INVALID_HANDLE_VALUE == hDebugConsole) {
		AfxMessageBox(L"Can't get debug console handle. Debug output disabled.", MB_OK);
		FreeConsole();
		return;
	}

	SetConsoleTitle(L"TV Pilot Console Output");
	WriteDebugConsole(L"Console Ready\n\n");
}


void CloseDebugConsole()
{
	FreeConsole();
	hDebugConsole = INVALID_HANDLE_VALUE;
}


#endif


