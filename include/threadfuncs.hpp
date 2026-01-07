#pragma once

#include "pch.h"



// These worker threads manage the actual downloading & parsing. One thread per slot.
UINT __cdecl thrSlotThread(LPVOID pParam);

// Control threads to handle: download requests, results available, results processed
UINT __cdecl thrRequest(LPVOID pParam);
UINT __cdecl thrResults(LPVOID pParam);
UINT __cdecl thrRelease(LPVOID pParam);


