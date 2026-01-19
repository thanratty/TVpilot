#pragma once

#include "pch.h"

#include <queue>
#include <array>
#include <string>

#include "debugConsole.h"
#include "Cslot.hpp"
#include <threadfuncs.hpp>


extern std::vector<Cslot>  gSlots;





class cRequests
{
public:

    cRequests()
    {
        handles[0]      = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermRequest");     // AUTO reset, initial state
        handles[1]      = CREATE_EVENT(NULL, FALSE, FALSE, L"evRequest");         // AUTO reset, initial state

        sem_requests    = CREATE_SEMAPHORE(NULL, 1, 1, L"semRequestData");

        m_pWinThread = AfxBeginThread(thrRequests, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        ASSERT(m_pWinThread);
        m_pWinThread->m_bAutoDelete = false;
        m_pWinThread->ResumeThread();
        SetThreadDescription(m_pWinThread->m_hThread, L"thrRequest");
    }

    ~cRequests()
    {
        // Terminate the requests handler thread
        SetEvent(handles[0]);

        DWORD result = WaitForSingleObject(m_pWinThread->m_hThread, 5000);
        VERIFY(result == WAIT_OBJECT_0);

        CloseHandle(handles[0]);
        CloseHandle(handles[1]);

        delete m_pWinThread;
        m_pWinThread = nullptr;

        CloseHandle(sem_requests);
    }





    inline const HANDLE* Handles() const
    {
        return handles.data();
    }

    inline bool Pending() const
    {
        Lock();
        bool retval = !url_queue.empty();
        Unlock();
        return retval;
    }

    inline void ClearQueue()
    {
        Lock();
        url_queue = {};
        Unlock();
    }

    inline void Trigger() const
    {
        SetEvent(handles[1]);
    }

    inline void Push(const std::string url)
    {
        Lock();
        url_queue.push(url);
        Unlock();
        Trigger();
    }

    // Pop off the top of the FIFO
    std::string Pop()
    {
        std::string str;
        if (url_queue.empty()) {
            WriteDebugConsole(L"cReguestData Pop() on empty queue!");
        }
        else {
            Lock();
            str = url_queue.front();
            url_queue.pop();
            Unlock();
        }
        return str;
    }


private:

    bool Lock() const
    {
        DWORD result = WaitForSingleObject(sem_requests, INFINITE);
        return CheckWaitResult(1, result);
    }

    // This should be private & handled internally?
    bool Unlock() const
    {
        BOOL result = ReleaseSemaphore(sem_requests, 1, NULL);
        ASSERT(result);
        return result;
    }



private:
    HANDLE                      m_evRequest{ INVALID_HANDLE_VALUE };
    HANDLE                      sem_requests{ INVALID_HANDLE_VALUE };

    CWinThread*                 m_pWinThread{ nullptr };

    std::array<HANDLE,2>        handles;                // Entry 0 is the terminate event, 1 is the request event
    std::queue<std::string>     url_queue;
};
















// Results data struct doesn't change, so no semaphore needed
class cResults
{
public:

