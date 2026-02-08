#pragma once

#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <string>

#include "common.hpp"


#define     ENABLE_THREAD_DATA_LOGGING      0       // Dbg message thread data classes constructors/destructors & Cslot & threads
#define     ENABLE_THREAD_FUNC_LOGGING      0       // Dbg message thread data classes constructors/destructors & Cslot & threads
#define     ENABLE_SYNC_OBJECT_LOGGING      0       // Must also have console window enabled to work [DEBUG build only]










enum class eLogFlags : UINT32
{
    LOCK         = 0x00000001,
    UNLOCK       = 0x00000002,
    DL_THREADS   = 0x00000004,
    INFO         = 0x00000008,
	WM_MSGS      = 0x00000010,
    THREAD_DATA  = 0x00000020,
    THREAD_FUNC  = 0x00000040,
    SYSTEM       = 0x00000080,
    MODEL        = 0x00000100,
    SYNC_OBJECTS = 0x00000200,
    CURL         = 0x00000400,
    XML          = 0x00000800,
    TEST         = 0x00001000,
    //
    ALL          = 0x00001FFF,
    NONE         = 0x00000000
};

ENUM_FLAGS(eLogFlags)





/**
 * Optionally display a separate text console for loggin & debug messages.
 */
#if (ENABLE_CONSOLE_LOGGING==1)

void        LOG_INIT( void );
void        LOG_PRINT(eLogFlags type, const wchar_t* format, ...);
void        LOG_EXIT( void );

#else

#define     LOG_INIT()          do {} while(0)
#define     LOG_PRINT(x,...)    do {} while(0)
#define     LOG_EXIT()          do {} while(0)

#endif





/**
 * Routines for writing debug/trace messages to the message window. This window is always
 * available in both RELEASE & DEBUG builds.
 */

void LogSetMsgWin(CEdit* pedit);
void LogMsgWin(CString& msg);
void LogMsgWin(const char* pchars);
void LogMsgWin(const wchar_t* pwchars);
void LogMsgWin(const std::string& str);





