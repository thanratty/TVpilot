#pragma once

#include "pch.h"

#include <queue>
#include <string>

#include "CslotData.hpp"





class cRequestData
{
public:
    cRequestData(CslotData* _pslots);
    ~cRequestData();

    inline BOOL TerminateThread()
    {
        return ::SetEvent(handles[0]);
    }

    bool SemLock() const
    {
        ASSERT(sem_request_data != nullptr);
        DWORD result = WaitForSingleObject(sem_request_data, INFINITE);
        return CheckWaitResult(1, result);
    }

    bool SemUnlock() const
    {
        LONG last_value;
        BOOL result = ReleaseSemaphore(sem_request_data, 1, &last_value);
        ASSERT(result);
        return result;
    }

    inline void Trigger() const
    {
        SetEvent(handles[1]);
    }

    inline CslotData* Slots() const
    {
        return pslots;
    }

    inline const HANDLE* Handles() const
    {
        return handles.data();
    }

    inline bool Pending() const
    {
        return !url_queue.empty();
    }

    inline void Push(const std::string url)
    {
        url_queue.push(url);
    }

    inline void ClearQueue()
    {
        url_queue = {};
    }


    std::string Pop()
    {
        std::string str;
        if (url_queue.empty()) {
            AfxDebugBreak();
        }
        else {
            str = url_queue.front();
            url_queue.pop();
        }
        return str;
    }

private:
    inline static HANDLE        sem_request_data;

    std::vector<HANDLE>         handles;                // Entry 0 is the terminate event
    std::queue<std::string>     url_queue;
    CslotData*                  pslots{ nullptr };      // Ptr to the actual slot array
    DWORD                       m_last_error{ 0 };
};















// Results data struct doesn't change, so no semaphore needed
class cResultsData
{
public:
    cResultsData(CslotData* _pslots);
    ~cResultsData();


    inline void SetMsgWindow(HWND hWin)
    {
        hMessageWindow = hWin;
    }

    inline BOOL TerminateThread() const
    {
        return ::SetEvent(handles[0]);
    }

    // Returns true if locked OK
    bool SemLock() const
    {
        DWORD result = WaitForSingleObject(sem_results_data, INFINITE);
        return CheckWaitResult(1, result);
    }

    // Returns true if unlocked OK
    bool SemUnlock() const
    {
        LONG last_value;
        BOOL result = ReleaseSemaphore(sem_results_data, 1, &last_value);
        VERIFY(result);
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

    inline HWND GetMessageWindow() const
    {
        return hMessageWindow;
    }

    inline CslotData* Slots() const
    {
        return pslots;
    }


private:
    inline static HANDLE    sem_results_data;

    HWND                    hMessageWindow{ NULL };
    DWORD                   m_last_error{ 0 };
    std::vector<HANDLE>     handles;                    // Entry 0 is the terminate event
    CslotData*              pslots{ nullptr };          // Ptr to the actual slot array
};






class cReleaseData
{
public:
    cReleaseData(CslotData* _pslots);
    ~cReleaseData();


    inline BOOL TerminateThread() const
    {
        return SetEvent(handles[0]);
    }

    inline void ResetAndFree(DWORD index)
    {
        pslots[index].ResetAndFree();
    }

    bool SemLock()
    {
        DWORD result = WaitForSingleObject(sem_release_data, INFINITE);
        return CheckWaitResult(1, result);
    }

    bool SemUnlock()
    {
        [[ maybe_unused]] LONG last_value;
        BOOL result = ReleaseSemaphore(sem_release_data, 1, &last_value);
        ASSERT(result);
        return result;
    }

    inline const HANDLE* Handles() const
    {
        return handles.data();
    }

    inline CslotData* Slots() const
    {
        return pslots;
    }

private:
    // Only >ONE< copy of sem_release_data among all instances
    inline static HANDLE    sem_release_data;

    CslotData*              pslots{ nullptr };          // Ptr to the actual slot array
    DWORD                   m_last_error{ 0 };
    std::vector<HANDLE>     handles;                    // Entry 0 is the terminate event
};

