#pragma once

#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;


#include "CdownloadManager.h"
#include "CdataFile.hpp"
#include "utils.hpp"





// Which show list selector
typedef enum class SHOWLIST_tag : unsigned {
    SEARCH_ACTIVE,
    SEARCH_ARCHIVE,
    SEARCH_BOTH
} eSHOWLIST;






/**
 * This is an element in the guide/schedule list of all active episodes in the date range
 */
class sGuideEntry
{
public:
    std::string         show_title;
    std::string         episode_number;
    std::string         episode_title;
    gregorian::date     airdate;

    showflags           show_flags{ showflags::SH_FL_NONE };
    episodeflags        episode_flags{ episodeflags::EP_FL_NONE };
    DWORD               hash{ DWORD_MAX };

    // Used by std::sort, std::upper_bound and std::lower_bound
    inline bool operator<(const sGuideEntry& rhs) const
    {
        return airdate < rhs.airdate;
    }
};






/**
 * The actual database class with all the functionality.
 */
class model
{
public:
    model()
    {
        // A new empty model just needs a few dates preparing
        m_today      = gregorian::day_clock::local_day();
        EvalScheduleDateWindow();

        if (m_datafile.IsNewFile())
            SaveFile();
    };


    friend std::ostream& operator<< (std::ostream& wostream, const model& m);
    friend std::istream& operator>> (std::istream& wistream, model& m);


    // Read/Write data file
    bool        LoadFile();
    bool        SaveFile();
    void        ExploreDataFile();

    bool        EpisodeFlagsChange(const sPopupContext* pcontext);

    bool        GetFirstActiveShow(sShowListEntry* sle);
    bool        GetNextActiveShow(sShowListEntry* sle);
    bool        GetFirstArchiveShow(sShowListEntry* sle);
    bool        GetNextArchiveShow(sShowListEntry* sle);

    // To step through episodes within the date filter & return info
    void        SetDateInterval(int lower, int upper);
    void        ResetToday();
    bool        GetFirstFilteredEpisode(sScheduleListEntry* sle);
    bool        GetNextFilteredEpisode(sScheduleListEntry* sle);

    // Download from internet
    void        DownloadPing(DWORD slotnum);
    bool        DownloadAllShows();
    bool        DownloadNewShow(CString& url);
    bool        DownloadRefreshShow(show* pshow);
    bool        DownloadComplete();
    void        AbortDownload(void);

    void        BuildEpisodeList();
    void        DeleteShow(DWORD hash);
    void        AddShow(show& show);

    bool        ArchiveShow(DWORD hash);
    bool        UnarchiveShow(DWORD hash);

    unsigned    GetNumActiveShows(void) const { return m_active_shows.size(); }
    unsigned    GetNumArchiveShows(void) const { return m_archive_shows.size(); }
    show*       FindShow(DWORD searchhash, eSHOWLIST source);


    void SetMsgWindow(HWND hMsgWin)
    {
        m_hMsgWin = hMsgWin;
        dm.SetMsgWindow( m_hMsgWin );
    }

    inline void ShowMissedOnly(bool missed_only)
    {
        m_missed_edpisodes_only = missed_only;
    }

    inline bool DownloadInProgress() const
    {
        return dm.DownloadInProgress();
    }

    inline bool IsNewDataFile() const
    {
        return m_datafile.IsNewFile();
    }


private:
    void        EvalScheduleDateWindow();
    void        CheckDownloadComplete();


    std::vector<show>           m_active_shows;
    std::vector<show>           m_archive_shows;
    std::vector<sGuideEntry>    m_guide;

    // Iterators for the filtered range in the guide list
    std::vector<sGuideEntry>::iterator      guide_list_last;
    std::vector<sGuideEntry>::iterator      guide_list_next;

    CCriticalSection            m_critical;
    CdownloadManager	        dm;
    CdataFile                   m_datafile;

    // Where to post 'ping' and 'complete' msgs to.
    HWND                        m_hMsgWin { NULL };

    // Default # days interval. Updated in CepcheckDlg::OnInitDialog()
    int                         m_pre_days { 2 };
    int                         m_post_days{ 2 };

    boost::gregorian::date      m_start_date, m_end_date, m_today;
    BOOL                        m_missed_edpisodes_only { FALSE };
    bool                        m_abort_download { false };

    // Current indices used when app requests next show/episode
    unsigned int                m_current_show { 0 };

    unsigned int                m_ping_expected{ 0 };
    unsigned int                m_ping_received{ 0 };
};

