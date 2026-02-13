#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <mutex>
#include <queue>
#include <array>
#include <vector>

#include "boost/algorithm/string/trim.hpp"

#include "common.hpp"
#include "logging.hpp"








// Global const, referenced in the CDLogging dialog
//
std::array<sLogFlagDef, NUM_LOG_FLAGS>  log_flags {{
	{ eLogFlags::INFO,          L"Info" },
	{ eLogFlags::SLOT_THREAD,   L"Slot Thread" },
	{ eLogFlags::WM_MSGS,       L"WM_MSGS" },
	{ eLogFlags::MODEL,         L"Model" },
	{ eLogFlags::CURL,          L"Curl" },
	{ eLogFlags::XML,           L"XML" },
	{ eLogFlags::FATAL,         L"Fatal" },

	{ eLogFlags::SLOT_LOCK,     L"Slot Lock" },
	{ eLogFlags::SLOT_USE,      L"Slot Use" },
	{ eLogFlags::SYNC_OBJECTS,  L"SYNC Objects" },
	{ eLogFlags::APP_EVENT,     L"APP Events" },

	{ eLogFlags::TEST,          L"Test" },
	{ eLogFlags::CONSOLE_ECHO,  L"Console Echo" }
}};






/**
 * The control in the message Dialog window for debug/log text.
 */
STATIC CEdit* pMsgWindow = nullptr;




#if (ENABLE_CONSOLE_LOGGING==1)

constexpr unsigned LOG_BUFFER_LEN = 256;

STATIC HANDLE	hLogHandle { INVALID_HANDLE_VALUE };
STATIC HANDLE	hLogEvent  { INVALID_HANDLE_VALUE };
STATIC HANDLE	hLogThread { INVALID_HANDLE_VALUE };


STATIC std::queue<const wchar_t*>	myQueue;						// FIFO of ptrs to message strings

STATIC CSemaphore*  semQueue{nullptr};
STATIC bool			bConsoleReady{ false };
STATIC DWORD		bExitThreadFlag{ 0 };							// Set non-zero to exit the logging thread on the next event


STATIC eLogFlags	LogEnableFlags = eLogFlags::INFO  |
									 eLogFlags::TEST  |
									 eLogFlags::FATAL;

#endif	// (ENABLE_CONSOLE_LOGGING==1)




eLogFlags GetLogFlags()
{
	return LogEnableFlags;
}

void SetLogFlags(eLogFlags newflags)
{
	LogEnableFlags = newflags;
}




#if (ENABLE_CONSOLE_LOGGING==1)

STATIC bool MsgQueueIsEmpty()
{
	bool bIsEmpty;

	CSingleLock lock(semQueue);
	lock.Lock();
	bIsEmpty = myQueue.empty();
	lock.Unlock();

	return bIsEmpty;
}


STATIC void PushToMsgQueue(const wchar_t* str)
{
//	std::unique_lock<std::mutex> lock(queueMutex);

	CSingleLock lock(semQueue);
	lock.Lock();
	myQueue.push(str);
	lock.Unlock();
}


STATIC const wchar_t*  PopFromMsgQueue()
{
	const wchar_t* val = nullptr;
	
	CSingleLock lock(semQueue);
	lock.Lock();

	if (!myQueue.empty()){
		val = myQueue.front();
		myQueue.pop();
	}

	lock.Unlock();
	return val;
}

#endif		// (ENABLE_CONSOLE_LOGGING==1)




#if (ENABLE_CONSOLE_LOGGING==1)

DWORD WINAPI LogThread([[maybe_unused]] LPVOID pParam)
{
DWORD	dwCharsWritten;
const	wchar_t* msg;

	bConsoleReady = true;

	while (true)
	{
		DWORD dwResult = WaitForSingleObject(hLogEvent, INFINITE);
		if (dwResult == WAIT_OBJECT_0)
		{
			msg = PopFromMsgQueue();
			while (msg != nullptr)
			{
				WriteConsole(hLogHandle, msg, wcslen(msg), &dwCharsWritten, NULL);
				delete[] msg;

				msg = PopFromMsgQueue();
			}
		}
		else
		{
			::MessageBox(NULL, L"Wait error!\n", L"Console Thread", MB_OK | MB_ICONEXCLAMATION);
		}

		if (bExitThreadFlag != 0)
			return 0;
	}
}

