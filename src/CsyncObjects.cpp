#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "utils.hpp"
#include "debugConsole.h"

#include "CsyncObjects.hpp"





constexpr DWORD SLOT_LOCK_TIMEOUT = 5000;







#if (ENABLE_CONSOLE_WINDOW==1) && (ENABLE_SYNC_OBJECT_TRACKING==1) && defined (_DEBUG)
#define     LOG_SYNC_OBJECT(msg)     WriteDebugConsole(msg)
#else
#define     LOG_SYNC_OBJECT(x,...)     do {} while(0)
#endif







CslotsSem::CslotsSem()
{
    m_refcount++;
    m_name.Format(L"slotsSem-%-u", m_refcount);

    // There's only one underlying semaphore object for all instances
    if (m_hSem == INVALID_HANDLE_VALUE)
    {
        LOG_SYNC_OBJECT(L"Creating underlying slot semaphore\n");
        m_hSem = CREATE_SEMAPHORE(
            NULL,               // Default security attributes
            1, 1,               // Initial count, Maximum count
            m_name );

        if (m_hSem == NULL) {
            m_last_error = GetLastError();
            LOG_SYNC_OBJECT(L"Can't create slots semaphore\n");
        }
        else
            LOG_SYNC_OBJECT(L"Base CSlotSem semaphore handle created\n");
    }
    LOG_SYNC_OBJECT(m_name + L" created\n");
}


CslotsSem::~CslotsSem()
{
    CString str;
    str.Format(L"CslotsSem destructor for %s, refcount %d\n", (LPCWSTR) m_name, m_refcount);
    LOG_SYNC_OBJECT(str);

    // There's only ONE underlying semaphore handle, so only close the last instance
    if (m_refcount == 1)
    {
        LOG_SYNC_OBJECT(L"Closing underlying slot semaphore\n");
        if (CloseHandle(m_hSem) == 0)
        {
            m_last_error = GetLastError();
            LOG_SYNC_OBJECT(L"Can't delete base slots CslotsSem semaphore\n");
        }
    }

    m_refcount--;
}


bool CslotsSem::Lock()
{
    ASSERT(m_hSem != nullptr);

    DWORD result = WaitForSingleObject(m_hSem, SLOT_LOCK_TIMEOUT);
    if (CheckWaitResult(1, result))
        return true;

    m_last_error = GetLastError();
    LOG_SYNC_OBJECT(L"CslotsSem::Lock() wait fail\n");
    return false;
}


bool CslotsSem::Unlock()
{
    static LONG last_count = -1;

    if (ReleaseSemaphore(m_hSem, 1, &last_count) != 0)
        return true;

    m_last_error = GetLastError();

    CString str;
    str.Format(L"CslotsSem::Unlock() release fail. Error %u. Count %lu\n", m_last_error, last_count);
    LOG_SYNC_OBJECT(str);

    return false;
}








CMultiEvents::CMultiEvents(const std::vector<HANDLE>& handles)
{
    m_handles.insert(m_handles.end(), handles.begin(), handles.end());

    CString str;
    str.Format(L"CMultiEvents created with %u handles\n", m_handles.size());
    LOG_SYNC_OBJECT(str);
}


// Returns the index of the signalled event, or E_SO_WAIT_FAIL
//
int CMultiEvents::Wait()
{
    DWORD result = WaitForMultipleObjects( m_handles.size(), m_handles.data(), FALSE, INFINITE);

    if (!CheckWaitResult(m_handles.size(), result))
    {
        m_last_error = GetLastError();
        CString str;
        str.Format(L"CMultiEvents::Wait() failed. Error %u\n", m_last_error);
        LOG_SYNC_OBJECT(str);
        return E_SO_WAIT_FAIL;
    }

    return result;
}


int CMultiEvents::Reset(DWORD index)
{
    int retval = E_SO_OK;

    if (index >= m_handles.size())
        retval = E_SO_RESET_FAIL_INDEX;
    else if (!ResetEvent(m_handles[ index ]))
        retval = E_SO_RESET_FAIL;

    if (retval != E_SO_OK)
    {
        m_last_error = GetLastError();
        CString str;
        str.Format(L"CMultiEvents::Reset() failed. Error %u\n", m_last_error);
        LOG_SYNC_OBJECT(str);
    }

    return retval;
}






Sing::Sing()
{
    m_refcount++;
    m_name.Format(L"slotsSem-%-u", m_refcount);

    // There's only one underlying semaphore object for all instances
    if (m_hSem == INVALID_HANDLE_VALUE)
    {
        LOG_SYNC_OBJECT(L"Creating underlying slot semaphore\n");
        m_hSem = CREATE_SEMAPHORE(
            NULL,               // Default security attributes
            1, 1,               // Initial count, Maximum count
            m_name);

        if (m_hSem == NULL) {
            m_last_error = GetLastError();
            LOG_SYNC_OBJECT(L"Can't create slots semaphore\n");
        }
        else
            LOG_SYNC_OBJECT(L"Base CSlotSem semaphore handle created\n");
    }
    LOG_SYNC_OBJECT(m_name + L" created\n");
}

Sing::~Sing()
{
    CString str;
    str.Format(L"Destroying Sing refcount %u\n", m_refcount);
    LOG_SYNC_OBJECT(str);

    m_refcount--;
    if (m_hSem)
        CloseHandle(m_hSem);
    m_hSem = INVALID_HANDLE_VALUE;
}

Sing& Sing::getInstance()
{
    static Sing instance;      // Guaranteed to be destroyed. Instantiated on first use.
    return instance;
}

bool Sing::Lock()
{
    ASSERT(m_hSem != INVALID_HANDLE_VALUE);

    DWORD result = WaitForSingleObject(m_hSem, SLOT_LOCK_TIMEOUT);
    if (CheckWaitResult(1, result))
        return true;

    m_last_error = GetLastError();

    CString str;
    str.Format(L"CslotsSem::Lock() wait fail. Error %u. Count %lu\n", m_last_error, m_refcount);

    LOG_SYNC_OBJECT(L"CslotsSem::Lock() wait fail\n");
    return false;
}

bool Sing::Unlock()
{
    static LONG last_count = -1;

    ASSERT(m_hSem != INVALID_HANDLE_VALUE);

    if (ReleaseSemaphore(m_hSem, 1, &last_count) != 0)
        return true;

    m_last_error = GetLastError();

    CString str;
    str.Format(L"CslotsSem::Unlock() release fail. Error %u. Count %lu\n", m_last_error, last_count);
    LOG_SYNC_OBJECT(str);

    return false;
}
