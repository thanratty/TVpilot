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


    void ReleaseSlot(DWORD slotnum)
    {
        xxSlots.SetState(slotnum, eSlotState::SS_PROCESSED);
        xxSlots.SignalRelease(slotnum);
    }


private:

    // Destination of WM_DOWNLOAD_PING message
    HWND             m_hMsgWindow{ NULL };

    cRequests        requests;
    cResults         results;
    cReleases        releases;
};

