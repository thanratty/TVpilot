#pragma once

#include <vector>
#include <string>

#include "Cslots.hpp"

#include "threadData.hpp"







class CdownloadManager : public Cslots
{
public:
    CdownloadManager() {};
    ~CdownloadManager();

    void SetMsgWindow(HWND hMsgWindow);

    void DownloadShow(const std::string& url);
    bool DownloadInProgress() const;
    void AbortDownload();

    const show& GetShow(UINT slotnum) const;
    void ReleaseSlot(DWORD slotnum);

//    eSlotState GetSlotState(UINT slotnum) const;
//    void SetSlotState(UINT slotnum, eSlotState state);
    eThreadResult GetThreadResult(UINT slotnum) const;
    const std::string& GetErrorString(UINT slotnum) const;


private:

    // Destination of WM_DOWNLOAD_PING message
    HWND             m_hMsgWindow{ NULL };

    // Thread data objects
    cRequests        requests;
    cResults         results;
    cReleases        releases;
};

