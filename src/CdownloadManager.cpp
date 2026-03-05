#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "CsyncObjects.hpp"
#include "Cslots.hpp"
#include "utils.hpp"

#include "CdownloadManager.hpp"



/**
 * Individual slots now start & shutdown their own threads 
 * and event handles in the constructors/destructors.
 */

CdownloadManager::CdownloadManager()
{}

CdownloadManager::~CdownloadManager()
{}


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
			CONSOLE_PRINT(eLogFlags::SLOT_USE, "Slot %2u downloading %s\n", freeslot, url.c_str());

			SetUrl(freeslot, url);
			SetSlotState(freeslot, eSlotState::SS_URL_SET);
			SignalRequest(freeslot);
		}
		else
		{
			Push(url);
		}

		slotslock.Unlock();
	}
}




bool CdownloadManager::DownloadInProgress() const
{
	// If there's a busy slot, we're mid-download
	return (FirstBusySlot() != -1);
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
	ReleaseAllSlots();
}

