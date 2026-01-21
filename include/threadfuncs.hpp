#pragma once

#include "pch.h"



// Thread status & return codes.
//
enum eThreadResult : UINT
{
    TR_UNKNOWN,
    TR_OK,
    TR_NORMAL_EXIT,
    TR_CURL_ERROR,
    TR_XML_ERROR,
    TR_PARSE_ERROR,
    TR_DOC_ERROR
};


enum class eThreadState : UINT
{
    TS_UNKNOWN,
    TS_RUNNING,
    TS_FINISHED
};






//
// This worker thread manages the actual downloading & parsing. One thread per slot.
// 
UINT __cdecl thrSlotThread(LPVOID pParam);



//
// Control threads to handle: download requests, results available, results processed
//
UINT __cdecl thrRequests(LPVOID pParam);
UINT __cdecl thrResults(LPVOID pParam);
UINT __cdecl thrReleases(LPVOID pParam);


