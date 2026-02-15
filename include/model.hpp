#pragma once

#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;


#include "CdownloadManager.hpp"
#include "CdataFile.hpp"
#include "utils.hpp"





// Which show list selector
typedef enum class ShowList_tag : UINT32 {
    ACTIVE,
    ARCHIVE,
    BOTH
} eShowList;



// When iterating a vector, which object to return
typedef enum class GetActiontag : UINT32 {
    GET_FIRST,
    GET_NEXT
} eGetAction;





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
 
        bool        GetShow(eShowList list, eGetAction action, sShowListEntry* sle) const;

        // To step through episodes within the date filter & return info
        void        SetDateInterval(int lower, int upper);
        void        SetTodaysDate();
        bool        GetFilteredEpisode(eGetAction locus, sScheduleListEntry* sle);
        void        BuildEpisodeList();

        void        AddNewShow(const show& showtoadd);
        bool        UpdateShow(const show& showtoupdate);


        void        DeleteShow(DWORD hash);
        bool        ArchiveShow(DWORD hash);
        bool        UnarchiveShow(DWORD hash);

        show* FindShow(DWORD searchhash, eShowList source);
        show* FindShow(const CString& url, eShowList source);

        unsigned NumShows(eShowList list) const
        { 
            if (list == eShowList::ACTIVE)
                return m_active_shows.size();
            else if (list == eShowList::ARCHIVE)
                return m_archive_shows.size();
            else if (list == eShowList::BOTH)
                return m_active_shows.size() + m_archive_shows.size();
            else
            {
                // Bad list?
                return 0;
            }
        }


inline  void ShowMissedOnly(bool missed_only)
        {
            m_missed_edpisodes_only = missed_only;
        }

inline  bool IsNewDataFile() const
        {
            return m_datafile.IsNewFile();
        }

inline unsigned NumActiveShows() const
        {
            return m_active_shows.size();
        }




private:
        void        EvalScheduleDateRange();


        std::vector<show>           m_active_shows;
        std::vector<show>           m_archive_shows;
        std::vector<sGuideEntry>    m_guide;

        CCriticalSection            m_critical;
        CdataFile                   m_datafile;

        // Where to post 'ping' and 'complete' msgs to.
        HWND                        m_hMsgWin { NULL };

        // Default # days interval. Updated in CepcheckDlg::OnInitDialog()
        int                         m_pre_days { 2 };
        int                         m_post_days{ 2 };

        boost::gregorian::date      m_start_date, m_end_date, m_today;
        BOOL                        m_missed_edpisodes_only { FALSE };
        bool                        m_abort_download { false };

};

