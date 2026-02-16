#pragma once

#include "pch.h"

#include "common.hpp"


enum class eThreadState : UINT32
{
    TS_NOT_STARTED,
    TS_WAITING,
    TS_RUNNING,
    TS_FINISHED
};



enum eThreadResult : UINT32
{
    TR_UNKNOWN,
    TR_NORMAL_EXIT,
    TR_ERROR_STALL
};





//
// Each slot has a worker thread manages the actual downloading & parsing.
// 
UINT __cdecl thrSlotThread(LPVOID pParam);


