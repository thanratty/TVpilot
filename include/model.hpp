#pragma once

#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;


#include "CdownloadManager.hpp"
#include "CdataFile.hpp"
#include "utils.hpp"





// Which show list selector
typedef enum class SHOWLIST_tag : unsigned {
    ACTIVE,
    ARCHIVE,
    BOTH
} eSHOWLIST;



// When iterating a vector, which object to return
typedef enum class GET_ACTION_tag : unsigned {
    GET_FIRST,
    GET_NEXT
} eGETACTION;





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
            EvalScheduleDateRange();

            if (m_datafile.IsNewFile())
                SaveFile();
        };


        friend std::ostream& operator<< (std::ostream& wostream, const model& m);
        friend std::istream& operator>> (std::istream& wistream, model& m);


        // Read/Write data file
        bool        LoadFile();
        bool        SaveFile();
        void        OpenDataFileFolder();

        bool        EpisodeFlagsChange(const sPopupContext* pcontext);
 
        bool        GetShow(eSHOWLIST list, eGETACTION action, sShowListEntry* sle) const;

        // To step through episodes within the date filter & return info
        void        SetDateInterval(int lower, int upper);
        void        SetTodaysDate();
        bool        GetFilteredEpisode(eGETACTION locus, sScheduleListEntry* sle);


        // Download from internet
        void        OnDownloadPing(DWORD slotnum);
        void        OnSlotReleased(DWORD slotnum);
        bool        DownloadAllShows();
        bool        DownloadSingleShow(DWORD hash);
        bool        DownloadComplete();
        void        AbortDownload(void);

        void        BuildEpisodeList();
        void        DeleteShow(DWORD hash);
        DWORD       AddNewShow(const CString& url);

        bool        ArchiveShow(DWORD hash);
        bool        UnarchiveShow(DWORD hash);

        show* FindShow(DWORD searchhash, eSHOWLIST source);
        show* FindShow(const CString& url, eSHOWLIST source);

        unsigned NumShows(eSHOWLIST list) const
        { 
            if (list == eSHOWLIST::ACTIVE)
                return m_active_shows.size();
            else if (list == eSHOWLIST::ARCHIVE)
                return m_archive_shows.size();
            else if (list == eSHOWLIST::BOTH)
                return m_active_shows.size() + m_archive_shows.size();
            else
            {
                CString str(L"NumShows() : Bad show list");
                LogMsgWindow(str);
                AfxMessageBox(str);
                return 0;
            }
        }

        void SetMsgWindow(HWND hMsgWin)
        {
            m_hMsgWin = hMsgWin;
            dm.SetMsgWindow( m_hMsgWin );
        }

inline  void ShowMissedOnly(bool missed_only)
        {
            m_missed_edpisodes_only = missed_only;
        }

inline  bool DownloadInProgress() const
        {
            return dm.DownloadInProgress();
        }

inline  bool IsNewDataFile() const
        {
            return m_datafile.IsNewFile();
        }


private:
        void        EvalScheduleDateRange();
        void        CheckDownloadComplete();


        std::vector<show>           m_active_shows;
        std::vector<show>           m_archive_shows;
        std::vector<sGuideEntry>    m_guide;

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

        unsigned int                m_ping_expected{ 0 };
        unsigned int                m_ping_received{ 0 };
};

