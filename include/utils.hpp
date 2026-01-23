#pragma once

#include <string>
#include <vector>
#include <queue>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;

#include "common.hpp"

#include "Cshow.hpp"


class CslotData;


enum class eAppevent
{
	AE_APP_STARTED,
	AE_TAB_CHANGED,
	AE_SHOW_ADDED,
	AE_SHOW_REFRESHED,
	AE_SHOW_DELETED,
	AE_DOWNLOAD_STARTED,
	AE_DOWNLOAD_OK,
	AE_DOWNLOAD_FAILED,
	AE_DOWNLOAD_ABORTED,
	AE_FILE_LOADED,
	AE_FILE_SAVED,
	AE_FILE_CREATED,
	AE_URL_EDITED,
	AE_ARCHIVE_CHANGED,
	AE_EPISODES_OPENED,
	AE_EPISODES_CLOSED,
	AE_EP_FLAGS_CHANGED
};




/**
 * Information passed in from child dialogs to display the context menu
 * 
 */
typedef struct PopupContext_tag
{
	int				dialog_id{ 0 };			// Dialog ID
	CListCtrl*		pList{ nullptr };		// Dialog objecy pointer
	int				list_index{ 0 };		// Item # in the list control
	DWORD			show_hash{ DWORD_MAX };
	CPoint			click_point;
	episodeflags	ep_flags{ episodeflags::EP_FL_NONE };			// If present - episode flags. Schedule list only?
	CString			ep_num;
} sPopupContext;











/**
 * This is the structure passed for GetFirstActiveShow() / GetNextActiveShow() queries to the model
 * 
 */
typedef struct ShowListEntry_tag
{
	std::string     show_title;
	std::string		epguides_url;
	DWORD           hash{ DWORD_MAX };
	size_t          num_episodes{ 0 };
	gregorian::date	last_airdate;
	gregorian::date	next_airdate;
	std::string     last_airdate_string;
	std::string     next_airdate_string;
	std::string     next_episode_number;
	showstate       show_state{ showstate::SH_ST_NONE };
	showflags		show_flags{ showflags::SH_FL_NONE };
	//
	CString         ui_title;
	CString			ui_epguides_url;
	CString         ui_last_airdate_string;
	CString         ui_next_airdate_string;
	CString         ui_next_episode_number;
	CString         ui_last_airdate_sort;
	CString         ui_next_airdate_sort;
} sShowListEntry;




/**
 * This is the structure passed in for GetFilteredEpisode() / GetNextFilteredEpisode() from the UI episode guide list
 * 
 */
typedef struct sScheduleListEntry_tag
{
	// Top half of the struct is filled in from the model daya
	//
	std::string         show_title;
	showflags			show_flags{ showflags::SH_FL_NONE };
	std::string         airdate_string;
	std::string         episode_number;
	std::string         episode_title;
	DWORD               hash{ DWORD_MAX };
	gregorian::date     airdate;
	episodeflags		episode_flags{ episodeflags::EP_FL_NONE };
	//
	// The CStrings are filled in by the UI
	//
	CString             ui_show_title;
	CString             ui_airdate_string;
	CString             ui_episode_number;
	CString             ui_episode_title;
	CString             ui_airdate_sort;
} sScheduleListEntry;














/**
 * Hash a URL into a unique numeric identifier
 */
DWORD SimpleHash(const std::string& str);
DWORD SimpleHash(const CString& str);


void CopyOutShowInfo(sShowListEntry* sle, const show* pshow);




/**
 * Sorting functions for list control columns
 */
int AlphaCompareFunc(const CString& item1, const CString& item2, bool ascending);
int NumberCompareFunc(const CString& item1, const CString& item2, bool ascending);
int EpisodeCompareFunc(const CString& episode1, const CString& episode2, bool ascending);




/**
 * Do conversion of std::string to CStrings & boost::dates to strings for display in a list box
 */
void ShowListStringsToLocal(sShowListEntry* sle);
void ScheduleListStringsToLocal(sScheduleListEntry* gle);



/**
 * Scan a show's episode list and work out the last/next air dates & save the in the show data
 */
void EvalShowLastNextDates(show* pshow);




#ifdef _DEBUG
#define		DebugWriteMessageLog(x)		WriteMessageLog(x)
#else
#define		DebugWriteMessageLog(x)		do {} while(0)
#endif


/**
 * Set and write to the debug dialog box
 */
void SetMessageLog(CEdit* pedit);
void WriteMessageLog(CString& msg);
void WriteMessageLog(const char* pchars);
void WriteMessageLog(const wchar_t* pwchars);
void WriteMessageLog(const std::string& str);


/**
 * Fatal error - show a messagebox & exit the program.
 */
void MessageExit(const wchar_t* msg);


/**
 * Website URLs might need different validation.
 */
bool EditUrl_Epguides(show* pshow);
bool EditUrl_TVmaze(show* pshow);
bool EditUrl_IMDB(show* pshow);
bool EditUrl_TheTVDB(show* pshow);

void SetListHeaderFont(CListCtrl* plist);
int  GetSelectedListItem(CListCtrl& pctrl);




/**
 * Copy a text string to the clipboard
 */
void CopyToClipboard(const CString& str);
void CopyToClipboard(const char* str);
void CopyToClipboard(const std::string str);


/**
 * Deletes all occurances of a substring 
 */
void ReplaceAllSubstrings(std::string& str, const char* sub);


bool CheckWaitResult(unsigned numevents, DWORD result);




#if (ENABLE_OBJECT_TRACKING==1) && defined(_DEBUG) && (ENABLE_CONSOLE_WINDOW==1)
void LOG_THREAD_OBJECT(const wchar_t* str);
#else
#define		LOG_THREAD_OBJECT(x)     do {} while (0)
#endif

