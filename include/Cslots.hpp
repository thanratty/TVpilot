#pragma once

#include "Cshow.hpp"
#include "threadFuncs.hpp"




constexpr UINT THREAD_TERMINATE_TIMEOUT     (5000);



enum class eSlotState
{
    SS_FREE,
    SS_URL_SET,
    SS_JOB_AWAKE,
    SS_DOWNLOAD_ERROR,
    SS_RESULTS_READY,
    SS_RESULTS_THREAD,
    SS_NOTIFY_SENT,
    SS_JOB_ERROR,
    SS_PROCESSING,
    SS_PROCESSED,
    SS_THREAD_EXIT_FLAGGED,
    SS_THREAD_EXITING,
    SS_THREAD_EXITED
};







class CslotData
{
public:

    // This slot's instance number
    unsigned        m_SlotNumber{ 0 };

    show            m_show;

    // Status & errors
    //
    std::string     m_error_string;
    int             m_http_status{ INT_MAX };
    int             m_curl_status{ INT_MAX };
    int             m_xml_status{ INT_MAX };

    CWinThread*     m_pWinThread{ nullptr };

protected:

    bool            m_exit_thread{ false };

    HANDLE          m_hEvRequest { INVALID_HANDLE_VALUE };
    HANDLE          m_hEvResult  { INVALID_HANDLE_VALUE };
    HANDLE          m_hEvRelease { INVALID_HANDLE_VALUE };

    eThreadState    m_thread_state{ eThreadState::TS_UNKNOWN };
    eThreadResult   m_thread_result{ eThreadResult::TR_OK };
    eSlotState      m_slotstate{ eSlotState::SS_FREE };
};















class Cslot : public CslotData
{
public:

    Cslot();
    ~Cslot();

    void CloseSlot();
    void SetUrl(const std::string& url);
    void ResetAndFree();
    void TerminateThread();
    void SetExitFlag();
    bool GetExitFlag() const;

    inline bool IsBusy() const;
    inline bool IsFree() const;

    eSlotState GetState() const;
    void SetState(eSlotState state);

    eThreadState GetThreadState() const;
    void SetThreadState(eThreadState state);

    eThreadResult GetThreadResult() const;
    void SetThreadResult(eThreadResult result);

    inline const show& GetShow() const;
    const std::string& GetErrorString() const;

    HANDLE GetRequestHandle() const;
    HANDLE GetResultHandle() const;
    HANDLE GetReleaseHandle() const;

    void SignalRequest() const;
    void SignalResult() const;
    void SignalRelease() const;

    void StartThread();

private:
    // One instance of this variable is shared between all Cslot objects
    inline static LONG gSlotCount{ -1 };
    CString     m_SlotName;
};






class Cslots
{

public:

    Cslots::Cslots();
    Cslots::~Cslots();

    void TerminateSlotThreads();

    bool IsFree(unsigned slotnum) const;
    bool IsBusy(unsigned slotnum) const;

    const show& GetShow(unsigned slotnum) const;

    eSlotState GetState(unsigned slotnum) const;
    void SetState(unsigned slotnum, eSlotState state);

    void SetUrl(unsigned slotnum, const std::string& url);

    void SignalRequest(unsigned slotnum) const;
    void SignalResult(unsigned slotnum) const;
    void SignalRelease(unsigned slotnum) const;

    const std::string& GetErrorString(unsigned slotnum) const;
    eThreadResult GetThreadResult(unsigned slotnum) const;

    const std::vector<HANDLE>& GetResultHandles() const;
    const std::vector<HANDLE>& GetReleaseHandles() const;
    void ResetAndFree(unsigned slotnum);

    int FirstFreeSlot();
    int FirstBusySlot();


private:

    // The actual slot 'array'
    std::vector<Cslot> m_slots = std::vector<Cslot>(NUMBER_OF_DOWNLOAD_THREADS);

    std::vector<HANDLE> m_ResultHandles;
    std::vector<HANDLE> m_ReleaseHandles;

};