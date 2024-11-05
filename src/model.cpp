#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>

#include "boost/algorithm/string/trim.hpp"

#include "Shlwapi.h"
#include "strsafe.h"
#include "pathcch.h"

#include "common.hpp"

#include "Cepisode.h"
#include "Cshow.h"
#include "CepcheckDlg.h"
#include "threadFuncs.hpp"
#include "xmlParse.hpp"
#include "utils.hpp"

#include "model.hpp"







/**
 *  Useful function objects (functors).
 *
 */
struct fnoMatchShowHash
{
    fnoMatchShowHash() = delete;
    explicit fnoMatchShowHash(DWORD h) : hash(h) {}
    bool operator()(const show& s) const
    {
        return s.hash == hash;
    }
private:
    DWORD hash;
};




struct fnoMatchEpisodeNumber
{
    fnoMatchEpisodeNumber() = delete;
    explicit fnoMatchEpisodeNumber(const CString& epnum)
    {
        ep_num = CW2A(epnum, CP_UTF8);
    }

    bool operator()(const episode& test_ep) const
    {
        return ep_num == test_ep.ep_num;
    }
private:
    std::string ep_num;
};




/**
 *  Stream the whole model OUT to disk
 *
 */
std::ostream& operator<< (std::ostream& ostream, const model& model)
{
    size_t num_shows = model.m_active_shows.size() + model.m_archive_shows.size();

    ostream << num_shows << std::endl;

    for (const show& s : model.m_active_shows)
        ostream << s;

    for (const show& s : model.m_archive_shows)
        ostream << s;

    return ostream;
}




/**
 *  Stream the whole model IN from disk
 *
 */
std::istream& operator>> (std::istream& istream, model& model)
{
    std::string str;
    do
    {
        // Lines starting with '#' or blank lines at the top of the file are ignored
        std::getline(istream, str);
        boost::trim(str);
    } while (!istream.eof() && ((str.length()==0) || (str[0]=='#')));

    // Bad file format!
    if (istream.eof())
    {
        AfxMessageBox(L"ERROR! Bad data file format.", MB_ICONERROR | MB_OK | MB_APPLMODAL);
        ::PostQuitMessage(1);
        return istream;
    }

    // First line in the file is 'number of shows'
    unsigned numShows;
    std::stringstream(str) >> numShows;

    for (unsigned i = 0; i < numShows; i++)
    {
        show s;

        istream >> s;

        if (s.flags & showflags::SH_FL_ARCHIVED)
            model.m_archive_shows.push_back(s);
        else
            model.m_active_shows.push_back(s);
    }

    return istream;
}




/**
 * Search the active and/or the archived lists for a show with the given hash
 *
 */
show* model::FindShow(DWORD search_hash, eSHOWLIST source)
{
    show* retval = nullptr;
    auto funcObject = fnoMatchShowHash(search_hash);

    if (search_hash == DWORD_MAX)
    {
        WriteMessageLog(L"model::FindShow() bad hash");
    }
    else
    {
        if ((source == eSHOWLIST::SEARCH_ACTIVE) || (source == eSHOWLIST::SEARCH_BOTH))
        {
            auto it = std::find_if(m_active_shows.begin(), m_active_shows.end(), funcObject);
            if (it != m_active_shows.end())
                retval = &*it;
        }

        if ((retval == nullptr) && ((source == eSHOWLIST::SEARCH_ARCHIVE) || (source == eSHOWLIST::SEARCH_BOTH)))
        {
            auto it = std::find_if(m_archive_shows.begin(), m_archive_shows.end(), funcObject);
            if (it != m_archive_shows.end())
                retval = &*it;
        }
    }
    return retval;
}



/**
 * Shows can only be deleted from the archived tab
 * 
 */
void model::DeleteShow(DWORD hash)
{
    m_archive_shows.erase( std::remove_if(m_archive_shows.begin(), m_archive_shows.end(), fnoMatchShowHash(hash)),
        m_archive_shows.end());

    BuildEpisodeList();
}




/**
 * Add a new show to the model. All shos start off in the active list.
 *
 */
void model::AddShow(show& show)
{
    m_active_shows.push_back(show);
}




/**
 * Clear out the data model and re-load it from disk
 * 
 */
