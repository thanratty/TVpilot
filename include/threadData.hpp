#pragma once

#include "CsyncObjects.hpp"
#include "threadFuncs.hpp"
#include "debugConsole.h"





class cRequests
{
public:

    cRequests();
    ~cRequests();

    const HANDLE* Handles() const;

    bool RequestsPending() const;
    void ClearQueue();

    void Push(const std::string url);
    std::string Pop();
    void NotifyRequestThread() const;


private:

    bool Lock() const;
    bool Unlock() const;
    void TerminateThread();


private:
    HANDLE                      m_evRequest{ INVALID_HANDLE_VALUE };
    HANDLE                      sem_requests{ INVALID_HANDLE_VALUE };

    CWinThread*                 m_pWinThread{ nullptr };    // The thrRequests() thread

    std::array<HANDLE, 2>       handles;                    // Entry #0 is the terminate event, #1 is the request event
    std::queue<std::string>     url_queue;
};
















// Results data struct doesn't change, so no semaphore needed
class cResults
{
public:

    cResults();
    ~cResults();

    void SetMsgWindow(HWND hWin);
    HWND GetMsgWindow() const;

    bool Lock() const;
    bool Unlock() const;

    const HANDLE* Handles() const;
    const unsigned NumHandles() const;


private:

    void TerminateThread();

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
    cReleases();
    ~cReleases();

    void ReleaseSlot(DWORD slotnum);

    bool Lock();
    bool Unlock();

    const HANDLE* Handles() const;
    const unsigned NumHandles() const;

    void SetMsgWindow(HWND hWin);
    HWND GetMsgWindow() const;


private:

    void TerminateThread();

private:
    CWinThread*             m_pWinThread{ nullptr };
    HANDLE                  sem_releases{ INVALID_HANDLE_VALUE };
    DWORD                   m_last_error{ 0 };
    HWND                    m_hMsgWin{ 0 };
    std::vector<HANDLE>     handles;                    // Entry 0 is the terminate event
};

