#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "Cslots.hpp"
#include "utils.hpp"
#include "logging.hpp"

#include "threadData.hpp"





extern Cslots gSlots;



cRequests::cRequests()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cRequests constructor\n");

    handles[0]   = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermRequest");     // AUTO reset, initial state
    handles[1]   = CREATE_EVENT(NULL, FALSE, FALSE, L"evRequest");         // AUTO reset, initial state
    sem_requests = CREATE_SEMAPHORE(NULL, 1, 1, L"semRequestData");

    StartThread();
}


cRequests::~cRequests()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cRequests destructor\n");

    if (m_pRequestsThread) {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cRequests Thread still running!\n");
        TerminateThread();
    }

    CloseHandle(handles[0]);    handles[0]   = INVALID_HANDLE_VALUE;
    CloseHandle(handles[1]);    handles[1]   = INVALID_HANDLE_VALUE;
    CloseHandle(sem_requests);  sem_requests = INVALID_HANDLE_VALUE;
}


void cRequests::StartThread()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cRequests StartThread()\n");

    if (m_pRequestsThread == nullptr)
    {
        m_pRequestsThread = AfxBeginThread(thrRequests, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        ASSERT(m_pRequestsThread);
        m_pRequestsThread->m_bAutoDelete = false;
        m_pRequestsThread->ResumeThread();
        SetThreadDescription(m_pRequestsThread->m_hThread, L"thrRequest");
    }
    else
    {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cRequests Thread already started!\n");
    }
}


void cRequests::TerminateThread()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cRequests TerminateThread()\n");

    if (m_pRequestsThread)
    {
        SetEvent(handles[0]);
        DWORD result = WaitForSingleObject(m_pRequestsThread->m_hThread, 5000);
        VERIFY(result == WAIT_OBJECT_0);
        delete m_pRequestsThread;
        m_pRequestsThread = nullptr;
    }
    else
    {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cRequests Thread already terminated!\n");
    }
}

bool cRequests::ThreadRunning() const
{
    return (m_pRequestsThread != nullptr);
}



void cRequests::Push(const std::string url)
{
    Lock();
    url_queue.push(url);
    Unlock();

    NotifyRequestThread();
}


std::string cRequests::Pop()
{
    std::string str;
    Lock();
    if (url_queue.empty()) {
        Unlock();
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cReguests Pop() on empty queue!\n");
    }
    else {
        str = url_queue.front();
        url_queue.pop();
        Unlock();
    }

    return str;
}


bool cRequests::Lock() const
{
    DWORD result = WaitForSingleObject(sem_requests, INFINITE);
    return CheckWaitResult(1, result);
}

bool cRequests::Unlock() const
{
    BOOL result = ReleaseSemaphore(sem_requests, 1, NULL);
    ASSERT(result);
    return result;
}




const std::vector<HANDLE>& cRequests::Handles() const
{
    return handles;
}

void cRequests::NotifyRequestThread() const             // TODO SignalRequest() ???
{
    SetEvent(handles[1]);
}

bool cRequests::RequestsPending() const
{
    Lock();
    bool bPending = !url_queue.empty();
    Unlock();
    return bPending;
}

void cRequests::ClearQueue()
{
    Lock();
    url_queue = {};
    Unlock();
}















cResults::cResults()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults constructor\n");

    sem_results   = CREATE_SEMAPHORE(NULL, 1, 1, L"semResultsData");
    HANDLE handle = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermResults");     // Auto reset, initial state
    handles.push_back(handle);

    // The gSlots global is guaranteed to be fully constructed by now. Add all the slots evResult handles.
    auto slothandles = gSlots.GetResultHandles();
    handles.insert(handles.end(), slothandles.begin(), slothandles.end());

    StartThread();
}


cResults::~cResults()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults destructor\n");

    if (m_pResultsThread) {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults Thread still running!\n");
        TerminateThread();
    }

    // We only own the evTermResults handle. The individual slots own the other handles
    CloseHandle(handles[0]);    handles[0]  = INVALID_HANDLE_VALUE;
    CloseHandle(sem_results);   sem_results = INVALID_HANDLE_VALUE;
}


void cResults::StartThread()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults StartThread()\n");

    if (m_pResultsThread == nullptr)
    {
        m_pResultsThread = AfxBeginThread(thrResults, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        ASSERT(m_pResultsThread);
        m_pResultsThread->m_bAutoDelete = false;
        m_pResultsThread->ResumeThread();
        SetThreadDescription(m_pResultsThread->m_hThread, L"thrResults");
    }
    else
    {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults Thread already started!\n");
    }
}