bool model::LoadFile()
{
    // Clear internal lists
    m_guide.clear();
    m_active_shows.clear();
    m_archive_shows.clear();

    // Just in case, set a bunch of members vars to defaults!
    guide_list_last   = m_guide.begin();
    guide_list_next   = m_guide.begin();
    m_current_show    = 0;

    // The file's definately there thanks to the constructor
    std::ifstream ifs(m_datafile.Filename(), std::ofstream::in);
    if (!ifs.is_open())
    {
        AfxMessageBox(L"Can't open data file", MB_ICONERROR | MB_OK);
        return false;
    }

    ifs >> *this;
    ifs.close();

    BuildEpisodeList();

    return true;
}




/**
 * Save the data model to disk
 * 
 */
bool model::SaveFile()
{
    // Sort on show title using show::operator<()
    std::sort(m_active_shows.begin(), m_active_shows.end());

    std::ofstream ofs(m_datafile.Filename(), std::ofstream::out);
    if (ofs.is_open()) {
        ofs << *this;
        ofs.close();
        return true;
    }

    return false;
}




/**
 * After downloading or deleting anything, rescan all show episodes into the guide vector.
 * 
 * Apply wide filters here: m_active_shows_only, and m_missed_episodes_only
 * 
 */
void model::BuildEpisodeList()
{
    m_guide.clear();

    for (const auto& show : m_active_shows)
    {
        sGuideEntry ge;
        ge.show_title = show.title;
        ge.show_flags = show.flags;
        ge.hash       = show.hash;

        for (const auto& episode : show.episodes)
        {
            // Don't bother adding it to the guide if it has no air date
            if (!episode.ep_date.is_infinity())
            {
                ge.episode_title  = episode.ep_title;
                ge.episode_number = episode.ep_num;
                ge.airdate        = episode.ep_date;
                ge.episode_flags  = episode.ep_flags;

                m_guide.push_back(ge);
            }
        }
    }

    std::sort(m_guide.begin(), m_guide.end());
}




/**
 * Populate the sShowListEntry structure with first Show 'alpabetically' <==???
 * 
 */
bool model::GetFirstActiveShow(sShowListEntry* sle)
{
    m_current_show    = 0;

    if (m_active_shows.size() == 0)
        return false;

    CopyOutShowInfo(sle, &m_active_shows[ m_current_show ]);

    return true;
}




/**
 * Populate the sShowListEntry structure with next Show.
 * AFTER first calling to GetFirstActiveShow().
 * 
 */
bool model::GetNextActiveShow(sShowListEntry* sle)
{
    // No shows?
    if (m_active_shows.size() == 0)
    {
        WriteMessageLog(L"GetNextActiveShow(): size=0");
        return false;
    }

    // At the end already?
    if (m_current_show == (m_active_shows.size() - 1))
    {
        DebugWriteMessageLog(L"GetNextActiveShow(): All done");
        return false;
    }

    m_current_show++;

    CopyOutShowInfo(sle, &m_active_shows[ m_current_show ]);

    return true;
}




bool model::GetFirstArchiveShow(sShowListEntry* sle)
{
    m_current_show = 0;

    if (m_archive_shows.size() == 0)
        return false;

    CopyOutShowInfo(sle, &m_archive_shows[ m_current_show ]);

    return true;
}




bool model::GetNextArchiveShow(sShowListEntry* sle)
{
    // No shows?
    if (m_archive_shows.size() == 0)
    {
        WriteMessageLog(L"GetNextArchiveShow(): size=0");
        return false;
    }

    // At the end already?
    if (m_current_show == (m_archive_shows.size() - 1))
    {
        DebugWriteMessageLog(L"GetNextArchivShow(): All done");
        return false;
    }

    m_current_show++;

    CopyOutShowInfo(sle, &m_archive_shows[ m_current_show ]);

    return true;
}




STATIC void
CopyGuideEntryToScheduleListEntry(const sGuideEntry* ge, sScheduleListEntry* sle)
{
    sle->show_title     = ge->show_title;
    sle->hash           = ge->hash;
    sle->show_flags     = ge->show_flags;
    sle->episode_title  = ge->episode_title;
    sle->episode_flags  = ge->episode_flags;
    sle->episode_number = ge->episode_number;
    sle->airdate        = ge->airdate;
    sle->airdate_string = gregorian::to_simple_string(ge->airdate);
}




/**
 * Populate the sEpisodeListEntry structure with FIRST episode within the filter range
 * 
 */
