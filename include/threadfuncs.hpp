#pragma once

#include "pch.h"



// The worker threads do the actual downloading & parsing
UINT __cdecl thrWorkerThread(LPVOID pParam);

// Control threads to handle: download requests, results available, results processed
UINT __cdecl thrRequest(LPVOID pParam);
UINT __cdecl thrResults(LPVOID pParam);
UINT __cdecl thrRelease(LPVOID pParam);


