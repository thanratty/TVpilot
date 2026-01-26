#pragma once

#include <vector>
#include <queue>


/**
 * This class is instantiated in the DownloadManager and passed to the theRequests thread.
 * 
 */
class cRequests
{
public:

    cRequests();
    ~cRequests();

    const std::vector<HANDLE>& Handles() const;

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
    inline static HANDLE        sem_requests{ INVALID_HANDLE_VALUE };
    CWinThread*                 m_pRequestsThread{ nullptr };

    // Entry #0 is the terminate event, #1 is the request event
    std::vector<HANDLE>         handles = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
    std::queue<std::string>     url_queue;
};








/**
 * Data & methods for the thrResults thread.
 *
 */

class cResults
{
public:

    cResults();
    ~cResults();

    void SetMsgWindow(HWND hWin);
    HWND GetMsgWindow() const;

    bool Lock() const;
    bool Unlock() const;

    const std::vector<HANDLE>& Handles() const;
    const unsigned NumHandles() const;


private:

    void TerminateThread();

private:
    inline static HANDLE    sem_results;
    CWinThread*             m_pResultsThread{ nullptr };

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

    const std::vector<HANDLE>& Handles() const;
    const unsigned NumHandles() const;

    void SetMsgWindow(HWND hWin);
    HWND GetMsgWindow() const;


private:

    void TerminateThread();

private:
    inline static HANDLE    sem_releases{ INVALID_HANDLE_VALUE };
    CWinThread*             m_pReleasesThread{ nullptr };

    DWORD                   m_last_error{ 0 };
    HWND                    m_hMsgWin{ 0 };
    std::vector<HANDLE>     handles;                    // Entry 0 is the terminate event
};

