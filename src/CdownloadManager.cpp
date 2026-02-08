#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "CsyncObjects.hpp"
#include "Cslots.hpp"
#include "utils.hpp"

#include "CdownloadManager.hpp"



//extern Cslots gSlots;



CdownloadManager::CdownloadManager()
{
}


CdownloadManager::~CdownloadManager()
{
	// Stop the slot threads first

	TerminateSlotThreads();

}


/**
 * Will always pass - requests get queued by the Request task/thread
 *
 */
void CdownloadManager::DownloadShow(const std::string& url)
{
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
bool retval = false;

	if (FirstBusySlot() != -1)
			retval = true;

	return retval;
}




/// TODO This doesnt work reliably
//
void CdownloadManager::AbortDownload()
{
	// Clear all queued URLs then wait in the
	// ping handler till all slots are free.

	// TODO - Lock reset all slots with SS_URL
}


