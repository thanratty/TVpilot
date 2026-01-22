#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "Cslots.hpp"



//
// The global slot 'manager'
//

Cslots gSlots;





Cslot::Cslot()
{
    CString str;

    // Object tracing
    TRACK_DYNAMIC_OBJECTS(L"Cslot constructor\n");

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

Cslot::~Cslot()
{
    TRACK_DYNAMIC_OBJECTS(L"CSlot destructor\n");
    CloseSlot();
}


void Cslot::CloseSlot()
{
    TerminateThread();

    CloseHandle(m_hEvRequest);
    CloseHandle(m_hEvResult);
    CloseHandle(m_hEvRelease);

    m_hEvRelease = m_hEvResult = m_hEvRequest = INVALID_HANDLE_VALUE;
}




// Only called from the UI thread so no need to lock the slots ??? TODO check
void Cslot::SetUrl(const std::string& url)
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


void Cslot::ResetAndFree()
{
    m_show.Reset();
    m_error_string.erase();

    m_http_status = 0;
    m_curl_status = 0;
    m_xml_status = 0;

    m_slotstate = eSlotState::SS_FREE;
}

void Cslot::TerminateThread()
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

void Cslot::SetState(eSlotState state) {
    m_slotstate = state;
}

eSlotState Cslot::GetState() const {
    return m_slotstate;
}

const show& Cslot::GetShow() const {
    return m_show;
}

eThreadResult Cslot::GetThreadResult() const {
    return m_thread_result;
}

void Cslot::SetThreadResult(eThreadResult result) {
    m_thread_result = result;
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

HANDLE Cslot::GetResultHandle() const {
    return m_hEvResult;
}

HANDLE Cslot::GetReleaseHandle() const {
    return m_hEvRelease;
}

void Cslot::SignalRequest() const  {
    SetEvent(m_hEvRequest);
}

void Cslot::SignalResult() const {
    SetEvent(m_hEvResult);
}

void Cslot::SignalRelease() const {
    SetEvent(m_hEvRelease);
}









Cslots::Cslots()
{
    // The Cslot 'array' is constructed by now
    for (const Cslot& slot : m_slots) {
        m_ResultHandles.push_back(slot.GetResultHandle());
        m_ReleaseHandles.push_back(slot.GetReleaseHandle());
    }
}


Cslots::~Cslots() {}

bool Cslots::IsFree(unsigned slotnum) const {
    return m_slots.at(slotnum).GetState() == eSlotState::SS_FREE;
}

bool Cslots::IsBusy(unsigned slotnum) const {
    return !IsFree(slotnum);
}

const show& Cslots::GetShow(unsigned slotnum) const {
    return m_slots.at(slotnum).GetShow();
}

eSlotState Cslots::GetState(unsigned slotnum) const {
    return m_slots.at(slotnum).GetState();
}

void Cslots::SetState(unsigned slotnum, eSlotState state) {
    m_slots.at(slotnum).SetState(state);
}

eThreadResult Cslots::GetThreadResult(unsigned slotnum) const {
    return m_slots.at(slotnum).GetThreadResult();
}

void Cslots::SetUrl(unsigned slotnum, const std::string& url) {
    m_slots.at(slotnum).SetUrl(url);
}

void Cslots::SignalRequest(unsigned slotnum) const {
    m_slots.at(slotnum).SignalRequest();
}

void Cslots::SignalResult(unsigned slotnum) const {
    m_slots.at(slotnum).SignalResult();
}

void Cslots::SignalRelease(unsigned slotnum) const {
    m_slots.at(slotnum).SignalRelease();
}

const std::string& Cslots::GetErrorString(unsigned slotnum) const {
    return m_slots.at(slotnum).GetErrorString();
}

const std::vector<HANDLE>& Cslots::GetResultHandles() const {
    return m_ResultHandles;
}

const std::vector<HANDLE>& Cslots::GetReleaseHandles() const {
    return m_ReleaseHandles;
}

void Cslots::ResetAndFree(unsigned slotnum) {
    m_slots.at(slotnum).ResetAndFree();
}

int Cslots::FirstFreeSlot()
{
    auto iter = std::find_if(m_slots.begin(), m_slots.end(), [](const Cslot& s) { return s.IsFree(); });
    return (iter == m_slots.end()) ? -1 : (iter - m_slots.begin());
}

int Cslots::FirstBusySlot()
{
    auto iter = std::find_if(m_slots.begin(), m_slots.end(), [](const Cslot& s) { return s.IsBusy(); });
    return (iter == m_slots.end()) ? -1 : (iter - m_slots.begin());
}