void cResults::TerminateThread()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults TerminateThread()\n");

    if (m_pResultsThread)
    {
        SetEvent(handles[0]);
        DWORD result = WaitForSingleObject(m_pResultsThread->m_hThread, THREAD_EXIT_TIMEOUT);
        VERIFY(result == WAIT_OBJECT_0);
        delete m_pResultsThread;
        m_pResultsThread = nullptr;
    }
    else
    {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults Thread already terminated!\n");
    }
}


bool cResults::ThreadRunning() const
{
    return (m_pResultsThread != nullptr);
}




bool cResults::Lock() const
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults::Lock Enter\n");

    DWORD result = WaitForSingleObject(sem_results, INFINITE);
    bool retval  = CheckWaitResult(1, result);

    return retval;

}

// Returns true if unlocked OK
bool cResults::Unlock() const
{
    [[ maybe_unused]] LONG last_value;

    LOG_PRINT(eLogFlags::THREAD_DATA, L"cResults::Unlock\n");

    BOOL result = ReleaseSemaphore(sem_results, 1, &last_value);

    return (result) ? true : false;
}




const std::vector<HANDLE>& cResults::Handles() const
{
    return handles;
}

const unsigned cResults::NumHandles() const
{
    return handles.size();
}

void cResults::SetMsgWindow(HWND hWin)
{
    m_hMsgWin = hWin;
}

HWND cResults::GetMsgWindow() const
{
    return m_hMsgWin;
}








cReleases::cReleases()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cReleases constructor\n");

    // Assume these Win32 Create API calls will succeed

    sem_releases = CREATE_SEMAPHORE(NULL, 1, 1, L"semReleases");
    HANDLE h     = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermReleases");     // Terminate thread, always index 0, manual reset, initial state
    handles.push_back(h);

    // The gSlots global is guaranteed to be fully constructed by now. Add all the slots evResult handles.
    auto slothandles = gSlots.GetReleaseHandles();
    handles.insert(handles.end(), slothandles.begin(), slothandles.end());

    StartThread();
}


cReleases::~cReleases()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cReleases destructor\n");

    if (m_pReleasesThread) {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cReleases Thread still running!\n");
        TerminateThread();
    }

    // We only own the evTermRelease handle. The individual slots own the other handles
    CloseHandle(handles[0]);    handles[0]   = INVALID_HANDLE_VALUE;
    CloseHandle(sem_releases);  sem_releases = INVALID_HANDLE_VALUE;
}


void::cReleases::StartThread()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"cReleases StartThread\n");

    if (m_pReleasesThread == nullptr)
    {
        m_pReleasesThread = AfxBeginThread(thrReleases, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        ASSERT(m_pReleasesThread);
        m_pReleasesThread->m_bAutoDelete = false;
        m_pReleasesThread->ResumeThread();
        SetThreadDescription(m_pReleasesThread->m_hThread, L"thrReleases");
    }
    else
    {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cReleases Thread already started!\n");
    }
}


void cReleases::TerminateThread()
{
    if (m_pReleasesThread != nullptr)
    {
        SetEvent(handles[0]);
        DWORD result = WaitForSingleObject(m_pReleasesThread->m_hThread, THREAD_EXIT_TIMEOUT);
        VERIFY(result == WAIT_OBJECT_0);
        delete m_pReleasesThread;
        m_pReleasesThread = nullptr;
    }
    else
    {
        LOG_PRINT(eLogFlags::THREAD_DATA, L"cReleases Thread already terminated!\n");
    }

}


bool cReleases::ThreadRunning() const
{
    return (m_pReleasesThread != nullptr);
}


void cReleases::ReleaseSlot(DWORD slotnum)
{
    gSlots.ResetAndFree(slotnum);
}

bool cReleases::Lock()
{
    LOG_PRINT(eLogFlags::THREAD_DATA, L"Releases::Lock Enter\n");

    DWORD result = WaitForSingleObject(sem_releases, INFINITE);
    bool retval = CheckWaitResult(1, result);

    LOG_PRINT(eLogFlags::THREAD_DATA, L"Releases::Lock Exit\n");

    return retval;
}

bool cReleases::Unlock()
{
    [[ maybe_unused]] LONG last_value;

    LOG_PRINT(eLogFlags::THREAD_DATA, L"Releases::Unlock Enter\n");

    BOOL result = ReleaseSemaphore(sem_releases, 1, &last_value);

    LOG_PRINT(eLogFlags::THREAD_DATA, L"Releases::Unlock Exit\n");
    return (result) ? true : false;
}



const std::vector<HANDLE>& cReleases::Handles() const
{
    return handles;
}

const unsigned cReleases::NumHandles() const
{
    return handles.size();
}

void cReleases::SetMsgWindow(HWND hWin)
{
    m_hMsgWin = hWin;
}

HWND cReleases::GetMsgWindow() const
{
    return m_hMsgWin;
}

