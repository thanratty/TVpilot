#pragma once

#include "Cshow.hpp"
#include "threadFuncs.hpp"

#include "utils.hpp"


class Cslots;

extern Cslots xxSlots;


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

protected:
    HANDLE          m_hEvRequest { INVALID_HANDLE_VALUE };
    HANDLE          m_hEvResult  { INVALID_HANDLE_VALUE };
    HANDLE          m_hEvRelease { INVALID_HANDLE_VALUE };
};















class Cslot : public CslotData
{
public:

    Cslot()
    {
        CString str;

        // Object tracing
        TRACK_DYNAMIC_OBJECTS(L"Cslot constructor");

        // Increment & copy the singleton variable to identify this slot instance (starts at -1)
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
        TRACK_DYNAMIC_OBJECTS(L"CSlot destructor");
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




    // Only called from the UI thread so no need to lock the slots ??? TODO check
    void SetUrl(const std::string& url)
    {
        if (IsFree())
        {
            SetState(eSlotState::SS_URL_SET);

            m_show.epguides_url = url;
            m_show.hash = SimpleHash(url);
        }
        else
        {
            WriteMessageLog(L"SetUrl slot not free!");
        }
    }


    void ResetAndFree()
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

    inline void SetState(eSlotState state) {
        m_slotstate = state;
    }

    inline eSlotState GetState() const {
        return m_slotstate;
    }

    inline const show& GetShow() const {
        return m_show;
    }

    inline eThreadResult GetThreadResult() const {
        return m_thread_result;
    }

    inline void SetThreadResult(eThreadResult result) {
        m_thread_result = result;
    }

    const std::string& GetErrorString() const {
        return m_error_string;
    }

    HANDLE GetRequestHandle() const {
        return m_hEvRequest;
    }

    HANDLE GetResultHandle() const {
        return m_hEvResult;
    }

    HANDLE GetReleaseHandle() const {
        return m_hEvRelease;
    }




    inline void SignalRequest() const {  SetEvent(m_hEvRequest);  }
    inline void SignalResult()  const {  SetEvent(m_hEvResult);   }
    inline void SignalRelease() const {  SetEvent(m_hEvRelease);  }

private:
    // One instance of this variable is shared between all Cslot objects
    inline static int gSlotCount{ -1 };
};






class Cslots
{

public:

    Cslots::Cslots()
    {
        // The Cslot 'array' is constructed by now
        for (const Cslot& slot : m_slots) {
            m_ResultHandles.push_back(slot.GetResultHandle());
            m_ReleaseHandles.push_back(slot.GetReleaseHandle());
        }
    };


    Cslots::~Cslots() {};

    inline bool IsFree(unsigned slotnum) const {
        return m_slots.at(slotnum).m_slotstate == eSlotState::SS_FREE;
    }

    inline bool IsBusy(unsigned slotnum) const {
        return !IsFree(slotnum);
    }

    const show& GetShow(unsigned slotnum) const {
        return m_slots.at(slotnum).GetShow();
    }

    eSlotState GetState(unsigned slotnum) const {
        return m_slots.at(slotnum).GetState();
    }

    void SetState(unsigned slotnum, eSlotState state) {
        m_slots.at(slotnum).SetState(state);
    }

    eThreadResult GetThreadResult(unsigned slotnum) const {
        return m_slots.at(slotnum).GetThreadResult();
    }

    void SetUrl(unsigned slotnum, const std::string& url) {
        m_slots.at(slotnum).SetUrl(url);
    }

    inline void SignalRequest(unsigned slotnum) const {
        m_slots.at(slotnum).SignalRequest();
    }

    inline void SignalResult(unsigned slotnum) const {
        m_slots.at(slotnum).SignalResult();
    }

    inline void SignalRelease(unsigned slotnum) const {
        m_slots.at(slotnum).SignalRelease();
    }

    const std::string& GetErrorString(unsigned slotnum) const {
        return m_slots.at(slotnum).GetErrorString();
    }

    const std::vector<HANDLE>& GetResultHandles() const {
        return m_ResultHandles;
    }

    const std::vector<HANDLE>& GetReleaseHandles() const {
        return m_ReleaseHandles;
    }

    void ResetAndFree(unsigned slotnum) {
        m_slots.at(slotnum).ResetAndFree();
    }

    int FirstFreeSlot()
    {
        auto iter = std::find_if(m_slots.begin(), m_slots.end(), [](const Cslot& s) { return s.IsFree(); });
        return (iter == m_slots.end()) ? -1 : (iter - m_slots.begin());
    }

    int FirstBusySlot()
    {
        auto iter = std::find_if(m_slots.begin(), m_slots.end(), [](const Cslot& s) { return s.IsBusy(); });
        return (iter == m_slots.end()) ? -1 : (iter - m_slots.begin());
    }



private:

    // The actual slot 'array'
    std::vector<Cslot> m_slots = std::vector<Cslot>(NUMBER_OF_DOWNLOAD_THREADS);

    std::vector<HANDLE> m_ResultHandles;
    std::vector<HANDLE> m_ReleaseHandles;

};