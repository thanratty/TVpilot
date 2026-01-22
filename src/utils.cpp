#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <conio.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <map>

#include "boost/algorithm/string/find.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "common.hpp"

#include "CDShows.hpp"
#include "CDSchedule.hpp"
#include "CDInputBox.hpp"
#include "CsortContext.hpp"
#include "Cslots.hpp"
#include "debugConsole.h"

#include "utils.hpp"



using namespace boost;



STATIC CEdit* pLoggingWindow = nullptr;



/**
 * Check if a CMultiEvent wait result is within range for the # events waited on.
 */
bool CheckWaitResult(unsigned numevents, DWORD result)
{
	if ((result >= WAIT_OBJECT_0) && (result < WAIT_OBJECT_0 + numevents))
		return true;

	// WAIT_TIMEOUT  WAIT_FAILED.   WAIT_ABANDONED (mutexes only)

	WriteDebugConsole(L"CheckWaitResult() failed!\n");
	return false;
}



/**
 * Compute a simple hash on the string parameter
 */
DWORD SimpleHash(const std::string& str)
{
    constexpr unsigned int b = 378551;
    unsigned int a = 63689;
    DWORD hash = 0;

    for(const char& c : str)
    {
        hash = hash * a + c;
        a *= b;
    }

    return hash;
}



/**
 * Compute a simple hash on the CString parameter
 */
DWORD SimpleHash(const CString& url)
{
	std::string new_url = CW2A(url, CP_UTF8);
	return SimpleHash(new_url);
}



/**
 * Convert "SS-EE" formatted episode string to a number for temporal ordering
 */
static unsigned EpisodeToNumber(const CString& epstring)
{
	int season{ 0 }, epnum{ 0 };

	int hpos = epstring.Find('-');
	if (hpos != -1)
	{
		season = _ttoi(epstring.Left(hpos));
		epnum  = _ttoi(epstring.Mid(hpos + 1));
	}

	return season * 1000 + epnum;
}



/**
 * Compare/Order two episode-number strings SS-EE
 */
int EpisodeCompareFunc(const CString& episode1, const CString& episode2, bool ascending)
{
	int result{ 0 };

	unsigned ep1 = EpisodeToNumber( episode1 );
	unsigned ep2 = EpisodeToNumber( episode2 );

	if (ep1 < ep2)
		result = -1;
	else if (ep1 > ep2)
		result = 1;

	return (ascending) ? result : -result;
}



/**
 * Compare two text CStrings
 */
int AlphaCompareFunc(const CString& item1, const CString& item2, bool ascending)
{
	int result = item1.Compare(item2);

	return (ascending) ? result : -result;
}



/**
 * Compare two numeric CStrings by value
 */
int NumberCompareFunc(const CString& item1, const CString& item2, bool ascending)
{
	int num1 = _ttoi(item1);
	int num2 = _ttoi(item2);

	return (ascending) ? (num1 - num2) : (num2 - num1);
}




void ShowListStringsToLocal(sShowListEntry* sle)
{
	CString s_title = CA2W(sle->show_title.c_str(), CP_UTF8);
	sle->ui_title = s_title;

	CString s_last(sle->last_airdate_string.c_str());
	sle->ui_last_airdate_string = s_last.Right(2) + s_last.Mid(4, 5) + s_last.Left(4);;

	CString s_next(sle->next_airdate_string.c_str());
	sle->ui_next_airdate_string = s_next.Right(2)+s_next.Mid(4,5) + s_next.Left(4);

	CString s_episode(sle->next_episode_number.c_str());
	sle->ui_next_episode_number = s_episode;

	std::string last = std::to_string(sle->last_airdate.julian_day());
	sle->ui_last_airdate_sort = CString(last.c_str());

	std::string next = std::to_string(sle->next_airdate.julian_day());
	sle->ui_next_airdate_sort = CString(next.c_str());
}



