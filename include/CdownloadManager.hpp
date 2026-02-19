#pragma once

#include <string>

#include "Cslots.hpp"




class CdownloadManager : public Cslots
{
public:
    CdownloadManager();
    ~CdownloadManager();

    void        TerminateSlotThreads();

    void        SetMsgWin(HWND hMsgWin);

    void        DownloadShow(const std::string& url);
    bool        DownloadInProgress() const;
    void        AbortDownload();
    void        ClearAbortCondition();

    void        Push(const std::string& url);
    std::string Pop();
    void        ReleaseSlot(int slotnum);

private:

    // Destination of WM_DOWNLOAD_PING message
    HWND                        m_hMsgWin{ NULL };
    std::queue<std::string>     m_url_queue;
    bool                        m_abort_pending{ false };
};