    cResults()
    {
        sem_results = CREATE_SEMAPHORE(NULL, 1, 1, L"semResultsData");

        HANDLE handle = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermResults");     // Auto reset, initial state
        handles.push_back(handle);

        // The gSlots global is guaranteed to be fully constructed by now.
        for (const Cslot& slot : gSlots)
            handles.push_back(slot.m_hEvResult);

        m_pWinThread = AfxBeginThread(thrResults, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        ASSERT(m_pWinThread);
        m_pWinThread->m_bAutoDelete = false;
        m_pWinThread->ResumeThread();
        SetThreadDescription(m_pWinThread->m_hThread, L"thrResults");
    }


    ~cResults()
    {
        SetEvent(handles[0]);
        DWORD result = WaitForSingleObject(m_pWinThread->m_hThread, 5000);
        VERIFY( result == WAIT_OBJECT_0 );
        std::for_each(handles.begin(), handles.end(), [](const HANDLE& h) { CloseHandle(h); });

        delete m_pWinThread;
        m_pWinThread = nullptr;

        CloseHandle(sem_results);
    }



    inline void SetMsgWindow(HWND hWin)
    {
        m_hMsgWin = hWin;
    }

    inline HWND GetMessageWindow() const
    {
        return m_hMsgWin;
    }

    inline BOOL TerminateThread() const
    {
        return ::SetEvent(handles[0]);
    }

    // Returns true if locked OK
    bool Lock() const
    {
        WriteDebugConsole(L"Results::Lock Enter\n");
        DWORD result = WaitForSingleObject(sem_results, INFINITE);
        WriteDebugConsole(L"Results::Lock Exit\n");

        return CheckWaitResult(1, result);
    }

    // Returns true if unlocked OK
    bool Unlock() const
    {
        [[ maybe_unused]] LONG last_value;

        WriteDebugConsole(L"Results::Unock Enter\n");
        BOOL result = ReleaseSemaphore(sem_results, 1, &last_value);
        VERIFY(result);
        WriteDebugConsole(L"Results::Unlock Exit\n");

        return (result) ? true : false;
    }

    inline const HANDLE* Handles() const
    {
        return handles.data();
    }

    inline const unsigned NumHandles() const
    {
        return handles.size();
    }



private:
    inline static HANDLE    sem_results;

    CWinThread*             m_pWinThread{ nullptr };
    HWND                    m_hMsgWin{ NULL };
    DWORD                   m_last_error{ 0 };
    std::vector<HANDLE>     handles;                    // Entry 0 is the terminate event, the rest are each slot's result event
};






class cReleases
{
public:
    cReleases()
    {
        HANDLE handle;

        // It's safe to assume these Win32 Create API calls will succeed

        sem_releases = CREATE_SEMAPHORE(NULL, 1, 1, L"semReleases");

        handle = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermReleases");     // Terminate thread, always index 0, manual reset, initial state
        handles.push_back(handle);

        for (const Cslot& slot : gSlots)
            handles.push_back(slot.m_hEvRelease);

        m_pWinThread = AfxBeginThread(thrReleases, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        ASSERT(m_pWinThread);
        m_pWinThread->m_bAutoDelete = false;
        m_pWinThread->ResumeThread();
        SetThreadDescription(m_pWinThread->m_hThread, L"thrResults");
    }


    ~cReleases()
    {
        SetEvent(handles[0]);
        DWORD result = WaitForSingleObject(m_pWinThread->m_hThread, 5000);
        VERIFY(result == WAIT_OBJECT_0);
        std::for_each(handles.begin(), handles.end(), [](const HANDLE& h) { CloseHandle(h); });

        delete m_pWinThread;
        m_pWinThread = nullptr;

        CloseHandle(sem_releases);
        CloseHandle(handles[0]);
    }


    inline BOOL Terminate() const
    {
        return SetEvent(handles[0]);
    }

    inline void ReleaseSlot(DWORD slotnum)
    {
        gSlots.at(slotnum).ResetAndFree();
    }

    bool Lock()
    {
        WriteDebugConsole(L"Releases::Lock Enter\n");
        DWORD result = WaitForSingleObject(sem_releases, INFINITE);
        WriteDebugConsole(L"Releases::Lock Exit\n");

        return CheckWaitResult(1, result);
    }

    bool Unlock()
    {
        [[ maybe_unused]] LONG last_value;

        WriteDebugConsole(L"Releases::Unlock Enter\n");
        BOOL result = ReleaseSemaphore(sem_releases, 1, &last_value);
        ASSERT(result);
        WriteDebugConsole(L"Releases::Unlock Exit\n");

        return result;
    }

    inline const HANDLE* Handles() const
    {
        return handles.data();
    }

    inline const unsigned NumHandles() const
    {
        return handles.size();
    }



private:
    CWinThread*             m_pWinThread{ nullptr };
    HANDLE                  sem_releases{ INVALID_HANDLE_VALUE };
    DWORD                   m_last_error{ 0 };
    std::vector<HANDLE>     handles;                    // Entry 0 is the terminate event
};

