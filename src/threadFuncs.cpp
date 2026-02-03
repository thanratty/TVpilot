#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "Cslots.hpp"
#include "CcurlJob.hpp"
#include "xmlParse.hpp"
#include "CsyncObjects.hpp"
#include "threadData.hpp"
#include "utils.hpp"
#include "logging.hpp"

#include "threadFuncs.hpp"





#if (SAVE_WEBPAGE_ON_ERROR==1) && defined(_DEBUG)
void SAVE_WEB_PAGE(const cCurlJob& curljob);
#else
#define     SAVE_WEB_PAGE(x)      do{} while(0)
#endif



// _THE_ global slots array object
//
extern Cslots gSlots;



/**
 * Each worker thread handles one slot. The thread waits on a single request event.
 * 
 */
UINT __cdecl thrSlotThread(LPVOID pParam)
{
	Cslot&			slot   = * static_cast<Cslot*>(pParam);
	eThreadResult	retval;

	LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread %u created\n", slot.m_SlotNumber);

	slot.SetThreadState(eThreadState::TS_RUNNING);

	while (true)
	{
		retval = eThreadResult::TR_OK;
		slot.SetThreadState(eThreadState::TS_WAITING);

		// Wait for a new request event
		DWORD wait_result = WaitForSingleObject(slot.GetRequestHandle(), INFINITE);
		slot.SetThreadState(eThreadState::TS_RUNNING);
		if (wait_result != WAIT_OBJECT_0)
		{
			// If the Wait fails - just loop & try again.
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread wait failed\n");
			continue;
		}

		// We own this slot, no need to lock it
		slot.SetState(eSlotState::SS_JOB_AWAKE);
	
		// Terminate flag set? Exit the thread.
		if (slot.GetExitFlag())
		{
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread %u exiting\n", slot.m_SlotNumber);

			slot.SetState(eSlotState::SS_THREAD_EXITING);
			slot.SetThreadState(eThreadState::TS_FINISHED);
			slot.SetThreadResult(eThreadResult::TR_NORMAL_EXIT);
			return eThreadResult::TR_NORMAL_EXIT;
		}

		// Download the webpage from the show URL
		slot.SetThreadState(eThreadState::TS_CURLING);
		cCurlJob  curljob(slot.m_show.epguides_url);
		bool curl_ok = curljob.downloadShow();

		// Handy status/error codes for debugging.
		slot.m_curl_status = curljob.m_curl_result;
		slot.m_http_status = curljob.m_http_response;

		if (curl_ok == false)
		{
			slot.SetState(eSlotState::SS_DOWNLOAD_ERROR);
			SAVE_WEB_PAGE(curljob);

			std::ostringstream os;
			os << "CURL code " << curljob.m_curl_result << ", HTTP response " << curljob.m_http_response;
			slot.m_error_string = os.str();
			retval = eThreadResult::TR_CURL_ERROR;
		}
		else
		{
			slot.SetThreadState(eThreadState::TS_SCANNING);

			// No download errors. Parse the HTML into the show object
			sXmlErrorInfo xml_error_info;
			slot.m_xml_status = xmlParse(slot.m_show, curljob, xml_error_info);
			if (slot.m_xml_status != E_XPARSE_OK)
			{
				// In debug mode, optionally save a local copy of the faulting page (see config.h).
				SAVE_WEB_PAGE(curljob);

				if (slot.m_xml_status == E_XPARSE_DOC_FORMAT_ERROR) {
					LOG_PRINT(eLogFlags::XML, L"E_XPARSE_DOC_FORMAT_ERROR: %s\n", slot.m_show.epguides_url.c_str());
					slot.m_error_string = "xmlParse doc error";
					retval = eThreadResult::TR_DOC_ERROR;
				}
				else if (slot.m_xml_status == E_XPARSE_PAGE_FORMAT_ERROR) {
					LOG_PRINT(eLogFlags::XML, L"E_XPARSE_PAGE_FORMAT_ERROR: %s\n", slot.m_show.epguides_url.c_str());
					slot.m_error_string = "xmlParse page format error";
					retval = eThreadResult::TR_PARSE_ERROR;
				}
				else {
					LOG_PRINT(eLogFlags::XML, L"xmlParse() returned %d\n", slot.m_xml_status);
					slot.m_error_string = "xmlParse unknown error";
					retval = eThreadResult::TR_XML_ERROR;
				}
			}
		}

		// Save the status & Signal that we're all done
		slot.SetThreadResult(retval);					// Bad name. 'LastJobResult' maybe? etc
		slot.SetState(eSlotState::SS_RESULTS_READY);	// Expand name... SetSlotState(...)
		slot.SignalResult();
	}
}




