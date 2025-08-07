#pragma once


#include "Cshow.h"
#include "utils.hpp"


// We create one of these for each worker thread.
//

class CslotData
{
public:
    CslotData() 
    {
        CString str;

        str.Format(L"evRequest-%-u", nInstanceCount);
        evRequest = CREATE_EVENT(NULL, TRUE, FALSE, str);          // Manual reset, initial state

        str.Format(L"evResults-%-u", nInstanceCount);
        evResults = CREATE_EVENT(NULL, TRUE, FALSE, str);          // Manual reset, initial state

        str.Format(L"evRelease-%-u", nInstanceCount);
        evRelease = CREATE_EVENT(NULL, TRUE, FALSE, str);          // Manual reset, initial state

        nInstanceCount++;
    }

    ~CslotData()
    {
        CloseHandle(evRelease);
        CloseHandle(evResults);
        CloseHandle(evRequest);
    }

    inline bool IsBusy() const
    {
        return m_slot_busy;
    }

    inline bool IsFree() const
    {
        return !IsBusy();
    }

    inline UINT ThreadStatus() const
    {
        return m_thread_status;
    }

    inline void SetThreadStatus(UINT status) 
    {
        m_thread_status = status;
    }

    inline const std::string& ErrorString() const
    {
        return m_error_string;
    }


    /**
    Do we an acquire()? Can another thread could get an IsFree on the same slot and they both
    try to grab it.?
    */

    void SetUrl(const std::string& url)
    {
        m_slot_busy = true;
        m_show.epguides_url = url;
        m_show.hash = SimpleHash(url);
    }

    void ResetAndFree()
    {
        m_show.Reset();
        m_error_string.erase();

        m_thread_status = E_THREAD_OK;
        m_http_status   = 0;
        m_curl_status   = 0;
        m_xml_status    = 0;

        m_slot_busy     = false;
    }

    void TerminateThread()
    {
        // Terminates the worker thread for this slot
        m_exit_thread = true;
        SetEvent(evRequest);
    }

    inline bool TerminateRequested() const
    {
        return m_exit_thread;
    }


public:
    // Slot data
    //
    show            m_show;
    
    HANDLE          evRequest{ nullptr };
    HANDLE          evResults{ nullptr };
    HANDLE          evRelease{ nullptr };

    // Status & errors
    //
    std::string     m_error_string;
    int             m_http_status{ INT_MAX };
    int             m_curl_status{ INT_MAX };
    int             m_xml_status{ INT_MAX };

private:
    UINT            m_thread_status{ E_THREAD_OK };

    bool            m_slot_busy{ false };
    bool            m_exit_thread{ false };

    // One instance of this var shared between all objects
    inline static unsigned nInstanceCount = 1;
};


