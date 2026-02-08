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

    void StartThread();
    void TerminateThread();
    bool ThreadRunning() const;

    void Push(const std::string url);
    std::string Pop();
    bool RequestsPending() const;
    void ClearQueue();

    void NotifyRequestThread() const;

    const std::vector<HANDLE>& Handles() const;

private:

    bool Lock() const;
    bool Unlock() const;


private:
    inline static HANDLE        sem_requests{ INVALID_HANDLE_VALUE };
    CWinThread*                 m_pRequestsThread{ nullptr };

    // Entry #0 is the terminate event, #1 is the request event
    std::vector<HANDLE>         handles = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
    std::queue<std::string>     url_queue;
};










class cReleases
{
public:
        cReleases();
        ~cReleases();

        void StartThread();
        void TerminateThread();
        bool ThreadRunning() const;

        bool Lock();
        bool Unlock();

        void ReleaseSlot(DWORD slotnum);

        const std::vector<HANDLE>& Handles() const;
        const unsigned NumHandles() const;

        void SetMsgWin(HWND hWin);
        HWND GetMsgWin() const;

    private:
        inline static HANDLE    sem_releases{ INVALID_HANDLE_VALUE };
        CWinThread*             m_pReleasesThread{ nullptr };

        DWORD                   m_last_error{ 0 };
        HWND                    m_hMsgWin{ 0 };
        std::vector<HANDLE>     handles;                    // Entry 0 is always the terminate event handle
};