bool model::GetFirstFilteredEpisode(sScheduleListEntry* sle)
{
    sGuideEntry ge;

    if (m_missed_edpisodes_only)
    {
        ge.airdate - gregorian::min_date_time;
        guide_list_next = std::lower_bound(m_guide.begin(), m_guide.end(), ge);

        // Include EVERY episode, dates ignored, just the 'NOT_GOT' flag checked
        guide_list_next = m_guide.begin();
        guide_list_last = m_guide.end();

        while (!(guide_list_next->episode_flags & episodeflags::EP_FL_NOT_GOT))
        {
            guide_list_next++;
            if (guide_list_next == guide_list_last)
                return false;
        }
    }
    else
    {
        // Get 1st element not 'less than' m_start_date
        ge.airdate = m_start_date;
        guide_list_next = std::lower_bound(m_guide.begin(), m_guide.end(), ge);

        // No episodes in the date range?
        if (guide_list_next == m_guide.end())
            return false;
        // Future episode was listed but with no air date yet
        if (guide_list_next->airdate.is_not_a_date())
            return false;
        // Ran off the end of the list?
        if (guide_list_next->airdate.is_infinity())
            return false;

        // Get 1st element 'greater than' m_end_date    
        ge.airdate = m_end_date;
        guide_list_last = std::upper_bound(guide_list_next, m_guide.end(), ge);
    }


    CopyGuideEntryToScheduleListEntry(&*guide_list_next, sle);

    ++guide_list_next;

    return true;
}




/**
 * Populate the sGuideListEntry structure with NEXT episode within the filter range
 * 
 */
bool model::GetNextFilteredEpisode(sScheduleListEntry* sle)
{
    if ((guide_list_next == guide_list_last) || (guide_list_next == m_guide.end()) )
        return false;

    // Future episode was listed but with no air date yet
    if (guide_list_next->airdate.is_not_a_date())
        return false;

    // Ran off the end of the list
    if (guide_list_next->airdate.is_infinity())
        return false;

    // Not by date, just on the NOT_GOT flag
    if (m_missed_edpisodes_only)
    {
        while (!(guide_list_next->episode_flags & episodeflags::EP_FL_NOT_GOT))
        {
            guide_list_next++;
            if (guide_list_next == guide_list_last)
                return false;
        }
    }


    CopyGuideEntryToScheduleListEntry(&*guide_list_next, sle);

    ++guide_list_next;

    return true;
}




/**
 * Download a show's information from the URL and create a new database entry.
 * Currently only called when adding a new show.
 * 
 */
