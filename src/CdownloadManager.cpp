#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "CsyncObjects.hpp"
#include "threadData.hpp"
#include "threadFuncs.hpp"
#include "utils.hpp"

#include "CdownloadManager.h"






CdownloadManager::~CdownloadManager()
{
	// Terminate all the worker threads
	//
	for(auto i=0 ; i< NUM_WORKER_THREADS ; i++)
		m_slots[i].TerminateThread();

	// Terminate the three download-event handler threads
	//
	request_data.TerminateThread();
	results_data.TerminateThread();
	release_data.TerminateThread();
}




CdownloadManager::CdownloadManager() :
	request_data(m_slots),
	results_data(m_slots),
	release_data(m_slots)
{
	CWinThread* thread;
	CString		str;

	// Start the worker threads (m_slots is initialised by now)
	//
	for (auto i=0 ; i < NUM_WORKER_THREADS ; i++)
	{
		thread = AfxBeginThread(thrWorkerThread, &m_slots[i]);
		ASSERT(thread);
		str.Format(L"slot-%-u", i);
		SetThreadDescription(thread->m_hThread, str);
	}


	// Start the three event handler threads
	//
	m_thrRequest = AfxBeginThread(thrRequest, &request_data);
	ASSERT(m_thrRequest);
	SetThreadDescription(m_thrRequest->m_hThread, L"thrRequest");

	m_thrResults = AfxBeginThread(thrResults, &results_data);
	ASSERT(m_thrResults);
	SetThreadDescription(m_thrResults->m_hThread, L"thrResults");

	m_thrRelease = AfxBeginThread(thrRelease, &release_data);
	ASSERT(m_thrRelease);
	SetThreadDescription(m_thrRelease->m_hThread, L"thrRelease");
};





/**
 * Will always pass - requests get queued by the Request task/thread
 *
 */
void CdownloadManager::DownloadShow(const std::string& url)
{
	request_data.SemLock();
	request_data.Push(url);
	request_data.SemUnlock();

	request_data.Trigger();
}




void CdownloadManager::SetMsgWindow(HWND hMsgWindow)
{
	m_hMsgWindow = hMsgWindow;
	results_data.SetMsgWindow(hMsgWindow);
}




bool CdownloadManager::DownloadInProgress() const
{
bool retval = true;

	CslotsSem		slotslock;
	
	request_data.SemLock();

	if (request_data.Pending()) {
		retval = false;
	}
	else
	{
		slotslock.SemLock();
		retval = (FindBusySlot(m_slots) == -1);
		slotslock.SemUnlock();
	}

	request_data.SemUnlock();

	return !retval;
}




void CdownloadManager::AbortDownload()
{
	// Clear all queued URLs then wait in the
	// ping handler till all slots are free.

	request_data.SemLock();
	request_data.ClearQueue();
	request_data.SemUnlock();
}

