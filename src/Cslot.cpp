#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "Cslot.hpp"



//
// The global array of download slots. This is 
//
std::vector<Cslot> gSlots = std::vector<Cslot>( NUMBER_OF_DOWNLOAD_THREADS );