void ScheduleListStringsToLocal(sScheduleListEntry* gle)
{
	const CString aDOW[7] = { L"Sun ", L"Mon ", L"Tue ", L"Wed ", L"Thr ", L"Fri ", L"Sat " };

	CString s_show_title = CA2W(gle->show_title.c_str(), CP_UTF8);
	gle->ui_show_title = s_show_title;

	CString s_airdate_string(gle->airdate_string.c_str());
	gle->ui_airdate_string = aDOW[(gle->airdate.day_of_week())] + s_airdate_string.Right(2) + s_airdate_string.Mid(4, 4);

	CString s_episode_number(gle->episode_number.c_str());
	gle->ui_episode_number = s_episode_number;

	CString s_episode_title = CA2W(gle->episode_title.c_str(), CP_UTF8);
	gle->ui_episode_title = s_episode_title;

	std::string str = std::to_string(gle->airdate.julian_day());
	gle->ui_airdate_sort = CString(str.c_str());
}




/**
 * For the given show ptr, scan it's epsidos list and determine last & next airdates
 */
void EvalShowLastNextDates( show* pshow )
{
	// Prepare default dates
	gregorian::date today = gregorian::day_clock::local_day();
	gregorian::date _last = gregorian::date(gregorian::min_date_time);
	gregorian::date _next = gregorian::date(gregorian::max_date_time);

	for (const episode& ep : pshow->episodes)
	{
		// Track most recent before today
		if ((ep.ep_date > _last) && (ep.ep_date < today))
			_last = ep.ep_date;

		// Track next episode from today
		if ((ep.ep_date < _next) && (ep.ep_date >= today))
			_next = ep.ep_date;
	}

	pshow->last_airdate = _last;
	pshow->next_airdate = _next;
	pshow->last_airdate_string = (_last == gregorian::date(gregorian::min_date_time)) ? "" : gregorian::to_simple_string(_last);
	pshow->next_airdate_string = (_next == gregorian::date(gregorian::max_date_time)) ? "" : gregorian::to_simple_string(_next);
}




void CopyOutShowInfo(sShowListEntry* sle, const show* pshow)
{
	sle->show_title          = pshow->title;
	sle->hash                = pshow->hash;
	sle->epguides_url        = pshow->epguides_url;
	sle->show_state          = pshow->state;
	sle->show_flags          = pshow->flags;
	sle->num_episodes        = pshow->episodes.size();
	sle->last_airdate        = pshow->last_airdate;
	sle->last_airdate_string = pshow->last_airdate_string;
	sle->next_airdate        = pshow->next_airdate;
	sle->next_airdate_string = pshow->next_airdate_string;
	sle->next_episode_number = pshow->next_episode_number;
}



/**
 * A few routines to allow writing debug/trace messages to the seperate debug
 * window.
 */
void SetMessageLog(CEdit* pedit)
{
	pLoggingWindow = pedit;
}

void WriteMessageLog(CString& msg)
{
	if (pLoggingWindow)
	{
		CString str = msg + CString(L"\r\n");
		int length = pLoggingWindow->GetWindowTextLength();
		pLoggingWindow->SetSel(length, length);
		pLoggingWindow->ReplaceSel(str);
	}
}

void WriteMessageLog(const char* pchars)
{
	CString s(pchars);
	WriteMessageLog(s);
}

void WriteMessageLog(const wchar_t* pwchars)
{
	CString s(pwchars);
	WriteMessageLog(s);
}

void WriteMessageLog(const std::string& str)
{
	WriteMessageLog(str.c_str());
}

void MessageExit(const wchar_t* msg)
{
	AfxMessageBox(msg, MB_ICONERROR | MB_APPLMODAL | MB_OK);
	ExitProcess(1);
}





bool EditUrl_Epguides(show* pshow)
{
	// Display an input dialog with an appropriate title & prompt
	CDInputBox dlg;
	dlg.m_title = L"Epguides";
	dlg.m_prompt = L"Enter show URL for epguides.com";
	dlg.m_input = pshow->epguides_url.c_str();
	if (dlg.DoModal() != IDOK)
		return false;

	std::string new_url = dlg.m_input_str;
	if (new_url.compare(pshow->epguides_url) == 0)
		return false;

	pshow->epguides_url = new_url;
	return true;
}

