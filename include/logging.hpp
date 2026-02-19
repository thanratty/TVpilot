#pragma once

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <string>




#define     NUM_LOG_FLAGS       13                  // !!! MUST MATCH THE NUMBER OF eLogFlags defined below !!!

enum class eLogFlags : UINT32
{
    INFO            = 0x00000001,
    SLOT_THREAD     = 0x00000002,
    WM_MSGS         = 0x00000004,
    MODEL           = 0x00000008,
    CURL            = 0x00000010,
    XML             = 0x00000020,
    FATAL           = 0x00000040,

    SLOT_LOCK       = 0x00000080,
    SLOT_USE        = 0x00000100,
    SYNC_OBJECTS    = 0x00000200,

    TEST            = 0x00000400,
    CONSOLE_ECHO    = 0x00000800,
    APP_EVENT       = 0x00001000,
    //
    // Excluse these last two from the NUM_LOG_FLAGS total above
    //
    ALL             = 0x00001FFF,
    NONE            = 0x00000000
};

ENUM_FLAGS(eLogFlags)





typedef struct tag_sLogFlags {
    eLogFlags           mask;
    const wchar_t*      description;
} sLogFlagDef;





/**
 * Optionally display a separate text console for loggin & debug messages.
 */
#if defined(_DEBUG) && (ENABLE_CONSOLE_LOGGING==1)

void        LOG_INIT( void );
void        LOG_PRINT(eLogFlags type, const wchar_t* format, ...);
void        LOG_PRINT(eLogFlags type, const char* format, ...);
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

void LogMsgWin(const wchar_t* format, ...);
void LogMsgWin(const char* format, ...);
void LogMsgWin(const std::string& str);
void LogMsgWin(const CString& msg);


eLogFlags GetLogFlags();
void      SetLogFlags(eLogFlags newflags);

