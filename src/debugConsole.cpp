#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "debugConsole.h"



#define		PRINTF_BUFFER_SIZE		256




#if (ENABLE_CONSOLE_WINDOW==1) && defined (_DEBUG)


STATIC HANDLE		hDebugConsole	= NULL;
STATIC wchar_t*		pBuffer			= nullptr;


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
		vswprintf_s(pBuffer, PRINTF_BUFFER_SIZE, format, args);
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

	SetConsoleTitle(L"TV Pilot Debug Output");
	WriteDebugConsole(L"Console Ready\n\n");

	pBuffer = new wchar_t[ PRINTF_BUFFER_SIZE ];
}


void CloseDebugConsole()
{
	if (pBuffer) {
		delete pBuffer;
		pBuffer = nullptr;
	}

	FreeConsole();
	hDebugConsole = NULL;
}


#endif