#endif		// (ENABLE_CONSOLE_LOGGING==1)




#if (ENABLE_CONSOLE_LOGGING==1)

STATIC void LogConsoleCreate(void)
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


STATIC void LogConsoleClose(void)
{
	LOG_PRINT(eLogFlags::INFO, L"Closing console\n");

	FreeConsole();
	hLogHandle = INVALID_HANDLE_VALUE;
}

#endif		// (ENABLE_CONSOLE_LOGGING==1)






#if (ENABLE_CONSOLE_LOGGING==1)

void LOG_INIT()
{
	LogConsoleCreate();

	semQueue   = new CSemaphore(1,1,L"semQueue", NULL);
	hLogEvent  = CREATE_EVENT(NULL, FALSE, FALSE, L"hLogEvent");         // NOT manual reset, initial state unsignalled
	hLogThread = CreateThread(NULL, 0, LogThread, NULL, 0, NULL);

	if (hLogThread)
		SetThreadDescription(hLogThread, L"thrLogging");

	LOG_PRINT(eLogFlags::INFO, L"Console Ready\n\n");

	return;
}


void LOG_EXIT()
{
	// Wait till all current messages are printed
	while (!MsgQueueIsEmpty());

	bExitThreadFlag = 1;
	SetEvent(hLogEvent);
	WaitForSingleObject(hLogThread, THREAD_EXIT_TIMEOUT);
	
	delete semQueue;
	semQueue = nullptr;

	CloseHandle(hLogThread);
	CloseHandle(hLogEvent);
}


void LOG_PRINT( eLogFlags type, const wchar_t* format, ...)
{
	while(!bConsoleReady) SwitchToThread();

	// Can't print anything once we're shutting down. This is a task sequence issue.
	// TODO - thread manager
	if (bExitThreadFlag==1)
		return;

	// Only print enable messages
	if (flags(LogEnableFlags & type))
	{
		wchar_t* newstr = new wchar_t[ LOG_BUFFER_LEN ]();

		va_list args;
		va_start(args, format);
		vswprintf_s(newstr, LOG_BUFFER_LEN - 1, format, args);
		va_end(args);

		newstr[ LOG_BUFFER_LEN - 1 ] = L'\0';

		size_t len = wcslen(newstr);
		if (len > 0)
		{
			PushToMsgQueue(newstr);
			SetEvent(hLogEvent);
		}
	}
}

#endif		// (ENABLE_CONSOLE_LOGGING==1)






/**
 * A few routines to allow writing debug/trace messages to the seperate debug
 * window.
 */
void LogSetMsgWin(CEdit* pedit)
{
	pMsgWindow = pedit;
}




void LogMsgWin(const CString& msg)
{
	LOG_PRINT(eLogFlags::CONSOLE_ECHO, msg);

	if (pMsgWindow)
	{
		CString str = msg + CString(L"\r\n");
		int length = pMsgWindow->GetWindowTextLength();
		pMsgWindow->SetSel(length, length);
		pMsgWindow->ReplaceSel(str);
	}
}

void LogMsgWin(const wchar_t* format, ...)
{
	wchar_t newstr[ LOG_BUFFER_LEN ];

	memset(newstr, 0, LOG_BUFFER_LEN*sizeof(wchar_t));

	va_list args;
	va_start(args, format);
	vswprintf_s(newstr, LOG_BUFFER_LEN - 1, format, args);
	va_end(args);

	size_t len = wcslen(newstr);
	if (len > 0)
	{
		CString s(newstr);
		LogMsgWin(s);
	}
}

void LogMsgWin(const char* format, ...)
{
	char newstr[ LOG_BUFFER_LEN ];

	memset(newstr, 0, LOG_BUFFER_LEN);

	va_list args;
	va_start(args, format);
	vsprintf_s(newstr, LOG_BUFFER_LEN - 1, format, args);
	va_end(args);

	size_t len = strlen(newstr);
	if (len > 0)
	{
		CString str = CA2W(newstr, CP_UTF8);
		LogMsgWin(str);
	}
}

void LogMsgWin(const std::string& str)
{
	LogMsgWin(str.c_str());
}
