#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <strsafe.h>

#include "boost/algorithm/string/trim.hpp"

#include "common.hpp"

#include "Cepisode.hpp"
#include "Cshow.hpp"
#include "utils.hpp"
#include "logging.hpp"

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
std::ostream& operator<<(std::ostream& ostream, const model& model)
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
std::istream& operator>>(std::istream& istream, model& model)
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
show* model::FindShow(DWORD search_hash, eShowList source) 
{
    show* retval = nullptr;
    auto funcObject = fnoMatchShowHash(search_hash);

    if (search_hash == DWORD_MAX)
    {
        LogMsgWin(L"model::FindShow() bad hash");
    }
    else
    {
        if ((source == eShowList::ACTIVE) || (source == eShowList::BOTH))
        {
            auto it = std::find_if(m_active_shows.begin(), m_active_shows.end(), funcObject);
            if (it != m_active_shows.end())
                retval = &*it;
        }

        if ((retval == nullptr) && ((source == eShowList::ARCHIVE) || (source == eShowList::BOTH)))
        {
            auto it = std::find_if(m_archive_shows.begin(), m_archive_shows.end(), funcObject);
            if (it != m_archive_shows.end())
                retval = &*it;
        }
    }
    return retval;
}




show* model::FindShow(const CString& url, eShowList source) 
{
    return FindShow(SimpleHash(url), source);
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
 * Clear out the data model and re-load it from disk
 * 
 */
bool model::LoadFile()
{
    // Clear internal lists
    m_guide.clear();
    m_active_shows.clear();
    m_archive_shows.clear();

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
    // Sort on show title
    std::sort(m_active_shows.begin(), m_active_shows.end(), [](const show& a, const show& b)
        {
            return a.title < b.title;
        });

    std::ofstream ofs(m_datafile.Filename(), std::ofstream::out);
    if (ofs.is_open()) {
        ofs << *this;
        ofs.close();
        return true;
    }

    return false;
}




/**
 * After downloading or deleting anything, add **ALL** active show episodes into the guide vector.
 * Leave the vector sorted on airdate.
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

    // Sorts on airdate
    std::sort(m_guide.begin(), m_guide.end());
}




bool model::GetShow(eShowList list, eGetAction action, sShowListEntry* sle) const
{
    static unsigned index{ 0 };
    const std::vector<show>* pVec;

    // Makes no sense to iterate through both lists at once
    if (list == eShowList::BOTH)
        return false;

    pVec = (list == eShowList::ACTIVE) ? &m_active_shows : &m_archive_shows;

    if (pVec->size() == 0)
        return false;

    if (action == eGetAction::GET_FIRST)
        index = 0;

    // At the end already?
    if (index > NumShows( list ) - 1)
        return false;

    CopyOutShowInfo(sle, &pVec->at(index));
    ++index;

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
 * Populate the sGuideListEntry structure with FIRST or NEXT episode within the filter range
 *
 */
bool model::GetFilteredEpisode(eGetAction action, sScheduleListEntry* sle)
{
    sGuideEntry ge;
    static unsigned index{ 0 },
                    index_end{ 0 };

    if (m_guide.size() == 0)
        return false;

    if (action == eGetAction::GET_FIRST)
    {
        // If we're using date filters, set them up here. sGuideEntry has an operator<() & is used as a functor.
        if (!m_missed_edpisodes_only)
        {
            ge.airdate = m_start_date;
            auto start_iter = std::lower_bound(m_guide.begin(), m_guide.end(), ge);
            index = start_iter - m_guide.begin();

            ge.airdate = m_end_date;
            auto end_iter = std::upper_bound(m_guide.begin(), m_guide.end(), ge);
            index_end = end_iter - m_guide.begin();
        }
        else
            index = 0;
    }

    // At the end already?
    if (index > m_guide.size() - 1)
    {
        LogMsgWin(L"GetFilteredEpisode(): All done");
        return false;
    }


    if (m_missed_edpisodes_only)
    {
        // Just return missed episodes. Ignore dates completely.
        while (true)
        {
            if (m_guide[ index ].episode_flags & episodeflags::EP_FL_NOT_GOT)
            {
                CopyGuideEntryToScheduleListEntry(&m_guide[index], sle);
                index++;
                return true;
            }

            index++;
            if (index >= index_end)
                return false;
        }
    }
    else
    {
        // Normal date filtering
        if (index >= index_end)
            return false;

        // Episode was listed on epguides.com with either no airdate or a stupid airdate
        if (
            (m_guide[ index ].airdate.is_not_a_date()) ||
            (m_guide[ index ].airdate.is_infinity())
           )
            return false;
    }

    CopyGuideEntryToScheduleListEntry(&m_guide[ index ], sle);

    ++index;

    return true;
}














void model::AddNewShow(const show& showtoadd)
{
    m_active_shows.push_back(showtoadd);        // Use std::move ? TODO
}


bool model::UpdateShow(const show& showtoupdate)
{
    show* originalShow = FindShow(showtoupdate.hash, eShowList::ACTIVE);
    if (originalShow == nullptr) {
        LOG_PRINT(eLogFlags::MODEL, L"UpdateShow() hash not found!\n");
        return false;
    }

    // Save all current episode flags in a map with the episode number as the key
    std::map<std::string, episodeflags> ep_flags_map;
    for (const auto& ep : originalShow->episodes)
        ep_flags_map[ep.ep_num] = ep.ep_flags;

    // Put the show's episodes in the database - lock the 'database' while we're twiddling.
    CSingleLock dbLock(&m_critical);
    VERIFY(dbLock.Lock());

    // Copy over URLs and stuff from the downloaded show into the original show. Preserve the original show flags.
    originalShow->episodes = showtoupdate.episodes;
    originalShow->title = showtoupdate.title;
    originalShow->imdb_url = showtoupdate.imdb_url;
    // This URL is 'guessed' from the show title - don't do it twice or you'll overwrite any manual corrections.
    if (originalShow->tvmaze_url.empty())
        originalShow->tvmaze_url = showtoupdate.tvmaze_url;
    originalShow->thetvdb_url = showtoupdate.thetvdb_url;
    originalShow->state |= showstate::SH_ST_UPDATED;
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
                LogMsgWin(L"New episode %s %s\n", originalShow->title.c_str(), ep.ep_num.c_str());
            }
        }
    }

    return true;
}






