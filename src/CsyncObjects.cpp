#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "utils.hpp"
#include "debugConsole.h"

#include "CsyncObjects.hpp"



#if (ENABLE_CONSOLE_WINDOW==1) && (TRACE_SYNC_OBJECTS==1) && defined (_DEBUG)
#define     SyncDebugMessage(x)     Sync_Debug_Message(x)
#else
#define     SyncDebugMessage(x)     do {} while(0)
#endif





/**
 * Local debugging output function tailored for sync classes to show GetLastError()
 * 
 * Don't use WriteMessageLog() because we can't safely access UI objects from worker threads
 * and this function would use SendMessage to manipulate the message window. 
 */
#if (ENABLE_CONSOLE_WINDOW==1) && (TRACE_SYNC_OBJECTS==1) && defined (_DEBUG)

STATIC void Sync_Debug_Message(const wchar_t* msg)
{
    CString str;
    DWORD err = GetLastError();
    if (err) {
        str.Format(L"%s : %08X\n", msg, err);
        Beep(750,150);
    }
    else
        str.Format(L"%s\n", msg);

    WriteDebugConsole(str);
}

#endif


CslotsSem::CslotsSem()
{
    m_refcount++;
    m_name.Format(L"slotsSem-%-u", m_refcount);

    // There's only one underlying semaphore object for all instances
    if (m_hSem == nullptr)
    {
        m_hSem = CREATE_SEMAPHORE(
            NULL,               // Default security attributes
            1,                  // Initial count
            1,                  // Maximum count
            m_name );

        if (m_hSem == NULL) {
            m_last_error = GetLastError();
            SyncDebugMessage(L"Can't create slots semaphore");
        }
        else
            SyncDebugMessage(L"** Base CSlotSem semaphore created");
    }
    SyncDebugMessage(m_name + L" created");
}


CslotsSem::~CslotsSem()
{
    CString str;
    str.Format(L"CslotsSem destructor for %s, refcount %d\n", (LPCWSTR) m_name, m_refcount);
    SyncDebugMessage(str);

    // There's only ONE underlying semaphore handle, so only close the last instance
    if (m_refcount == 1)
    {
        if (CloseHandle(m_hSem) == 0)
        {
            m_last_error = GetLastError();
            SyncDebugMessage(L"Can't delete base slots CslotsSem semaphore.");
        }
    }

    m_refcount--;
}


bool CslotsSem::Lock()
{
    ASSERT(m_hSem != nullptr);

    DWORD result = WaitForSingleObject(m_hSem, INFINITE);
    if (CheckWaitResult(1, result))
        return true;

    m_last_error = GetLastError();
    SyncDebugMessage(L"CslotsSem::Lock() wait fail");
    return false;
}


bool CslotsSem::Unlock()
{
    static LONG last_count = -1;

    if (ReleaseSemaphore(m_hSem, 1, &last_count) != 0)
        return true;

    m_last_error = GetLastError();
    SyncDebugMessage(L"CslotsSem::Unlock() release fail");
    return false;
}








CMultiEvents::CMultiEvents(const HANDLE* handles, unsigned num_events)
{
    for (unsigned i = 0; i < num_events; i++)
    {
        m_handles.push_back(handles[i]);
        m_bIsSignalled.push_back(false);
    }
    CString str;
    str.Format(L"CMultiEvents created with %d handles", num_events);
    SyncDebugMessage(str);
}


// Returns the index of the signalled event, or E_WAIT_FAIL
int CMultiEvents::Wait()
{
    DWORD result = WaitForMultipleObjects( m_handles.size(), m_handles.data(), FALSE, INFINITE);

    if (CheckWaitResult(m_handles.size(), result))
    {
        m_bIsSignalled[ result ] = true;
        return result;
    }

    m_last_error = GetLastError();
    SyncDebugMessage(L"CMultiEvents::Wait() failed");

    return E_WAIT_FAIL;
}


int CMultiEvents::Reset(DWORD index)
{
    if (index >= m_handles.size())
        return E_RESET_FAIL_INDEX;
    if (m_bIsSignalled[ index ] == false)
        return E_RESET_FAIL_UNLOCK;

    if (ResetEvent(m_handles[ index ])) {
        m_bIsSignalled[ index ] = false;
        return 0;
    }

    m_last_error = GetLastError();
    SyncDebugMessage(L"CMultiEvents::Reset() failed");

    return E_RESET_FAIL;
}

