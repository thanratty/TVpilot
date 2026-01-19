#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "CsyncObjects.hpp"
#include "Cslot.hpp"
#include "threadData.hpp"
#include "threadFuncs.hpp"
#include "utils.hpp"

#include "CdownloadManager.hpp"


extern std::vector<Cslot>  gSlots;







/**
 * Will always pass - requests get queued by the Request task/thread
 *
 */
void CdownloadManager::DownloadShow(const std::string& url)
{
	requests.Push(url);
}




void CdownloadManager::SetMsgWindow(HWND hMsgWindow)
{
	m_hMsgWindow = hMsgWindow;
	results.SetMsgWindow(hMsgWindow);
}




bool CdownloadManager::DownloadInProgress() const
{
bool retval = false;

	if (requests.Pending()) {
		retval = true;
	}
	else if (FirstBusySlot() != -1) {
			retval = true;
	}

	return retval;
}




/// TODO This doesnt work reliably
//
void CdownloadManager::AbortDownload()
{
	// Clear all queued URLs then wait in the
	// ping handler till all slots are free.

	requests.ClearQueue();	// TODO Separate abort function ???
}



void CdownloadManager::OnSlotReleased(DWORD slotnum)
{
	UNREFERENCED_PARAMETER(slotnum);

	if (requests.Pending())
		requests.Trigger();
}


