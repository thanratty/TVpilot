#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "CslotData.hpp"
#include "CdownloadManager.h"
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








/**
 * Each worker thread handles one slot
 * 
 */
UINT __cdecl thrWorkerThread(LPVOID pParam)
{
UINT	status = E_THREAD_OK;
int		index;

	CslotData&		slot = * (CslotData*) pParam;
	CMultiEvents	request(&slot.evRequest, 1);

	while (true)
	{
		// Wait for a new request
		if ((index=request.Wait()) != E_WAIT_FAIL)
		{
			// Terminate flag set? Exit the thread.
			if (slot.TerminateRequested())
				return E_THREAD_OK;

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
				status = E_THREAD_CURL;
			}
			else
			{
				// No download errors. Parse the HTML & extract the show info.
				sXmlErrorInfo xml_error_info;
				slot.m_xml_status = xmlParse(slot.m_show, curljob, xml_error_info);
				if (slot.m_xml_status != E_XPARSE_OK)
				{
					// In debug mode, optionally save a local copy of the faulting page (see config flag).
					SaveWebPage(curljob);

					if (slot.m_xml_status == E_XPARSE_DOC_ERROR) {
						slot.m_error_string = "xmlParse doc error";
						status = E_THREAD_DOC_ERR;
					}
					else if (slot.m_xml_status == E_XPARSE_PAGE_ERROR) {
						slot.m_error_string = "xmlParse page format error";
						status = E_THREAD_PARSE;
					}
					else {
						slot.m_error_string = "xmlParse unknown error";
						status = E_THREAD_XML;
					}
				}
			}

			// Save the status & Signal that we're all done
			slot.SetThreadStatus(status);
			if (!SetEvent(slot.evResults))
				WriteDebugConsole(L"Can't set worker evResults event");
		}
		else
		{
			AfxDebugBreak();
		}

		VERIFY(request.Reset(index) == 0);
	}
}




/**
 * Handle evRequest events from the main UI task from the CdownloadManager object
 * 
 */
 UINT __cdecl thrRequest( LPVOID pParam )
{
int slotnum;

	cRequestData&	request_data = * static_cast<cRequestData*>(pParam);
	CslotData*		slots = request_data.Slots();

	// Lock objects last the life of the thread
	CMultiEvents	events(request_data.Handles(), 2);
	CslotsSem		slotslock;


	while (true)
	{
		// Wait forever, on any event (there are only two!)
		DWORD index = events.Wait();

		// Terminate event?
		if (index == 0)
			return E_THREAD_OK;

		ASSERT(index == (WAIT_OBJECT_0 + 1));

		// There is at least one URL request in the request_data struct. Find a slot for it
		// If there's no free download slot, loop for the next timeout 
		// Keep popping requests off the queue till all slots are full or there's no more requests

		if (!request_data.SemLock())
			WriteDebugConsole(L"Can't acquire request_data semaphore");
		if (!slotslock.SemLock())
			WriteDebugConsole(L"Can't acquire thrRequert slots semaphore");

		while (request_data.Pending())
		{
			slotnum = FindFreeSlot(slots);
			if (slotnum == -1)
			    break;

			std::string str = request_data.Pop();
			slots[ slotnum ].SetUrl(str);
			if (!SetEvent(slots[ slotnum ].evRequest))
				WriteDebugConsole(L"Can't set thrRequest slot event");
		}

		if (!slotslock.SemUnlock())
			WriteDebugConsole(L"Can't release thrRequest slots semaphore");

		if (!request_data.SemUnlock())
			WriteDebugConsole(L"Can't release request_data semaphore");

		if (!events.Reset(index)==0)
			WriteDebugConsole(L"Can't reset thrRequest event");
	}
}




/**
 * Handle evResult event from a worker thread. CslotData contains the downloaded show info.
 * Pass the slot information to the database via a wimdows message.
 *
 */
UINT __cdecl thrResults( LPVOID pParam )
{
	cResultsData*	pData = static_cast<cResultsData*>(pParam);
	CMultiEvents	events(pData->Handles(), pData->NumHandles());

	while (true)
	{
		DWORD wait_result = events.Wait();

		// Terminate thread event?
		if (wait_result == WAIT_OBJECT_0)
			return E_THREAD_OK;

		if(!pData->SemLock())
			WriteDebugConsole(L"Can't acquire cResultsData semaphore");

		// Check every slot event before unlocking. slot # 0 = event index 1
		for (DWORD slotnum = 0 ; slotnum < NUM_WORKER_THREADS ; slotnum++)
		{
			if (events.IsSignalled(slotnum + 1))
			{
				PostMessage(pData->GetMessageWindow(), WM_DOWNLOAD_PING, slotnum, 0);
				VERIFY(events.Reset(slotnum + 1) == 0);
			}
		}

		if (!pData->SemUnlock())
			WriteDebugConsole(L"Can't acquire cResultsData semaphore");
	}
}




/**
 * Handle evRelease event from the database. the slot results have been processed & incorporated into
 * the database. Reset & release the slot.
 *
 */
UINT __cdecl thrRelease( LPVOID pParam )
{
	cReleaseData*	pData = static_cast<cReleaseData*>(pParam);

	CMultiEvents	events(pData->Handles(), NUM_WORKER_THREADS+1 );
	CslotsSem		slotslock;

	while (true)
	{
		DWORD wait_result = events.Wait();

		if (wait_result == WAIT_OBJECT_0)
			return E_THREAD_OK;

		DWORD slot = (wait_result - WAIT_OBJECT_0 - 1);

		if (!slotslock.SemLock())
			WriteDebugConsole(L"Can't acquire thrRelease slots semaphore");

		pData->ResetAndFree( slot );

		if (!slotslock.SemUnlock())
			WriteDebugConsole(L"Can't release thrRelease slots semaphore");

		VERIFY(events.Reset(wait_result)==0);
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

