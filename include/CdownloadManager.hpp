#pragma once

#include <vector>
#include <string>

#include "Cslots.hpp"

#include "threadData.hpp"







class CdownloadManager : public Cslots
{
public:
    CdownloadManager();
    ~CdownloadManager();

    void TerminateSlotThreads();

    void SetMsgWin(HWND hMsgWin);

    void DownloadShow(const std::string& url);
    bool DownloadInProgress() const;
    void AbortDownload();

private:

    // Destination of WM_DOWNLOAD_PING message
    HWND             m_hMsgWin{ NULL };
};

