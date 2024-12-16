#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "threadData.hpp"







cRequestData::cRequestData(CslotData* _pslots) :
    pslots(_pslots)
{
    HANDLE handle;

    // It's safe to assume these Win32 Create API calls will succeed

    handle = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermRequest");     // Manual reset, initial state
    handles[0] = handle;

    handle = CREATE_EVENT(NULL, TRUE, FALSE, L"evRequest");          // Auto reset, initial state
    handles[1] = handle;

    sem_request_data = CREATE_SEMAPHORE( NULL, 1, 1, L"semRequestData");
}


cRequestData::~cRequestData()
{
    CloseHandle(handles[0]);
    CloseHandle(handles[1]);
    CloseHandle(sem_request_data);
}











cResultsData::cResultsData(CslotData* _pslots) :
    pslots(_pslots)
{
    HANDLE handle;

    // It's safe to assume these Win32 Create API calls will succeed

    sem_results_data = CREATE_SEMAPHORE(NULL, 1, 1, L"semResultsData");

    handle = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermResults");     // Manual reset, initial state
    handles.push_back(handle);

    for (auto i = 0 ; i < NUM_WORKER_THREADS; i++)
        handles.push_back(  pslots[i].evResults );
}


cResultsData::~cResultsData()
{
    CloseHandle(sem_results_data);
    CloseHandle(handles[0]);
}











cReleaseData::cReleaseData(CslotData* _pslots) :
    pslots(_pslots)
{
HANDLE handle;

    // It's safe to assume these Win32 Create API calls will succeed

    sem_release_data = CREATE_SEMAPHORE(NULL, 1, 1, L"semReleaseData");

    handle = CREATE_EVENT(NULL, FALSE, FALSE, L"evTermRelease");     // Manual reset, initial state
    handles.push_back(handle);

    for (auto i=0 ; i < NUM_WORKER_THREADS ; i++) {
        handles.push_back(pslots[i].evRelease);
    }
}


cReleaseData::~cReleaseData()
{
    CloseHandle(sem_release_data);
    CloseHandle(handles[0]);
}


