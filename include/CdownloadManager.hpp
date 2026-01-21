#pragma once

#include <vector>
#include <string>

#include "threadData.hpp"
#include "Cslot.hpp"
#include "debugConsole.h"








class CdownloadManager
{
public:
    CdownloadManager() {};
    ~CdownloadManager() {};

    void SetMsgWindow(HWND hMsgWindow);
    void DownloadShow(const std::string& url);
    bool DownloadInProgress() const;
    void AbortDownload();
    void OnSlotReleased( DWORD slotnum );


    void ResultsProcessed(DWORD slotnum)
    {
        gSlots[ slotnum ].m_slotstate = eSlotState::SS_PROCESSED;

        // Notify the release task to free the slot
        if (SetEvent(gSlots[ slotnum ].m_hEvRelease) == 0)
            WriteDebugConsole(L"Can't set slot evRelease");
    }


private:

    // Destination of WM_DOWNLOAD_PING message
    HWND             m_hMsgWindow{ NULL };

    cRequests        requests;
    cResults         results;
    cReleases        releases;
};

