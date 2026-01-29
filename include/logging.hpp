#pragma once

#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"


#define     ENABLE_THREAD_DATA_LOGGING      1       // Dbg message thread data classes constructors/destructors & Cslot & threads
#define     ENABLE_THREAD_FUNC_LOGGING      1       // Dbg message thread data classes constructors/destructors & Cslot & threads
#define     ENABLE_SYNC_OBJECT_LOGGING      1       // Must also have console window enabled to work [DEBUG build only]










enum class eLogFlags : UINT32
{
    LOCKS        = 0x00000001,
    UNLOCKS      = 0x00000002,
    DL_THREADS   = 0x00000004,
    INFO         = 0x00000008,
	WM_MSGS      = 0x00000010,
    THREAD_DATA  = 0x00000020,
    THREAD_FUNC  = 0x00000040,
    SYSTEM       = 0x00000080,
    MODEL        = 0x00000100,
    SYNC_OBJECTS = 0x00000200,
    //
    ALL          = 0x000000FF
};



inline constexpr eLogFlags
operator&(eLogFlags x, eLogFlags y) {
    return static_cast<eLogFlags>
        (static_cast<UINT32>(x) & static_cast<UINT32>(y));
}

inline constexpr eLogFlags
operator|(eLogFlags x, eLogFlags y) {
    return static_cast<eLogFlags>
        (static_cast<UINT32>(x) | static_cast<UINT32>(y));
}

inline constexpr eLogFlags
operator^(eLogFlags x, eLogFlags y) {
    return static_cast<eLogFlags>
        (static_cast<UINT32>(x) ^ static_cast<UINT32>(y));
}

inline constexpr eLogFlags
operator~(eLogFlags x) {
    return static_cast<eLogFlags>(~static_cast<UINT32>(x));
}

inline eLogFlags&
operator&=(eLogFlags& x, eLogFlags y) {
    x = x & y;
    return x;
}

inline eLogFlags&
operator|=(eLogFlags& x, eLogFlags y) {
    x = x | y;
    return x;
}

inline eLogFlags&
operator^=(eLogFlags& x, eLogFlags y) {
    x = x ^ y;
    return x;
}




/**
 * Optionally display a separate text console for loggin & debug messages.
 */
#if (CONSOLE_LOGGING_ENABLED==1)

void        LOG_INIT( void );
void        LOG_WRITE(eLogFlags type, const wchar_t* format, ...);
void        LOG_EXIT( void );

#else

#define     LOG_INIT()          do {} while(0)
#define     LOG_WRITE(x,...)    do {} while(0)
#define     LOG_EXIT()          do {} while(0)

#endif





/**
 * Routines for writing debug/trace messages to the message window. This window is always
 * available in both RELEASE & DEBUG builds.
 */
void SetMsgWindow(CEdit* pedit);

void LogMsgWindow(CString& msg);
void LogMsgWindow(const char* pchars);
void LogMsgWindow(const wchar_t* pwchars);
void LogMsgWindow(const std::string& str);





