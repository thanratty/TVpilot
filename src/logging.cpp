#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <mutex>
#include <queue>

#include "boost/algorithm/string/trim.hpp"

#include "common.hpp"
#include "logging.hpp"

#include "logging.hpp"




constexpr unsigned LOG_BUFFER_LEN = 128;


STATIC wchar_t	pLogBuffer[ LOG_BUFFER_LEN ];
STATIC HANDLE	hLogHandle { INVALID_HANDLE_VALUE };
STATIC HANDLE	hLogEvent  { INVALID_HANDLE_VALUE };
STATIC HANDLE	hLogThread { INVALID_HANDLE_VALUE };


STATIC std::queue<const wchar_t*>	myQueue;				// FIFO of ptrs to message strings
STATIC std::mutex					queueMutex;				// To control Lock/Unlock access to the queue

STATIC DWORD bExitThreadFlag{ 0 };							// Set non-zero to exit the logging thread on the next event

static eLogFlags gLogFlags = static_cast<eLogFlags>(0xFFFFFFFF);








/**
 * Console logging is available in RELASE & DEBUG builds, but must
 * be specifically enabled in config.h
 */

#if (CONSOLE_LOGGING_ENABLED==1)

static void LogConsoleCreate(void);
static void LogConsoleClose(void);

#else

#define     LogConsoleCreate()        do{} while(0)
#define     LogConsoleClose()         do{} while(0)

#endif






STATIC void PushToMsgQueue(const wchar_t* str)
{
	std::unique_lock<std::mutex> lock(queueMutex);
	myQueue.push(str);
}


STATIC const wchar_t*  PopFromMsgQueue()
{
	const wchar_t* val = nullptr;
	
	std::unique_lock<std::mutex> lock(queueMutex);
	if (!myQueue.empty()) {
		val = myQueue.front();
		myQueue.pop();
	}

	return val;
}







DWORD WINAPI LogThread([[maybe_unused]] LPVOID pParam)
{
DWORD	dwCharsWritten;

	while (true)
	{
		DWORD dwResult = WaitForSingleObject(hLogEvent, INFINITE);
		if (dwResult == WAIT_OBJECT_0)
		{
			while (true)
			{
				const wchar_t* msg = PopFromMsgQueue();
				if (msg == nullptr)
					break;

				WriteConsole(hLogHandle, msg, wcslen(msg), &dwCharsWritten, NULL);
				::delete[] msg;
			}
		}

		if (bExitThreadFlag != 0)
			return 0;
	}
}











void LOG_INIT()
{
	LogConsoleCreate();

	hLogEvent  = CREATE_EVENT(NULL, FALSE, FALSE, L"hLogEvent");         // NOT manual reset, initial state
	hLogThread = CreateThread(NULL, 0, LogThread, NULL, 0, NULL);

	LOG_WRITE(eLogFlags::INFO, L"Console Ready\n\n");
	return;
}


void LOG_EXIT()
{
	bExitThreadFlag = 1;
	SetEvent(hLogEvent);
	WaitForSingleObject(hLogThread, THREAD_EXIT_TIMEOUT);
	CloseHandle(hLogThread);
	CloseHandle(hLogEvent);
}




void LOG_WRITE([[maybe_unused]] eLogFlags type, const wchar_t* format, ...)
{
	va_list args;

	va_start(args, format);
	vswprintf_s(pLogBuffer, LOG_BUFFER_LEN-1, format, args);
	va_end(args);
	pLogBuffer[LOG_BUFFER_LEN-1] = L'\0';

	size_t len = wcslen(pLogBuffer);
	if (len > 0)
	{
		wchar_t* newstr = ::new wchar_t[len+1]();
		wcsncpy(newstr, pLogBuffer, len);
		PushToMsgQueue(newstr);
		SetEvent(hLogEvent);
	}
}






/**
 * The control in the message Dialog window for debug/log text.
 */
STATIC CEdit* pMsgWindow = nullptr;


/**
 * A few routines to allow writing debug/trace messages to the seperate debug
 * window.
 */
void SetMsgWindow(CEdit* pedit)
{
	pMsgWindow = pedit;
}

void LogMsgWindow(CString& msg)
{
	if (pMsgWindow)
	{
		CString str = msg + CString(L"\r\n");
		int length = pMsgWindow->GetWindowTextLength();
		pMsgWindow->SetSel(length, length);
		pMsgWindow->ReplaceSel(str);
	}
}

void LogMsgWindow(const char* pchars)
{
	CString s(pchars);
	LogMsgWindow(s);
}

void LogMsgWindow(const wchar_t* pwchars)
{
	CString s(pwchars);
	LogMsgWindow(s);
}

void LogMsgWindow(const std::string& str)
{
	LogMsgWindow(str.c_str());
}









static void LogConsoleCreate(void)
{
	// Create a new console window
	if (0 == AllocConsole()) {
		AfxMessageBox(L"Can't create console. Logging disabled.", MB_OK);
		return;
	}


	hLogHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (INVALID_HANDLE_VALUE == hLogHandle) {
		AfxMessageBox(L"Can't get console handle. Logging disabled.", MB_OK);
		FreeConsole();
		return;
	}

	SetConsoleTitle(L"TV Pilot Console Output");
}


static void LogConsoleClose(void)
{
	LOG_WRITE(eLogFlags::SYSTEM, L"Closing console\n");

	FreeConsole();
	hLogHandle = INVALID_HANDLE_VALUE;
}