/**
 * Done when program launches & when the time rolls over past midnight
 *
 */
void model::SetTodaysDate()
{
    m_today = gregorian::day_clock::local_day();
    EvalScheduleDateRange();
}

void model::EvalScheduleDateRange()
{
    m_start_date = m_today - gregorian::date_duration(m_pre_days);
    m_end_date   = m_today + gregorian::date_duration(m_post_days);
}

void model::SetDateInterval(int lower, int upper)
{
    m_pre_days  = lower;
    m_post_days = upper;
    EvalScheduleDateRange();
}




bool model::ArchiveShow(DWORD hash)
{
    auto funcObject = fnoMatchShowHash(hash);
    auto show = std::find_if(m_active_shows.begin(), m_active_shows.end(), funcObject);

    // This shouldn't happen!
    if (show == m_active_shows.end())
        return false;

    show->flags |= showflags::SH_FL_ARCHIVED;

    m_archive_shows.push_back(*show);
    m_active_shows.erase(show);

    BuildEpisodeList();

    return true;
}




bool model::UnarchiveShow(DWORD hash)
{
    auto funcObject = fnoMatchShowHash(hash);
    auto show = std::find_if(m_archive_shows.begin(), m_archive_shows.end(), funcObject);

    // This shouldn't happen!
    if (show == m_archive_shows.end())
        return false;

    show->flags &= ~showflags::SH_FL_ARCHIVED;

    m_active_shows.push_back(*show);
    m_archive_shows.erase(show);

    BuildEpisodeList();

    return true;
}




bool model::EpisodeFlagsChange(const sPopupContext* pcontext)
{
    DWORD hash = pcontext->show_hash;
    bool retval = true;

    show* pshow = FindShow(hash, eShowList::ACTIVE);
    if (pshow != nullptr)
    {
        fnoMatchEpisodeNumber  fnoMatchEpNum(pcontext->ep_num);
        auto ep_ref = std::find_if(pshow->episodes.begin(), pshow->episodes.end(), fnoMatchEpNum);

        if (ep_ref == pshow->episodes.end())
        {
            LogMsgWin(L"EpisodeFlagsChange(): Episode not found");
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
 * Open an explorer window in the datafile location
 *
 */
void model::OpenDataFileFolder(void)
{
    wchar_t filepath[ MAX_PATH ];

    StringCchCopy(filepath, MAX_PATH, m_datafile.Filename());
    PathRemoveFileSpec(filepath);

    ShellExecute(NULL, L"open", filepath, NULL, NULL, SW_SHOWDEFAULT);
}

