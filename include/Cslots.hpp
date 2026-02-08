#pragma once

#include "Cshow.hpp"
#include "threadFuncs.hpp"




constexpr UINT THREAD_TERMINATE_TIMEOUT     (5000);



enum class eSlotState
{
    SS_FREE,
    SS_URL_SET,
    SS_AWAKE,
    SS_CURLING,
    SS_CURL_ERROR,
    SS_PARSING,
    SS_PARSE_ERROR,
    SS_PARSED_OK,
    SS_RESULTS_READY,
    SS_RESULTS_PROCESSING,
    SS_RESULTS_PROCESSED,
    //
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

    eThreadState    m_thread_state{ eThreadState::TS_NOT_STARTED };
    eSlotState      m_slotstate{ eSlotState::SS_FREE };
};















class Cslot : public CslotData
{
public:


    Cslot();
    ~Cslot();

    void        SetUrl(const std::string& url);

    void        StartThread();
    void        ResetAndFree();
    void        TerminateThread();
    void        CloseSlot();

    void        SetExitFlag();
    bool        GetExitFlag() const;

    inline bool IsBusy() const;
    inline bool IsFree() const;

    eSlotState  GetSlotState() const;
    void        SetSlotState(eSlotState state);

    eThreadState    GetThreadState() const;
    void            SetThreadState(eThreadState state);

    inline const show& GetShow() const;
    const std::string& GetErrorString() const;

    HANDLE      GetRequestHandle() const;
    void        SignalRequest() const;

    void        SetMsgWin(HWND hMsgWin);
    HWND        GetMsgWin(void) const;


private:
    // One instance of this variable is shared between all Cslot objects
    inline static LONG  gSlotCount{ -1 };
    CString             m_SlotName;
    HWND                m_hMsgWin;
};






class Cslots
{

public:

    Cslots::Cslots();
    Cslots::~Cslots();

    void SetMsgWin( HWND hWin );
    void TerminateSlotThreads();

    bool IsFree(unsigned slotnum) const;
    bool IsBusy(unsigned slotnum) const;

    const show& GetSlotShow(unsigned slotnum) const;

    eSlotState GetSlotState(unsigned slotnum) const;
    void SetSlotState(unsigned slotnum, eSlotState state);

    void SetUrl(unsigned slotnum, const std::string& url);

    void SignalRequest(unsigned slotnum) const;

    const std::string& GetErrorString(unsigned slotnum) const;

    const std::vector<HANDLE>& GetReleaseHandles() const;
    void ResetAndFree(unsigned slotnum);

    int FirstFreeSlot() const;
    int FirstBusySlot() const;


private:

    // The actual slot 'array'
    std::vector<Cslot>  m_slots = std::vector<Cslot>(NUMBER_OF_DOWNLOAD_THREADS);

    std::vector<HANDLE> m_ReleaseHandles;

};