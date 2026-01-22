#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "Cslot.hpp"
#include "CdownloadManager.hpp"
#include "model.hpp"
#include "CcurlJob.hpp"
#include "xmlParse.hpp"
#include "CsyncObjects.hpp"
#include "threadData.hpp"
#include "threadFuncs.hpp"






#if (SAVE_WEBPAGE_ON_ERROR==1)
void SaveWebPage(const cCurlJob& curljob);
#else
#define     SaveWebPage(x)      do{} while(0)
#endif




extern Cslots xxSlots;




/**
 * Each worker thread handles one slot. The thread waits on a single request event.
 * 
 */
UINT __cdecl thrSlotThread(LPVOID pParam)
{
	Cslot&			slot   = * static_cast<Cslot*>(pParam);
	eThreadResult	retval ;

	TRACK_DYNAMIC_OBJECTS(L"thrSlotThread created");

	slot.m_thread_state = eThreadState::TS_RUNNING;

	while (true)
	{
		// Wait for a new request event
		DWORD wait_result = WaitForSingleObject(slot.GetRequestHandle(), INFINITE);
		if (wait_result != WAIT_OBJECT_0)
		{
			// If the Wait fails - just loop & try again.
			WriteDebugConsole(L"thrSlotThread WAIT_FAIL!");
			continue;
		}

		slot.m_slotstate = eSlotState::SS_JOB_THREAD;
		retval = eThreadResult::TR_OK;

		// Terminate flag set? Exit the thread.
		if (slot.m_exit_thread) {
			slot.m_slotstate = eSlotState::SS_THREAD_EXITING;
			slot.m_thread_state = eThreadState::TS_FINISHED;
			TRACK_DYNAMIC_OBJECTS(L"thrSlotThread exiting");
			return eThreadResult::TR_NORMAL_EXIT;
		}

		// Download the webpage from the show URL
		cCurlJob  curljob(slot.m_show.epguides_url);
		bool curl_ok = curljob.downloadShow();

		// Handy status/error codes for debugging.
		slot.m_curl_status = curljob.m_curl_result;
		slot.m_http_status = curljob.m_http_response;

		if (curl_ok == false)
		{
			// In debug mode, optionally save a local copy of the faulting page (see config flag).
			SaveWebPage(curljob);

			std::ostringstream os;
			os << "CURL code " << curljob.m_curl_result << ", HTTP response " << curljob.m_http_response;
			slot.m_error_string = os.str();
			retval = eThreadResult::TR_CURL_ERROR;
		}
		else
		{
			// No download errors. Parse the HTML into the show object
			sXmlErrorInfo xml_error_info;
			slot.m_xml_status = xmlParse(slot.m_show, curljob, xml_error_info);
			if (slot.m_xml_status != E_XPARSE_OK)
			{
				// In debug mode, optionally save a local copy of the faulting page (see config flag).
				SaveWebPage(curljob);

				if (slot.m_xml_status == E_XPARSE_DOC_ERROR) {
					slot.m_error_string = "xmlParse doc error";
					retval = eThreadResult::TR_DOC_ERROR;
				}
				else if (slot.m_xml_status == E_XPARSE_PAGE_ERROR) {
					slot.m_error_string = "xmlParse page format error";
					retval = eThreadResult::TR_PARSE_ERROR;
				}
				else {
					slot.m_error_string = "xmlParse unknown error";
					retval = eThreadResult::TR_XML_ERROR;
				}
			}
		}

		// Save the status & Signal that we're all done
		slot.SetThreadResult(retval);
		slot.SetState(eSlotState::SS_RESULTS_READY);
		slot.SignalResult();

	}
}




