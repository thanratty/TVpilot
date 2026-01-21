#pragma once

#pragma once


#include "Cshow.hpp"
#include "threadFuncs.hpp"

#include "utils.hpp"






constexpr UINT THREAD_TERMINATE_TIMEOUT     (5000);



enum class eSlotState
{
    SS_FREE,
    SS_URL_SET,
    SS_JOB_THREAD,
    SS_RESULTS_READY,
    SS_RESULTS_THREAD,
    SS_NOTIFY_SENT,
    SS_PROCESSING,
    SS_JOB_ERROR,
    SS_PROCESSED,
    SS_THREAD_EXIT_FLAGGED,
    SS_THREAD_EXITING,
    SS_THREAD_EXITED
};





class CslotData
{
public:

    // This slot's instance number
    int             m_SlotNumber{ -1 };

    show            m_show;

    HANDLE          m_hEvRequest{ INVALID_HANDLE_VALUE };
    HANDLE          m_hEvResult{ INVALID_HANDLE_VALUE };
    HANDLE          m_hEvRelease{ INVALID_HANDLE_VALUE };

    // Status & errors
    //
    std::string     m_error_string;
    int             m_http_status{ INT_MAX };
    int             m_curl_status{ INT_MAX };
    int             m_xml_status{ INT_MAX };

    CWinThread*     m_pWinThread{ nullptr };

    bool            m_exit_thread{ false };

    eThreadState    m_thread_state{ eThreadState::TS_UNKNOWN };
    eThreadResult   m_thread_result{ eThreadResult::TR_OK };
    eSlotState      m_slotstate{ eSlotState::SS_FREE };
};















class Cslot : public CslotData
{
public:

    Cslot()
    {
        CString str;

        // Object tracing
        TRACE_CREATION(L"Cslot constructor");

        // Inc the singleton (starts at -1)
        gSlotCount++;
        m_SlotNumber = gSlotCount;

        str.Format(L"evRequest-%-d", m_SlotNumber);
        m_hEvRequest = CREATE_EVENT(NULL, FALSE, FALSE, str);         // AUTO reset, initial state

        str.Format(L"evResults-%-d", m_SlotNumber);
        m_hEvResult = CREATE_EVENT(NULL, FALSE, FALSE, str);          // AUTO reset, initial state

        str.Format(L"evRelease-%-d", m_SlotNumber);
        m_hEvRelease = CREATE_EVENT(NULL, FALSE, FALSE, str);         // AUTO reset, initial state


        m_pWinThread = AfxBeginThread(thrSlotThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        ASSERT(m_pWinThread);
        m_pWinThread->m_bAutoDelete = false;
        m_pWinThread->ResumeThread();
        str.Format(L"slot-%02u", m_SlotNumber);
        SetThreadDescription(m_pWinThread->m_hThread, str);
    }

    ~Cslot()
    {
        // Object tracing
        TRACE_CREATION(L"CSlot destructor");
        CloseSlot();
    }


    void CloseSlot()
    {
        TerminateThread();

        CloseHandle(m_hEvRequest);
        CloseHandle(m_hEvResult);
        CloseHandle(m_hEvRelease);

        m_hEvRelease = m_hEvResult = m_hEvRequest = INVALID_HANDLE_VALUE;
    }




    // Only called from the UI thread so no need to lock the data ??? TODO check
    void SetUrl(const std::string& url)
    {
        if (IsFree())
        {
            m_slotstate = eSlotState::SS_URL_SET;

            m_show.epguides_url = url;
            m_show.hash = SimpleHash(url);
        }
        else
        {
            WriteMessageLog(L"SetUrl slot not free!");
        }
    }


    void Reset()
    {
        m_show.Reset();
        m_error_string.erase();

        m_http_status = 0;
        m_curl_status = 0;
        m_xml_status  = 0;

        m_slotstate = eSlotState::SS_FREE;
    }

    void TerminateThread()
    {
        if (m_pWinThread)
        {
            // Terminates the worker thread for this slot
            m_exit_thread = true;
            m_slotstate = eSlotState::SS_THREAD_EXIT_FLAGGED;

            SetEvent(m_hEvRequest);
            if (WAIT_OBJECT_0 == WaitForSingleObject(m_pWinThread, THREAD_TERMINATE_TIMEOUT))
                m_slotstate = eSlotState::SS_THREAD_EXITED;
            else
                WriteMessageLog(L"Slot thread didn't terminate within timeout");

            delete m_pWinThread;
            m_pWinThread = nullptr;
        }
    }


    inline bool IsBusy() const {
        return !IsFree();
    }

    inline bool IsFree() const {
        return m_slotstate == eSlotState::SS_FREE;
    }



private:
    // One instance of this variable is shared between all Cslot objects
    inline static int gSlotCount{ -1 };

};




