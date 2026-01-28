#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <mutex>
#include <queue>

#include "boost/algorithm/string/trim.hpp"

#include "common.hpp"
#include "debugConsole.h"

#include "logging.hpp"




constexpr unsigned LOG_BUFFER_SIZE = 128;


STATIC wchar_t pLogBuffer[ LOG_BUFFER_SIZE ];
STATIC HANDLE  hLogHandle = INVALID_HANDLE_VALUE;

STATIC std::queue<const wchar_t*>	myQueue;
STATIC std::mutex					queueMutex;

STATIC HANDLE hLogEvent{ INVALID_HANDLE_VALUE };
STATIC HANDLE hLogThread{ INVALID_HANDLE_VALUE };




DWORD WINAPI LogThread(LPVOID pParam);
STATIC DWORD bExitThreadFlag{ 0 };






STATIC void pushToQueue(const wchar_t* str)
{
	std::unique_lock<std::mutex> lock(queueMutex);
	myQueue.push(str);
}


STATIC const wchar_t*  popFromQueue()
{
	const wchar_t* val = nullptr;
	
	std::unique_lock<std::mutex> lock(queueMutex);
	if (!myQueue.empty()) {
		val = myQueue.front();
		myQueue.pop();
	}

	return val;
}






static eLogFlags gLogFlags = static_cast<eLogFlags>(0xFFFFFFFF);




void LOG_INIT()
{
	if ((hLogHandle = ::GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"Can't get handle to debug console", MB_ICONERROR | MB_OK | MB_APPLMODAL);
	}

	hLogEvent = CREATE_EVENT(NULL, FALSE, FALSE, L"hLogEvent");         // NOT manual reset, initial state

	hLogThread = CreateThread(NULL, 0, LogThread, NULL, 0, NULL);

}


void LOG_EXIT()
{
	bExitThreadFlag = 1;
	SetEvent(hLogEvent);
	WaitForSingleObject(hLogThread, 5000);
	CloseHandle(hLogThread);
	CloseHandle(hLogEvent);
}




void LOG_WRITE([[maybe_unused]] eLogFlags type, const wchar_t* format, ...)
{
	va_list args;

	va_start(args, format);
	vswprintf_s(pLogBuffer, LOG_BUFFER_SIZE-1, format, args);
	va_end(args);
	pLogBuffer[LOG_BUFFER_SIZE-1] = L'\0';

	size_t len = wcslen(pLogBuffer);
	if (len > 0)
	{
		wchar_t* newstr = ::new wchar_t[len+1]();
		wcscpy_s(newstr, len+1, pLogBuffer);
		pushToQueue(newstr);
		SetEvent(hLogEvent);
	}
}






DWORD WINAPI LogThread( [[maybe_unused]] LPVOID pParam)
{
	while (true)
	{
		DWORD dwResult = WaitForSingleObject(hLogEvent, INFINITE);
		if (dwResult == WAIT_OBJECT_0)
		{
			const wchar_t* msg = popFromQueue();
			if (msg != nullptr)
			{
				WriteDebugConsole(msg);
				::delete[] msg;
			}
		}

		if (bExitThreadFlag != 0)
			return 0;
	}
}


