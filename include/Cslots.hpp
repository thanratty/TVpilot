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







class Cslot
{
public:
        Cslot();
        ~Cslot();

        void         SetUrl(const std::string& url);
        void         Reset();
        bool         IsBusy() const;
        bool         IsFree() const;


        void         SetExitFlag();
        bool         GetExitFlag(void) const;
        void         SetMsgWin(HWND hMsgWin);
        HWND         GetMsgWin(void) const;
        eThreadState GetThreadState() const;
        void         SetThreadState(eThreadState state);
        eSlotState   GetSlotState() const;
        void         SetSlotState(eSlotState state);

        show&           GetShow();
 const std::string&     GetErrorString() const;

        HANDLE       GetRequestHandle() const;
        void         SignalRequest() const;

private:
        void        StartThread();
        void        TerminateThread();


        // One instance of this variable is shared between all Cslot objects
        inline static volatile LONG  gSlotCount{ -1 };

        CString         m_SlotName;
        HWND            m_hMsgWin;
        CWinThread*     m_pWinThread{ nullptr };                    // The associated worker/download thread for this slot
        bool            m_exit_thread{ false };
        HANDLE          m_hEvRequest{ INVALID_HANDLE_VALUE };
        show            m_show;                                     // The show this slot is currently downloading
        eThreadState    m_thread_state{ eThreadState::TS_NOT_STARTED };
        eSlotState      m_slotstate{ eSlotState::SS_FREE };

        /**
         * Status & errors
         */
public:
        unsigned        m_SlotNumber{ 0 };                          // This slot's instance number
        unsigned        m_EventCounter{ 0 };

        std::string     m_error_string;
        int             m_http_status{ INT_MAX };
        int             m_curl_status{ INT_MAX };
        int             m_xml_status{ INT_MAX };
};






class Cslots
{
public:
    Cslots();
    ~Cslots();

    void        SetMsgWin( HWND hWin );

    bool        IsFree(unsigned slotnum) const;
    bool        IsBusy(unsigned slotnum) const;
    int         FirstFreeSlot() const;
    int         FirstBusySlot() const;
    void        ReleaseSlot(unsigned slotnum);
    void        ReleaseAllSlots(void);

    show&       GetSlotShow(unsigned slotnum);
    eSlotState  GetSlotState(unsigned slotnum) const;
    void        SetSlotState(unsigned slotnum, eSlotState state);
    void        SetUrl(unsigned slotnum, const std::string& url);
    const std::string& GetErrorString(unsigned slotnum) const;

    void        SignalRequest(unsigned slotnum) const;

private:
    // The actual slot array
    std::vector<Cslot>  m_slots = std::vector<Cslot>(NUMBER_OF_DOWNLOAD_THREADS);
};
