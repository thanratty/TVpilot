#pragma once

#include <vector>
#include <string>

#include "threadData.hpp"
#include "CslotData.hpp"
#include "debugConsole.h"



 

class CdownloadManager
{
public:
    // Delete copy constructor
    CdownloadManager(const CdownloadManager&) = delete;

    CdownloadManager();
    ~CdownloadManager();

    void SetMsgWindow(HWND hMsgWindow);
    void DownloadShow(const std::string& url);
    bool DownloadInProgress() const;
    void AbortDownload();


    void ReleaseSlot(DWORD slotnum)
    {
        // Notify the release task to free the slot
        if (SetEvent(m_slots[slotnum].evRelease) == 0)
            WriteDebugConsole(L"Can't set slot evRelease");

        // There's a free slot - check for queued requests
        // TODO The release event wont be handled yet will it?
        if (request_data.Pending())
            request_data.Trigger();
    }

    inline CslotData& GetSlot(DWORD slotnum)
    {
        ASSERT(slotnum < NUM_WORKER_THREADS);
        return m_slots[slotnum];
    }


private:

    // Array of slot data for all worker threads & its access control semaphore 
    // TODO Make this a vector<> ???
    CslotData	     m_slots[ NUM_WORKER_THREADS ];

    // The three event manager tasks
    CWinThread*      m_thrRequest{ nullptr };
    CWinThread*      m_thrResults{ nullptr };
    CWinThread*      m_thrRelease{ nullptr };

    // Destination of WM_DOWNLOAD_PING message
    HWND             m_hMsgWindow{ NULL };

    cRequestData     request_data;
    cResultsData     results_data;
    cReleaseData     release_data;
};

