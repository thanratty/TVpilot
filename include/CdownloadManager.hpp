#pragma once

#include <vector>
#include <string>

#include "Cslots.hpp"

#include "threadData.hpp"






extern Cslots gSlots;



class CdownloadManager
{
public:
    CdownloadManager() {};
    ~CdownloadManager();

    void SetMsgWindow(HWND hMsgWindow);
    void DownloadShow(const std::string& url);
    bool DownloadInProgress() const;
    void AbortDownload();

    void OnSlotReleased( DWORD slotnum );

    void ReleaseSlot(DWORD slotnum)
    {
        gSlots.SetState(slotnum, eSlotState::SS_PROCESSED);
        gSlots.SignalRelease(slotnum);
    }


private:

    // Destination of WM_DOWNLOAD_PING message
    HWND             m_hMsgWindow{ NULL };

    // Thread data objects
    cRequests        requests;
    cResults         results;
    cReleases        releases;
};

