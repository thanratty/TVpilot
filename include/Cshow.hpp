#pragma once

#include "config.h"
#include "pch.h"

//--

#include <iostream>
#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;

#include "Cepisode.hpp"


/**
 * 'showstate' is used internally & not saved to disk with other show information
 */
enum showstate : unsigned
{
    SH_ST_NONE           = 0x00000000,   // Uninitialised newly created show object
    SH_ST_LOADED         = 0x00000004,   // Show details loaded from disk
    SH_ST_WAITING        = 0x00000008,   // Waiting to be updated from internet
    SH_ST_UPDATING       = 0x00000010,   // Update thread created
    SH_ST_UPDATED        = 0x00000020,   // Show has been updated from internet
    SH_ST_UPDATE_FAILED  = 0x00000040,   // Updating the show and/or episodes from the internet encountered an error
    SH_ST_DELETED        = 0x00000080,   // Deleted, pending purge
    SH_ST_NEW_SHOW       = 0x00000100    // New show created from entered URL
};

DEFINE_ENUM_FLAG_OPERATORS(showstate)




enum showflags : unsigned
{
    SH_FL_NONE          = 0x00000000,    // Newly minted default value
    SH_FL_ARCHIVED      = 0x00000002,    // Don't include in guide list or download updates
};

DEFINE_ENUM_FLAG_OPERATORS(showflags)





// This really is just data. Make it a struct?

class show
{
public:

    friend std::ostream& operator<< (std::ostream& wstream, const show& s);
    friend std::istream& operator>> (std::istream& wstream, show& s);

    void Initialise()
    {
        title.clear();
        epguides_url.clear();
        tvmaze_url.clear();
        imdb_url.clear();
        thetvdb_url.clear();

        hash  = 0;
        flags = SH_FL_NONE;
        state = SH_ST_NONE;
 
        last_airdate_string.clear();
        next_airdate_string.clear();
        next_episode_number.clear();
        last_airdate = gregorian::date(date_time::not_a_date_time);
        next_airdate = gregorian::date(date_time::not_a_date_time);

        episodes.clear();
    }

    std::string         title;
    std::string         epguides_url;
    std::string         tvmaze_url;
    std::string         imdb_url;
    std::string         thetvdb_url;

    size_t              hash{ 0 };
    showflags           flags{ showflags::SH_FL_NONE };
    showstate           state{ showstate::SH_ST_NONE };         // Ephemeral - do not save to disk.

    gregorian::date     last_airdate;
    std::string         last_airdate_string;
    gregorian::date     next_airdate;
    std::string         next_airdate_string;

    std::string         next_episode_number;

    std::vector<episode> episodes;

};

