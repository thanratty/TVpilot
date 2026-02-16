#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "CsyncObjects.hpp"
#include "Cslots.hpp"
#include "utils.hpp"
#include "logging.hpp"

#include "CdownloadManager.hpp"



//extern Cslots gSlots;



CdownloadManager::CdownloadManager()
{
}


CdownloadManager::~CdownloadManager()
{
	// Individual slots shutdown their own threads & close event handles.
}


void CdownloadManager::Push(const std::string& url)
{
	m_url_queue.push(url);
}

std::string CdownloadManager::Pop()
{
	std::string url;
	
	if (m_url_queue.size() > 0) {
		url = m_url_queue.front();
		m_url_queue.pop();
	}
	else {
		LogMsgWin(L"Download Pop on empty URL queue");
	}

	return url;
}



void CdownloadManager::ReleaseSlot(int slotnum)
{
	Cslots::ReleaseSlot(slotnum);

	if (m_url_queue.size() == 0)
		return;

	std::string url = Pop();
	DownloadShow(url);
}




/**
 * Will always pass - requests get queued by the Request task/thread
 *
 */
void CdownloadManager::DownloadShow(const std::string& url)
{
	// Don't add anything new if we're mid-abort
	if (m_abort_pending)
		return;

	CslotsSem& slotslock = CslotsSem::getInstance();
	int freeslot;

	// Any requests in the queue, find a slot for the & signal a request for that slot
	if (slotslock.Lock())
	{
		if ((freeslot = FirstFreeSlot()) != -1)
		{
			SetUrl(freeslot, url);
			SetSlotState(freeslot, eSlotState::SS_URL_SET);
			SignalRequest(freeslot);
		}
		else
			Push(url);

		slotslock.Unlock();
	}
}




void CdownloadManager::SetMsgWin(HWND hMsgWin)
{
	m_hMsgWin = hMsgWin;

	Cslots::SetMsgWin( hMsgWin );
}




bool CdownloadManager::DownloadInProgress() const
{
	if (FirstBusySlot() != -1)
			return true;

	return false;
}




void CdownloadManager::AbortDownload()
{
	m_abort_pending = true;

	while(!m_url_queue.empty())
		m_url_queue.pop();
}



void CdownloadManager::ClearAbortCondition()
{
	m_abort_pending = false;

	for (unsigned i=0 ; i<NUMBER_OF_DOWNLOAD_THREADS ; i++)
		Cslots::ReleaseSlot(i);
}




void CdownloadManager::TerminateSlotThreads()
{
	Cslots::TerminateSlotThreads();

	while (!AllSlotThreadsTerminated());
}
