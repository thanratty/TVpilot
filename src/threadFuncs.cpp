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




bool CurlAndParse( Cslot& slot );





/**
 * Each worker thread handles one slot. The thread waits on a single request event.
 * 
 */
UINT __cdecl thrSlotThread(LPVOID pParam)
{
	Cslot&			slot	  = *static_cast<Cslot*>(pParam);
	CslotsSem&		slotslock = CslotsSem::getInstance();

	slot.SetThreadState(eThreadState::TS_RUNNING);

	LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread %u created\n", slot.m_SlotNumber);

	while (true)
	{
		slot.SetThreadState(eThreadState::TS_WAITING);

		//
		// Wait for valid request event
		//
		DWORD wait_result = WaitForSingleObject(slot.GetRequestHandle(), INFINITE);
		slot.SetThreadState(eThreadState::TS_RUNNING);
		if (wait_result != WAIT_OBJECT_0)
		{
			// If the Wait fails - just loop & try again.
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread wait failed\n");
			continue;
		}


		//
		// Terminate flag set? Exit the thread
		//
		if (slot.GetExitFlag())
		{
			LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread %u exiting\n", slot.m_SlotNumber);

			slot.SetSlotState(eSlotState::SS_THREAD_EXITING);
			slot.SetThreadState(eThreadState::TS_FINISHED);
			return eThreadResult::TR_UNKNOWN;
		}


		//
		// Got a good event. Check slot state to determine what to do next
		//
		eSlotState state = slot.GetSlotState();
		switch (state)
		{
			case eSlotState::SS_URL_SET:
				{
					bool curl_ok = CurlAndParse(slot);
					if (curl_ok) {
						slot.SetSlotState(eSlotState::SS_RESULTS_READY);
					}
					::PostMessage(slot.GetMsgWin(), WM_TVP_DOWNLOAD_PING, slot.m_SlotNumber, 0);
				}
				break;

			case eSlotState::SS_RESULTS_PROCESSED:
				if (!slotslock.Lock()) {
					LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread %u slot lock fail\n", slot.m_SlotNumber);
				}
				else {
					slot.ResetAndFree();
					if (!slotslock.Unlock())
						LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread %u slot release fail\n", slot.m_SlotNumber);
					::PostMessage(slot.GetMsgWin(), WM_TVP_SLOT_RELEASED, slot.m_SlotNumber, 0);
				}
				break;

			default:
				LOG_PRINT(eLogFlags::THREAD_FUNC, L"thrSlotThread %u : Unhandled slotstate %u\n", slot.m_SlotNumber, state);
				break;
		}

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





bool CurlAndParse(Cslot& slot)
{
	slot.SetSlotState(eSlotState::SS_CURLING);

	cCurlJob  curljob(slot.m_show.epguides_url);
	bool curl_ok = curljob.downloadShow();

	// Handy status/error codes for debugging.
	slot.m_curl_status = curljob.m_curl_result;
	slot.m_http_status = curljob.m_http_response;

	if (curl_ok == false)
	{
		slot.SetSlotState(eSlotState::SS_CURL_ERROR);
		SAVE_WEB_PAGE(curljob);

		std::ostringstream os;
		os << "CURL code " << curljob.m_curl_result << ", HTTP response " << curljob.m_http_response;
		slot.m_error_string = os.str();
		return false;
	}

	slot.SetSlotState(eSlotState::SS_PARSING);

	// No download errors. Parse the HTML into the show object
	sXmlErrorInfo xml_error_info;
	slot.m_xml_status = xmlParse(slot.m_show, curljob, xml_error_info);
	if (slot.m_xml_status != E_XPARSE_OK)
	{
		slot.SetSlotState(eSlotState::SS_PARSE_ERROR);
		SAVE_WEB_PAGE(curljob);

		if (slot.m_xml_status == E_XPARSE_DOC_FORMAT_ERROR) {
			slot.m_error_string = "xmlParse doc error";
		}
		else if (slot.m_xml_status == E_XPARSE_PAGE_FORMAT_ERROR) {
			slot.m_error_string = "xmlParse page format error";
		}

		return false;
	}


	slot.SetSlotState(eSlotState::SS_PARSED_OK);
	return true;
}

