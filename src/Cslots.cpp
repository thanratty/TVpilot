#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "utils.hpp"

#include "Cslots.hpp"







Cslot::Cslot()
{
    // NB The logging thread has not started at this point

    CString str;

    // Increment & copy the singleton variable to identify this slot instance (starts at -1)
    m_SlotNumber = InterlockedIncrement(&gSlotCount);
    m_SlotName.Format(L"slot-%02u", m_SlotNumber);

    str.Format(L"evRequest-%-d", m_SlotNumber);
    m_hEvRequest = CREATE_EVENT(NULL, FALSE, FALSE, str);         // AUTO reset, initial state unsignalled

    StartThread();
}


Cslot::~Cslot()
{
    CloseHandle(m_hEvRequest);
    m_hEvRequest = INVALID_HANDLE_VALUE;
}


void Cslot::StartThread()
{
    m_pWinThread = AfxBeginThread(thrSlotThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    ASSERT(m_pWinThread);
    m_pWinThread->m_bAutoDelete = false;

#if (NAMED_OBJECTS==1)
    SetThreadDescription(m_pWinThread->m_hThread, m_SlotName);
#endif

    m_pWinThread->ResumeThread();
}


void Cslot::TerminateThread()
{
    CONSOLE_PRINT(eLogFlags::SLOT_THREAD,L"CSlot::TerminateThread() %u\n", m_SlotNumber);

    if (m_pWinThread)
    {
        // Terminates the worker thread for this slot
        m_exit_thread = true;
        m_slotstate   = eSlotState::SS_THREAD_EXIT_FLAGGED;
        SignalRequest();

        DWORD result = WaitForSingleObject(m_pWinThread->m_hThread, THREAD_TERMINATE_TIMEOUT);
        if (result == WAIT_OBJECT_0)
                m_slotstate = eSlotState::SS_THREAD_EXITED;
        else {
            CString errmsg;
            errmsg.Format(L"Slot thread %u didn't terminate within timeout. Result %08X, Error %08X", m_SlotNumber, result, GetLastError());
            LogMsgWin(errmsg);
            CONSOLE_PRINT(eLogFlags::SLOT_THREAD, errmsg);
        }

        delete m_pWinThread;
        m_pWinThread = nullptr;
    }
    else
    {
        CONSOLE_PRINT(eLogFlags::SLOT_THREAD, L"Slot %u thread already NULL\n", m_SlotNumber);
    }
}




// Only called from the UI thread so no need to lock the slots.
void Cslot::SetUrl(const std::string& url)
{
    if (IsFree())
    {
        SetSlotState(eSlotState::SS_URL_SET);

        m_show.epguides_url = url;
        m_show.hash = std::hash<std::string>()(url);
    }
    else
    {
        CONSOLE_PRINT(eLogFlags::INFO, L"SetUrl() slot %u not free!", m_SlotNumber);
    }
}


void Cslot::Reset()
{
    m_show.Initialise();

    m_error_string.erase();
    m_http_status = 0;
    m_curl_status = 0;
    m_xml_status  = 0;

    SetSlotState(eSlotState::SS_FREE);
}

void Cslot::SetExitFlag() {
    m_exit_thread = true;
}

bool Cslot::GetExitFlag() const {
    return m_exit_thread;
}

bool Cslot::IsBusy() const {
    return !IsFree();
}

bool Cslot::IsFree() const {
    return m_slotstate == eSlotState::SS_FREE;
}

void Cslot::SetSlotState(eSlotState state) {
    m_slotstate = state;
}

eSlotState Cslot::GetSlotState() const {
    return m_slotstate;
}

const show& Cslot::GetShow() const {
    return m_show;
}

eThreadState Cslot::GetThreadState() const {
    return m_thread_state;
}

void Cslot::SetThreadState(eThreadState state) {
    m_thread_state = state;
}

const std::string& Cslot::GetErrorString() const {
    return m_error_string;
}

HANDLE Cslot::GetRequestHandle() const {
    return m_hEvRequest;
}

void Cslot::SignalRequest() const  {
    if (SetEvent(m_hEvRequest) == 0) {
        CONSOLE_PRINT(eLogFlags::FATAL, "Cslot::SignalRequest() SetEvent failed. Error %08X", GetLastError());
    }

}

void Cslot::SetMsgWin(HWND hMsgWin) {
    m_hMsgWin = hMsgWin;
}

HWND Cslot::GetMsgWin(void) const {
    return m_hMsgWin;
}












void Cslots::SetMsgWin(HWND hWin)
{
    for (Cslot& slot : m_slots)
        slot.SetMsgWin(hWin);
}

bool Cslots::IsFree(unsigned slotnum) const {
    return m_slots[slotnum].IsFree();
}

bool Cslots::IsBusy(unsigned slotnum) const {
    return m_slots[slotnum].IsBusy();
}

const show& Cslots::GetSlotShow(unsigned slotnum) const {
    return m_slots[slotnum].GetShow();
}

eSlotState Cslots::GetSlotState(unsigned slotnum) const {
    return m_slots[slotnum].GetSlotState();
}

void Cslots::SetSlotState(unsigned slotnum, eSlotState state) {
    m_slots[slotnum].SetSlotState(state);
}

void Cslots::SetUrl(unsigned slotnum, const std::string& url) {
    m_slots[slotnum].SetUrl(url);
}

void Cslots::SignalRequest(unsigned slotnum) const {
    m_slots[slotnum].SignalRequest();
}

const std::string& Cslots::GetErrorString(unsigned slotnum) const {
    return m_slots[slotnum].GetErrorString();
}

void Cslots::ReleaseSlot(unsigned slotnum) {
    m_slots[slotnum].Reset();
}

int Cslots::FirstFreeSlot() const
{
    auto iter = std::find_if(m_slots.begin(), m_slots.end(), [](const Cslot& s) { return s.IsFree(); });
    return (iter == m_slots.end()) ? -1 : (iter - m_slots.begin());
}

int Cslots::FirstBusySlot() const
{
    auto iter = std::find_if(m_slots.begin(), m_slots.end(), [](const Cslot& s) { return s.IsBusy(); });
    return (iter == m_slots.end()) ? -1 : (iter - m_slots.begin());
}

void Cslots::TerminateSlotThreads()
{
    std::for_each(m_slots.begin(), m_slots.end(), [](Cslot& slot){ slot.TerminateThread();});
}

bool Cslots::AllSlotThreadsTerminated() const
{
    auto it = std::find_if(m_slots.begin(), m_slots.end(), [](const Cslot& slot) { return slot.GetSlotState() != eSlotState::SS_THREAD_EXITED; });
    return (it == m_slots.end());
}
