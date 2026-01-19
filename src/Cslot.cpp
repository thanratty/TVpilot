#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "Cslot.hpp"
#include "CdownloadManager.hpp"
#include "model.hpp"
#include "CcurlJob.hpp"
#include "xmlParse.hpp"
#include "CsyncObjects.hpp"
#include "threadData.hpp"
#include "threadFuncs.hpp"



std::vector<Cslot> gSlots = std::vector<Cslot>(NUM_WORKER_THREADS);


