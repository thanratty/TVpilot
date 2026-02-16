#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "utils.hpp"
#include "logging.hpp"

#include "CsyncObjects.hpp"





constexpr DWORD SLOT_LOCK_TIMEOUT = 5000;








CMultiEvents::CMultiEvents(const std::vector<HANDLE>&handles)
{
    LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CMultiEvents() constructor");

    m_handles.insert(m_handles.end(), handles.begin(), handles.end());

    LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CMultiEvents created with %u handles\n", m_handles.size());
}


CMultiEvents::~CMultiEvents()
{
    LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CMultiEvents() destructor\n");
}


// Returns the index of the signalled event, or E_SO_WAIT_FAIL
//
int CMultiEvents::Wait()
{
    DWORD result = WaitForMultipleObjects( m_handles.size(), m_handles.data(), FALSE, INFINITE);

    if (!CheckWaitResult(result))
    {
        m_last_error = GetLastError();
        LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CMultiEvents::Wait() failed. Error %u\n", m_last_error);
        return E_SO_WAIT_FAIL;
    }

    return result;
}


/**
 * Check the return from WaitForMultipleObjects is in range
 */
bool CMultiEvents::CheckWaitResult(DWORD result) const
{
    if ((result >= WAIT_OBJECT_0) && (result < WAIT_OBJECT_0 + m_handles.size()))
        return true;

    // WAIT_TIMEOUT  WAIT_FAILED.   WAIT_ABANDONED (mutexes only?)

    LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CMultiEvents wait failed: %08X\n", result);
    return false;
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
        LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CMultiEvents::Reset() failed. Error %u\n", m_last_error);
    }

    return retval;
}






CslotsSem::CslotsSem()
{
    LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CslotSem constructor\n");

    // There's only one underlying semaphore object for all instances
    if (m_hSem == INVALID_HANDLE_VALUE)
    {
        LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"Creating underlying slots semaphore\n");
        m_hSem = CREATE_SEMAPHORE( NULL, 1, 1, m_name);     // Initial count 1, max count 1

        if (m_hSem == NULL) {
            m_last_error = GetLastError();
            LOG_PRINT(eLogFlags::FATAL, L"Can't create slots semaphore. Error %08X\n", m_last_error);
        }
    }
    else
    {
        LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"Slots semaphore already created\n");
    }
}


CslotsSem::~CslotsSem()
{
    LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CslotsSem destructor\n");

    // There's only one actual semaphore
    if (m_hSem != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hSem);
        m_hSem = INVALID_HANDLE_VALUE;
    }
    else
        LOG_PRINT(eLogFlags::SYNC_OBJECTS, L"CslotsSem destructor. m_hSem already closed");
}


CslotsSem& CslotsSem::getInstance()
{
    LOG_PRINT(eLogFlags::SLOT_LOCK, L"CslotSem::getInstance()\n");

    static CslotsSem instance;      // Guaranteed to be destroyed. Instantiated on first use.
    return instance;
}


bool CslotsSem::Lock()
{
    ASSERT(m_hSem != INVALID_HANDLE_VALUE);

    DWORD result = WaitForSingleObject(m_hSem, SLOT_LOCK_TIMEOUT);
    if (CheckWaitResult(result))
        return true;

    m_last_error = GetLastError();

    CString errmsg;
    errmsg.Format(L"FATAL! CslotsSem::Lock() fail. Result %08X, Error %08X\n", result, m_last_error);
    LogMsgWin( errmsg );
    LOG_PRINT( eLogFlags::FATAL, errmsg );

    return false;
}


bool CslotsSem::CheckWaitResult(DWORD result)
{
    if (result == WAIT_OBJECT_0)
        return true;

    m_last_error = GetLastError();

    CString errmsg;
    errmsg.Format(L"FATAL! CslotsSem::CheckWaitResult() fail. Result %08X, Error %08X\n", result, m_last_error);
    LogMsgWin( errmsg );
    LOG_PRINT( eLogFlags::FATAL, errmsg );

    return false;
}


bool CslotsSem::Unlock()
{
    static LONG last_count = -1;

    ASSERT(m_hSem != INVALID_HANDLE_VALUE);

    if (ReleaseSemaphore(m_hSem, 1, &last_count) != 0)
        return true;

    m_last_error = GetLastError();
    LOG_PRINT(eLogFlags::FATAL, L"CslotsSem::Unlock() release fail. Error %08X. Count %ld\n", m_last_error, last_count);

    return false;
}
