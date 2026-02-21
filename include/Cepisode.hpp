#pragma once

#pragma warning( disable : 26812 )

#include <iostream>
#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;





enum episodeflags : unsigned
{
    EP_FL_NONE    = 0x00000000,
    EP_FL_GOT     = 0x00000001,   // Got this episode
    EP_FL_NOT_GOT = 0x00000002    // Don't have this episode
};

DEFINE_ENUM_FLAG_OPERATORS(episodeflags)





class episode
{
public:
    episode() : ep_flags(episodeflags::EP_FL_NONE){};

    friend std::ostream& operator<< (std::ostream& wstream, const episode& ep);
    friend std::istream& operator>> (std::istream& wstream, episode& ep);

    std::string         error;

    std::string         ep_num;
    std::string         ep_title;
    std::string         ep_date_string;
    episodeflags        ep_flags;
    gregorian::date     ep_date;

};


