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
using namespace boost;

#include "common.hpp"

#include "Cshow.hpp"
#include "CDInputBox.hpp"

#include "utils.hpp"







/**
 * Convert "SS-EE" formatted episode string to a number for temporal ordering
 */
STATIC unsigned EpisodeToNumber(const CString& epstring)
{
	int season{ 0 }, epnum{ 0 };

	int hpos = epstring.Find('-');
	if (hpos != -1)
	{
		season = _wtoi(epstring.Left(hpos));
		epnum  = _wtoi(epstring.Mid(hpos + 1));
	}

	return season * 1000 + epnum;
}



/**
 * Compare/Order two episode-number strings SS-EE
 */
int EpisodeCompareFunc(const CString& episode1, const CString& episode2, bool ascending)
{
	int result;

	unsigned ep1 = EpisodeToNumber( episode1 );
	unsigned ep2 = EpisodeToNumber( episode2 );

	if (ep1 < ep2)
		result = -1;
	else if (ep1 > ep2)
		result = 1;
	else
		result = 0;

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
	CString s_title = (LPWSTR) CA2W(sle->show_title.c_str(), CP_UTF8);
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

	CString s_show_title = (LPWSTR) CA2W(gle->show_title.c_str(), CP_UTF8);
	gle->ui_show_title = s_show_title;

	CString s_airdate_string(gle->airdate_string.c_str());
	gle->ui_airdate_string = aDOW[(gle->airdate.day_of_week())] + s_airdate_string.Right(2) + s_airdate_string.Mid(4, 4);

	CString s_episode_number(gle->episode_number.c_str());
	gle->ui_episode_number = s_episode_number;

	CString s_episode_title = (LPCWSTR) CA2W(gle->episode_title.c_str(), CP_UTF8);
	gle->ui_episode_title = s_episode_title;

	std::string str = std::to_string(gle->airdate.julian_day());
	gle->ui_airdate_sort = CString(str.c_str());
}




/**
 * For the given show ptr, scan it's epsidos list and determine last & next airdates
 */
void EvalShowLastNextDates( show& aShow )
{
	// Prepare default dates
	gregorian::date today = gregorian::day_clock::local_day();
	gregorian::date _last = gregorian::date(gregorian::min_date_time);
	gregorian::date _next = gregorian::date(gregorian::max_date_time);

	for (const episode& ep : aShow.episodes)
	{
		// Track most recent before today
		if ((ep.ep_date > _last) && (ep.ep_date < today))
			_last = ep.ep_date;

		// Track next episode from today
		if ((ep.ep_date < _next) && (ep.ep_date >= today))
			_next = ep.ep_date;
	}

	aShow.last_airdate = _last;
	aShow.next_airdate = _next;
	aShow.last_airdate_string = (_last == gregorian::date(gregorian::min_date_time)) ? "" : gregorian::to_simple_string(_last);
	aShow.next_airdate_string = (_next == gregorian::date(gregorian::max_date_time)) ? "" : gregorian::to_simple_string(_next);
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




bool EditUrl(const CString&		title,
			 std::string&		url )
{
	// Display an input dialog with an appropriate title & prompt
	CDInputBox dlg;
	dlg.m_title  = title;
	dlg.m_prompt = CString(L"Enter URL for ") + title + L".com";
	dlg.m_input  = url.c_str();
	if (dlg.DoModal() != IDOK)
		return false;

	std::string new_url = dlg.m_input_str;
	if (new_url.compare(url) == 0)
		return false;

	url = new_url;
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
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, epname_len + 2);
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