bool model::DownloadNewShow(CString& url)
{
    bool retval = true;
    CString error1(L"Download already in progress!");

    // Already downloading?
    if (dm.DownloadInProgress()) {
        WriteMessageLog(error1);
        AfxMessageBox(error1, MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
        return false;
    }

    // Clear all other shows 'WAITING' flag so they aren't downloaded.
    std::for_each(m_active_shows.begin(), m_active_shows.end(), [](show& show) {show.state &= ~showstate::SH_ST_WAITING; });

    // Prepare a new Show object
    show	new_show;
    new_show.epguides_url = CW2A(url, CP_UTF8);
    new_show.hash         = SimpleHash(new_show.epguides_url);
    new_show.title        = "--NEW SHOW--";
    new_show.state        = showstate::SH_ST_NEW_SHOW | showstate::SH_ST_WAITING;
    new_show.flags        = showflags::SH_FL_NONE;

    // Add it to the shows vector
    AddShow(new_show);

    // Setup the ping counters
    m_ping_expected = 1;
    m_ping_received = 0;

    dm.DownloadShow(new_show.epguides_url);

    // Need to filter for new_show in ping handler

    return retval;
}




/**
 * Download from the internet. Update all shows / episodes.
 * 
 * Return:  true    Download started
 *          false   Error, or nothing to download
 * 
 */
bool model::DownloadAllShows()
{
    // No shows to download?
    if (m_active_shows.size() == 0)
        return false;

    // Already downloading?
    if (dm.DownloadInProgress()) {
        AfxMessageBox(L"Download already in progress!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
        return false;
    }

    // Reset counters
    m_ping_expected = m_active_shows.size();
    m_ping_received = 0;

    // Flag all shows 'WAITING' for download and queue for download
    for (auto& ashow : m_active_shows)
    {
        ashow.state |= showstate::SH_ST_WAITING;
        dm.DownloadShow(ashow.epguides_url);
    }

    WriteMessageLog(L"All shows queued for download");
    return true;
}




/**
 * Download and refresh a single existing show.
 * 
 */
bool model::DownloadRefreshShow(show* pshow)
{
    // Already downloading?
    if (dm.DownloadInProgress()) {
        AfxMessageBox(L"Download already in progress!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
        return false;
    }

    // Clear all other shows 'WAITING' flag so they aren't downloaded.
    std::for_each(m_active_shows.begin(), m_active_shows.end(), [](show& show) {show.state &= ~showstate::SH_ST_WAITING; });
    pshow->state |= showstate::SH_ST_WAITING;

    // Reset counters
    m_ping_expected = 1;
    m_ping_received = 0;

    dm.DownloadShow(pshow->epguides_url);
    WriteMessageLog(L"Download requested");

    return true;
}




/**
* If all shows are downloaded *OR* the download has been sucessfully
* aborted, send WM_DOWNLOAD_COMPLETE to the main dialog window.
*
*/
void model::CheckDownloadComplete()
{
    if (
        ((m_abort_download == true) && (dm.DownloadInProgress() == false)) ||
        (m_ping_expected == m_ping_received)
       )
    {
        ::PostMessage(m_hMsgWin, WM_DOWNLOAD_COMPLETE, 0, 0);
    }
}




 /**
 * One show has completed it's download. Update the show information in the model.
 * Existing episode flags must be copied over as that's not in the reveived data
 * 
 */
void model::DownloadPing(DWORD slotnum)
{
    CslotData& slot = dm.GetSlot(slotnum);
    const show& resultShow = slot.m_show;

    if (slot.IsFree())
    {
        const wchar_t* msg = L"ERROR! Empty slot signalled.";
        WriteMessageLog(msg);
        WriteDebugConsole(msg);
        MessageBeep(MB_ICONASTERISK);
        CheckDownloadComplete();
        return;
    }

    m_ping_received++;

    // If this is for a new show, a dummy database entry was added so this pointer will always be valid
    show* originalShow = FindShow(resultShow.hash, eSHOWLIST::SEARCH_ACTIVE);

    if (slot.ThreadStatus() != E_THREAD_OK)
    {
        WriteMessageLog("DownloadPing() Download error, aborting : " + slot.ErrorString());
        originalShow->state |= (showstate::SH_ST_UPDATE_FAILED | resultShow.state);
        AbortDownload();
    }
    else
    {
        // Save all current episode flags in a map with the episode number as the key
        std::map<std::string, episodeflags> ep_flags_map;
        for (const auto& ep : originalShow->episodes)
            ep_flags_map[ep.ep_num] = ep.ep_flags;

        // Put the show's episodes in the database - lock the 'database' while we're twiddling.
        CSingleLock dbLock(&m_critical);
        dbLock.Lock();

        // Copy over URLs and stuff from the downloaded show into the original show. Preserve the original show flags.
        originalShow->episodes    = resultShow.episodes;
        originalShow->title       = resultShow.title;
        originalShow->imdb_url    = resultShow.imdb_url;
        // This URL is 'guessed' from the show title - don't do it twice or you'll overwrite any manual corrections.
        if (originalShow->tvmaze_url.empty())
            originalShow->tvmaze_url = resultShow.tvmaze_url;
        originalShow->thetvdb_url = resultShow.thetvdb_url;
        originalShow->state      |= showstate::SH_ST_UPDATED;
        EvalShowLastNextDates(originalShow);

        VERIFY(dbLock.Unlock());

        // Now restore saved episode flags (Original_show is captured in the lambda only for the log message)
        for (auto& ep : originalShow->episodes)
        {
            auto iter = ep_flags_map.find(ep.ep_num);
            if (iter != ep_flags_map.end()) {
                ep.ep_flags = iter->second;
            }
            else {
                // If we didn't have that episode's flags, it's either a completely new show or a new episode. for
                // an existing show. (Only notify about new episodes for existing shows, otherwise it gets too noisy.)
                if (!(originalShow->state & showstate::SH_ST_NEW_SHOW)) {
                    std::ostringstream str;
                    str << "New episode " << (originalShow->title) << " " << ep.ep_num;
                    WriteMessageLog(str.str());
                }
            }
        }

        // Notify the appropriate worker thread we're done
        dm.ReleaseSlot( slotnum );
    }

    // Time to send WM_DOWNLOAD_COMPLETE ?
    CheckDownloadComplete();
}




/**
 * >ALL< shows have completed downloading.
 *
 */
bool model::DownloadComplete()
{
    WriteMessageLog(L"Download Complete()");

    bool bHadDownloadErrors = false;

    // Purge episodes for SH_ST_UPDATE_FAILED shows. Delete New Shows that failed too.
    // If we delete a show we must start again from the beginning as iterators are invalidated.

    bool finished;
    do
    {
        finished = true;
        for (unsigned i = 0; i < m_active_shows.size(); ++i)
        {
            show& sh = m_active_shows[i];
            if (sh.state & showstate::SH_ST_UPDATE_FAILED)
            {
                bHadDownloadErrors = true;
                std::string msg = "DownloadComplete(): Show download failed " + sh.epguides_url;
                WriteMessageLog(msg);

                if (sh.state & showstate::SH_ST_NEW_SHOW)
                {
                    // Adding a new show failed. Delete it & restart the scan.
                    m_active_shows.erase(m_active_shows.begin() + i);
                    finished = false;
                    break;
                }
            }
            else if (sh.state & showstate::SH_ST_UPDATED)
            {
                // No error, good download. Reset show state to something sensible (ie clear other flags)
                sh.state = showstate::SH_ST_UPDATED;
            }
        }
    } while (!finished);

    // Reset flag
    m_abort_download = false;

    BuildEpisodeList();

    return bHadDownloadErrors;
}








void model::SetToday()
{
    m_today = gregorian::day_clock::local_day();
    EvalScheduleDateWindow();
}

void model::EvalScheduleDateWindow()
{
    m_start_date = m_today - gregorian::date_duration(m_pre_days);
    m_end_date   = m_today + gregorian::date_duration(m_post_days);
}

void model::SetDateInterval(int lower, int upper)
{
    m_pre_days  = lower;
    m_post_days = upper;
    EvalScheduleDateWindow();
}




bool model::ArchiveShow(DWORD hash)
{
    auto funcObject = fnoMatchShowHash(hash);
    auto iter = std::find_if(m_active_shows.begin(), m_active_shows.end(), funcObject);

    // This shouldn't happen!
    if (iter == m_active_shows.end())
        return false;

    (*iter).flags |= showflags::SH_FL_ARCHIVED;

    m_archive_shows.push_back(*iter);
    m_active_shows.erase(iter);

    BuildEpisodeList();

    return true;
}




bool model::UnarchiveShow(DWORD hash)
{
    auto show = std::find_if(m_archive_shows.begin(), m_archive_shows.end(), fnoMatchShowHash(hash));

    // This shouldn't happen!
    if (show == m_archive_shows.end())
        return false;

    (*show).flags &= ~showflags::SH_FL_ARCHIVED;

    m_active_shows.push_back(*show);
    m_archive_shows.erase(show);

    BuildEpisodeList();

    return true;
}




bool model::EpisodeFlagsChange(const sPopupContext* pcontext)
{
    DWORD hash = pcontext->show_hash;
    bool retval = true;

    show* pshow = FindShow(hash, eSHOWLIST::SEARCH_ACTIVE);
    if (pshow != nullptr)
    {
        fnoMatchEpisodeNumber  fnoMatchEpNum(pcontext->ep_num);
        auto ep_ref = std::find_if(pshow->episodes.begin(), pshow->episodes.end(), fnoMatchEpNum);

        if (ep_ref == pshow->episodes.end())
        {
            WriteMessageLog(L"EpisodeFlagsChange(): Episode not found");
            retval = false;
        }
        else
        {
            ep_ref->ep_flags = pcontext->ep_flags;
            BuildEpisodeList();
        }
    }

    return retval;
}




/**
 * DownloadAllShows() can be aborted.
 *
 */
void model::AbortDownload(void)
{
    m_abort_download = true;
    dm.AbortDownload();
}




/**
 * Open an explorer window in the datafile location
 *
 */
void model::ExploreDataFile(void)
{
    wchar_t filepath[ MAX_PATH ];

    StringCchCopy(filepath, MAX_PATH, m_datafile.Filename());
    PathRemoveFileSpec(filepath);

    ShellExecute(NULL, L"open", filepath, NULL, NULL, SW_SHOWDEFAULT);
}

