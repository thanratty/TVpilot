#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <mutex>
#include <queue>

#include "boost/algorithm/string/trim.hpp"

#include "common.hpp"
#include "logging.hpp"




/**
 * The control in the message Dialog window for debug/log text.
 */
STATIC CEdit* pMsgWindow = nullptr;




#if (ENABLE_CONSOLE_LOGGING==1)

constexpr unsigned LOG_BUFFER_LEN = 256;

STATIC HANDLE	hLogHandle { INVALID_HANDLE_VALUE };
STATIC HANDLE	hLogEvent  { INVALID_HANDLE_VALUE };
STATIC HANDLE	hLogThread { INVALID_HANDLE_VALUE };


STATIC std::queue<const wchar_t*>	myQueue;				// FIFO of ptrs to message strings
//STATIC std::mutex					queueMutex;				// To control Lock/Unlock access to the queue

STATIC CSemaphore*   semQueue{nullptr};
bool bConsoleReady{ false };

STATIC DWORD bExitThreadFlag{ 0 };							// Set non-zero to exit the logging thread on the next event

STATIC eLogFlags LogEnableFlags = eLogFlags::THREAD_FUNC;

#endif	// (ENABLE_CONSOLE_LOGGING==1)









#if (ENABLE_CONSOLE_LOGGING==1)

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
	
//	std::unique_lock<std::mutex> lock(queueMutex);

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
			MessageBox(NULL, L"Wait error!\n", L"Console Thread", MB_OK | MB_ICONEXCLAMATION);
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
	LOG_PRINT(eLogFlags::SYSTEM, L"Closing console\n");

	FreeConsole();
	hLogHandle = INVALID_HANDLE_VALUE;
}

#endif		// (ENABLE_CONSOLE_LOGGING==1)






#if (ENABLE_CONSOLE_LOGGING==1)

void LOG_INIT()
{
	LogConsoleCreate();

	semQueue   = new CSemaphore(1,1,L"semQueue", NULL);
	hLogEvent  = CREATE_EVENT(NULL, FALSE, FALSE, L"hLogEvent");         // NOT manual reset, initial state
	hLogThread = CreateThread(NULL, 0, LogThread, NULL, 0, NULL);

	LOG_PRINT(eLogFlags::INFO, L"Console Ready\n\n");

	return;
}


void LOG_EXIT()
{
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




