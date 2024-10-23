#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <iostream>
#include <string>

#include "boost/algorithm/string/case_conv.hpp"

#include "Cshow.h"
#include "utils.hpp"




/***
 * Input a single show
 * 
 */
std::istream& operator>> (std::istream& istream, show& s)
{
    unsigned numEpisodes, uint_flags;
    std::string str_epguides_url, str_imdb_url, str_tvmaze_url, str_thetvdb_url;
    std::string str_title;
    std::string str_flags;
    std::string str;

    std::getline(istream, str_title);
    std::getline(istream, str_flags);
    std::getline(istream, str_epguides_url);
    std::getline(istream, str_tvmaze_url);
    std::getline(istream, str_imdb_url);
    std::getline(istream, str_thetvdb_url);

    std::getline(istream, str);
    std::stringstream(str) >> numEpisodes;

    // Make sure URLs are lowercase for consistent hash calculations
    boost::to_lower(str_imdb_url);
    boost::to_lower(str_tvmaze_url);
    boost::to_lower(str_thetvdb_url);
    boost::to_lower(str_epguides_url);

    s.title = str_title.c_str();
    std::stringstream(str_flags) >> uint_flags;
    s.flags = static_cast<showflags>(uint_flags);

    s.epguides_url = str_epguides_url;
    s.tvmaze_url   = str_tvmaze_url;
    s.imdb_url     = str_imdb_url;
    s.thetvdb_url  = str_thetvdb_url;

    s.hash  = SimpleHash(s.epguides_url);
    s.state = showstate::SH_ST_LOADED;

    for (unsigned i = 0; i < numEpisodes; i++)
    {
        episode ei;

        istream >> ei;
        s.episodes.push_back(ei);
    }

    // Evaluate last/next air dates
    EvalShowLastNextDates(&s);

    return istream;
}




/***
 * Output a single show
 * 
 */
std::ostream& operator<< (std::ostream& ostream, const show& s)
{
    ostream << s.title        << std::endl;
    ostream << s.flags        << std::endl;
    ostream << s.epguides_url << std::endl;
    ostream << s.tvmaze_url   << std::endl;
    ostream << s.imdb_url     << std::endl;
    ostream << s.thetvdb_url  << std::endl;

    ostream << s.episodes.size() << std::endl;

    for (const episode& ep : s.episodes)
        ostream << ep;

    return ostream;
}

