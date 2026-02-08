#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"
#include "utils.hpp"
#include "logging.hpp"

#include "Cslots.hpp"








Cslot::Cslot()
{
    // NB The logging thread has not started at this point

    CString str;

    // Increment & copy the singleton variable to identify this slot instance (starts at -1)
    m_SlotNumber = InterlockedIncrement(&gSlotCount);
    m_SlotName.Format(L"slot-%02u", m_SlotNumber);

    str.Format(L"evRequest-%-d", m_SlotNumber);
    m_hEvRequest = CREATE_EVENT(NULL, FALSE, FALSE, str);         // AUTO reset, initial state

    StartThread();
}


Cslot::~Cslot()
{
    // NB The logging thread has been shutdown by now
    CloseSlot();
}


void Cslot::StartThread()
{
    m_pWinThread = AfxBeginThread(thrSlotThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    ASSERT(m_pWinThread);
    m_pWinThread->m_bAutoDelete = false;
    m_pWinThread->ResumeThread();
    SetThreadDescription(m_pWinThread->m_hThread, m_SlotName);
}


void Cslot::TerminateThread()
{
    LOG_PRINT(eLogFlags::DL_THREADS,L"Slot %u TerminateThread()\n", m_SlotNumber);

    if (m_pWinThread)
    {
        // Terminates the worker thread for this slot
        m_exit_thread = true;
        m_slotstate = eSlotState::SS_THREAD_EXIT_FLAGGED;
        SetEvent(m_hEvRequest);

        if (WAIT_OBJECT_0 == WaitForSingleObject(m_pWinThread->m_hThread, THREAD_TERMINATE_TIMEOUT))
            m_slotstate = eSlotState::SS_THREAD_EXITED;
        else
            LogMsgWin(L"Slot thread didn't terminate within timeout");

        delete m_pWinThread;
        m_pWinThread = nullptr;
    }
}


void Cslot::CloseSlot()
{
    LOG_PRINT(eLogFlags::DL_THREADS, L"Slot %u CloseSlot()\n", m_SlotNumber);

    if (m_pWinThread != nullptr) {
        LOG_PRINT(eLogFlags::DL_THREADS, L"Slot %u thread still running\n", m_SlotNumber);
        TerminateThread();
    }

    CloseHandle(m_hEvRequest);
    m_hEvRequest = INVALID_HANDLE_VALUE;
}




// Only called from the UI thread so no need to lock the slots ??? TODO check
void Cslot::SetUrl(const std::string& url)
{
    if (IsFree())
    {
        SetSlotState(eSlotState::SS_URL_SET);

        m_show.epguides_url = url;
        m_show.hash = SimpleHash(url);
    }
    else
    {
        LogMsgWin(L"SetUrl slot not free!");
    }
}


void Cslot::ResetAndFree()
{
    m_show.Reset();
    m_error_string.erase();

    m_http_status = 0;
    m_curl_status = 0;
    m_xml_status = 0;

    m_slotstate = eSlotState::SS_FREE;
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
    SetEvent(m_hEvRequest);
}

void Cslot::SetMsgWin(HWND hMsgWin) {
    m_hMsgWin = hMsgWin;
}

HWND Cslot::GetMsgWin(void) const {
    return m_hMsgWin;
}










Cslots::Cslots() {}
Cslots::~Cslots() {}


void Cslots::SetMsgWin(HWND hWin)
{
    for (Cslot& slot : m_slots)
        slot.SetMsgWin(hWin);
}


void Cslots::TerminateSlotThreads()
{
    for (auto slot : m_slots)
    {
        slot.CloseSlot();
        while (slot.GetThreadState() != eThreadState::TS_FINISHED);
    }
}



bool Cslots::IsFree(unsigned slotnum) const {
    return m_slots.at(slotnum).GetSlotState() == eSlotState::SS_FREE;
}

bool Cslots::IsBusy(unsigned slotnum) const {
    return !IsFree(slotnum);
}

const show& Cslots::GetSlotShow(unsigned slotnum) const {
    return m_slots.at(slotnum).GetShow();
}

eSlotState Cslots::GetSlotState(unsigned slotnum) const {
    return m_slots.at(slotnum).GetSlotState();
}

void Cslots::SetSlotState(unsigned slotnum, eSlotState state) {
    m_slots.at(slotnum).SetSlotState(state);
}

void Cslots::SetUrl(unsigned slotnum, const std::string& url) {
    m_slots.at(slotnum).SetUrl(url);
}

void Cslots::SignalRequest(unsigned slotnum) const {
    m_slots.at(slotnum).SignalRequest();
}

const std::string& Cslots::GetErrorString(unsigned slotnum) const {
    return m_slots.at(slotnum).GetErrorString();
}

const std::vector<HANDLE>& Cslots::GetReleaseHandles() const {
    return m_ReleaseHandles;
}

void Cslots::ResetAndFree(unsigned slotnum) {
    m_slots.at(slotnum).ResetAndFree();
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