bool EditUrl_TVmaze(show* pshow)
{
	// Display an input dialog with an appropriate title & prompt
	CDInputBox dlg;
	dlg.m_title = L"TVMaze";
	dlg.m_prompt = L"Enter show URL for TVMaze.com";
	dlg.m_input = pshow->tvmaze_url.c_str();
	if (dlg.DoModal() != IDOK)
		return false;

	std::string new_url = dlg.m_input_str;
	if (new_url.compare(pshow->tvmaze_url) == 0)
		return false;

	pshow->tvmaze_url = new_url;
	return true;
}

bool EditUrl_IMDB(show* pshow)
{
	std::string new_url;

	// Display an input dialog with an appropriate title & prompt
	CDInputBox dlg;
	dlg.m_title = L"IMDB";
	dlg.m_prompt = L"Enter show URL for IMDB.com";
	dlg.m_input = pshow->imdb_url.c_str();
	if (dlg.DoModal() != IDOK)
		return false;

	new_url = dlg.m_input_str;
	if (new_url.compare(pshow->imdb_url) == 0)
		return false;

	pshow->imdb_url = new_url;
	return true;
}

bool EditUrl_TheTVDB(show* pshow)
{
	// Display an input dialog with an appropriate title & prompt
	CDInputBox dlg;
	dlg.m_title = L"TheTVDB";
	dlg.m_prompt = L"Enter show URL for TheTVDB.com";
	dlg.m_input = pshow->thetvdb_url.c_str();
	if (dlg.DoModal() != IDOK)
		return false;

	std::string new_url = dlg.m_input_str;
	if (new_url.compare(pshow->thetvdb_url) == 0)
		return false;

	pshow->thetvdb_url = new_url;
	return true;
}



void SetListHeaderFont(CListCtrl* plist)
{
	// Set all listcontrol column headers to the same bold font
	LOGFONT logfont;
	static CFont newfont;

	if (newfont.m_hObject == NULL)
	{
		plist->GetHeaderCtrl()->GetFont()->GetLogFont(&logfont);
		logfont.lfWeight = FW_BOLD;
		newfont.CreateFontIndirect(&logfont);
	}

	plist->GetHeaderCtrl()->SetFont(&newfont, 1);
}



/**
 * Return the index of the selected item in a CListCtrl
 * If 0 or >1 items are slected return -1
 */
int GetSelectedListItem(CListCtrl& pctrl)
{
	int index = -1;
	if (pctrl.GetSelectedCount() == 1)
	{
		POSITION pos = pctrl.GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			index = pctrl.GetNextSelectedItem(pos);
		}
	}
	return index;
}



/**
 * Copy CString str to the clipboard
 */
void CopyToClipboard(const CString& str)
{

	int epname_len = str.GetLength();
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, epname_len + 1);
	if (hMem)
	{
		char* pdest = (char*) GlobalLock(hMem);
		if (pdest)
		{
			CT2CA c_str(str,CP_UTF8);
			memcpy(pdest, c_str, epname_len);
			pdest[epname_len] = '\0';
			GlobalUnlock(hMem);
			OpenClipboard(0);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hMem);
			CloseClipboard();
		}
	}
}



/**
 * Copy C style string to clipboard
 */
void CopyToClipboard(const char *str)
{
	const CString _str(str);
	CopyToClipboard(_str);
}



/**
 * Copy std::string text to clipboard
 */
void CopyToClipboard(const std::string str)
{
	const CString _str(str.c_str());
	CopyToClipboard(_str);
}



/**
 * Stamp spaces over all occurances of a substring. Used to remove &nbsp; from HTML source.
 */
void ReplaceAllSubstrings(std::string& str, const char* sub)
{
	int len = strlen(sub);

	for (auto i = str.find(sub); i != std::string::npos; i = str.find(sub))
		str.replace(i, len, len, ' ');
}






#if (ENABLE_OBJECT_TRACKING==1) && defined(_DEBUG) && (ENABLE_CONSOLE_WINDOW==1)

void TRACK_DYNAMIC_OBJECTS(const wchar_t* str)
{
	WriteDebugConsole(str);
}

#else

#define		TRACK_DYNAMIC_OBJECTS(x)     do {} while (0)

#endif