/**
 * Handle evRequest events from the main UI task from the CdownloadManager object
 * 
 */
 UINT __cdecl thrRequests( LPVOID pParam )
{
	cRequests&		requests = * static_cast<cRequests*>(pParam);

	// Lock objects last the life of the thread. Only two events to watch.
	CMultiEvents	events(requests.Handles(), 2);
	CslotsSem		slotslock;
	int				freeslot;

	TRACK_DYNAMIC_OBJECTS(L"thrRequests starting");

	while (true)
	{
		// Wait forever, on any event (there are only two!)
		DWORD wait_result = events.Wait();

		if (wait_result == E_SO_WAIT_FAIL) {
			WriteDebugConsole(L"theRequests Wait failed");
			continue;
		}

		// Terminate event? (auto-reset)
		if (wait_result == WAIT_OBJECT_0) {
			TRACK_DYNAMIC_OBJECTS(L"thrRequests exiting");
			return eThreadResult::TR_NORMAL_EXIT;
		}

		ASSERT(wait_result == (WAIT_OBJECT_0 + 1));

		// There is at least one URL request in the request_data struct. Find a slot for it
		// If there's no free download slot, loop for the next timeout 
		// Keep popping requests off the queue till all slots are full or there are no more requests

		while (requests.Pending())
		{
			if ((freeslot = xxSlots.FirstFreeSlot()) == -1)
				break;

			std::string str = requests.Pop();		// Auto locks

			if (!slotslock.Lock())
				WriteDebugConsole(L"Can't acquire thrRequest slot semaphore");
			else {
				xxSlots.SetUrl(freeslot, str);
				slotslock.Unlock();
				xxSlots.SignalRequest(freeslot);
			}

		}
	}
}




/**
 * Handle evResults available event from a worker thread. CslotData contains the downloaded show info.
 * Pass the slot information to the database via a (thread safe) windows message.
 *
 */
UINT __cdecl thrResults( LPVOID pParam )
{
	cResults&		results = *static_cast<cResults*>(pParam);
	CMultiEvents	events(results.Handles(), results.NumHandles());

	while (true)
	{
		DWORD wait_result = events.Wait();

		if (wait_result == E_SO_WAIT_FAIL) {
			WriteDebugConsole(L"theResults Wait failed");
			continue;
		}

		// Terminate thread event?
		if (wait_result == WAIT_OBJECT_0) {
			TRACK_DYNAMIC_OBJECTS(L"thrResults exiting");
			return eThreadResult::TR_NORMAL_EXIT;
		}

		DWORD slotnum = wait_result - WAIT_OBJECT_0 - 1;
		PostMessage(results.GetMsgWindow(), WM_TVP_DOWNLOAD_PING, slotnum, 0);

		xxSlots.SetState(slotnum, eSlotState::SS_NOTIFY_SENT);
	}
}




/**
 * Handle evRelease event from the database. The slot results have been processed & incorporated into
 * the database. Reset & release the slot.
 *
 */
UINT __cdecl thrReleases( LPVOID pParam )
{
	cReleases&		releases = *static_cast<cReleases*>(pParam);

	CMultiEvents	events(releases.Handles(), releases.NumHandles());
	CslotsSem		slotslock;

	TRACK_DYNAMIC_OBJECTS(L"thrReleases starting");

	while (true)
	{
		DWORD wait_result = events.Wait();

		if (wait_result == E_SO_WAIT_FAIL) {
			WriteDebugConsole(L"theReleases Wait failed");
			continue;
		}

		// 1st handle is the terminate event. The remainder are worker thread events
		if (wait_result == WAIT_OBJECT_0) {
			TRACK_DYNAMIC_OBJECTS(L"thrReleases exiting");
			return eThreadResult::TR_NORMAL_EXIT;
		}

		// Get thread/slot index
		DWORD slotnum = (wait_result - WAIT_OBJECT_0 - 1);

		if (!slotslock.Lock()) {
			WriteDebugConsole(L"Can't acquire thrRelease slots semaphore");
		}
		else {
			releases.ReleaseSlot(slotnum);
			PostMessage(releases.GetMsgWindow(), WM_TVP_SLOT_RELEASED, slotnum, 0);
			if (!slotslock.Unlock())
				WriteDebugConsole(L"Can't release thrRelease slots semaphore");
		}

		VERIFY(events.Reset(wait_result) == E_SO_OK);
	}
}









#if (SAVE_WEBPAGE_ON_ERROR==1)

void SaveWebPage(const cCurlJob& curljob)
{
	CFile cfile;
	cfile.Open(L"webpage.txt", CFile::modeCreate | CFile::modeWrite);
	cfile.Write(&curljob.m_page[0], curljob.m_page.size());
	cfile.Flush();
	cfile.Close();
}

#endif