/**
 * Handle evRequest events from the main UI task from the CdownloadManager object
 * 
 */
 UINT __cdecl thrRequests( LPVOID pParam )
{
	cRequests&		requests = *static_cast<cRequests*>(pParam);

	// Lock objects last the life of the thread. Only two events to watch.
	CMultiEvents	events(requests.Handles());
	int				freeslot;
	CslotsSem&		slotslock = CslotsSem::getInstance();

	LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrRequests starting\n");

	while (true)
	{
		// Wait forever, on any event (there are only two!)
		DWORD wait_result = events.Wait();

		if (wait_result == E_SO_WAIT_FAIL) {
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"theRequests Wait failed\n");
			continue;
		}

		// Terminate event? (auto-reset)
		if (wait_result == WAIT_OBJECT_0) {
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrRequests exiting\n");
			return eThreadResult::TR_NORMAL_EXIT;
		}

		// Any requests in the queue, find a slot for the & signal a request for that slot
		if (requests.RequestsPending() && slotslock.Lock())
		{
			while (requests.RequestsPending())
			{
				if ((freeslot = gSlots.FirstFreeSlot()) == -1)
					break;

				std::string str = requests.Pop();		// Auto locks the request queue
				gSlots.SetUrl(freeslot, str);
				gSlots.SignalRequest(freeslot);
			}
			slotslock.Unlock();
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
	CMultiEvents	events(results.Handles());

	LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrResults starting\n");

	while (true)
	{
		DWORD wait_result = events.Wait();

		if (wait_result == E_SO_WAIT_FAIL) {
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrResults Wait failed\n");
			continue;
		}

		// Terminate thread event?
		if (wait_result == WAIT_OBJECT_0) {
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrResults exiting\n");
			return eThreadResult::TR_NORMAL_EXIT;
		}

		// Notify the app the download is done, good or bad.
		DWORD slotnum = wait_result - WAIT_OBJECT_0 - 1;
		PostMessage(results.GetMsgWindow(), WM_TVP_DOWNLOAD_PING, slotnum, 0);

		gSlots.SetState(slotnum, eSlotState::SS_NOTIFY_SENT);
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
	CMultiEvents	events(releases.Handles());
	CslotsSem&		slotslock = CslotsSem::getInstance();

	LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrReleases starting\n");

	while (true)
	{
		DWORD wait_result = events.Wait();

		if (wait_result == E_SO_WAIT_FAIL) {
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrReleases Wait failed\n");
			continue;
		}

		// 1st handle is the terminate event. The remainder are worker thread events
		if (wait_result == WAIT_OBJECT_0) {
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrReleases exiting\n");
			return eThreadResult::TR_NORMAL_EXIT;
		}

		// Get thread/slot index
		DWORD slotnum = (wait_result - WAIT_OBJECT_0 - 1);

		if (!slotslock.Lock()) {
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrRelease slots lock fail\n");
		}
		else {
			releases.ReleaseSlot(slotnum);
			PostMessage(releases.GetMsgWindow(), WM_TVP_SLOT_RELEASED, slotnum, 0);
			if (!slotslock.Unlock())
				LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrRelease slots unlock fail\n");
		}

		VERIFY(events.Reset(wait_result) == E_SO_OK);
	}
}









#if (SAVE_WEBPAGE_ON_ERROR==1)

void SAVE_WEB_PAGE(const cCurlJob& curljob)
{
	CFile cfile;
	cfile.Open(L"webpage.txt", CFile::modeCreate | CFile::modeWrite);
	cfile.Write(&curljob.m_page[0], curljob.m_page.size());
	cfile.Flush();
	cfile.Close();

	LOG_PRINT(eLogFlags::THREAD_FUNC, L"webpage.txt saved\n");
}

#endif

