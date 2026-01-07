#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <string>
#include <iostream>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;

#include "Cepisode.hpp"




/***
 * Input a single episode
 */
std::istream& operator>> (std::istream& istream, episode& ep)
{
    std::string str_ep_num;
    std::string str_ep_date;
    std::string str_ep_title;
    std::string str_ep_flags;

    std::getline(istream, str_ep_num);
    std::getline(istream, str_ep_date);
    std::getline(istream, str_ep_title);
    std::getline(istream, str_ep_flags);

    ep.ep_date = gregorian::from_string(str_ep_date);
    ep.ep_date_string = gregorian::to_simple_string(ep.ep_date);
    ep.ep_num = str_ep_num;
    ep.ep_title = str_ep_title;
    ep.ep_flags = static_cast<episodeflags>(std::stoi(str_ep_flags));

    return istream;
}



/***
 * Output a single episode
 */
std::ostream& operator<< (std::ostream& ostream, const episode& ep)
{
    ostream << ep.ep_num << std::endl;
    ostream << ep.ep_date << std::endl;
    ostream << ep.ep_title << std::endl;
    ostream << ep.ep_flags << std::endl;

    return ostream;
}





