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
    //
    ALL          = 0x0000007F
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


#if defined(_DEBUG) && (CONSOLE_LOGGING_ENABLED==1)

void    LOG_INIT( void );
void    LOG_WRITE(eLogFlags type, const wchar_t* format, ...);
void    LOG_EXIT( void );

#else

#define     LOG_INIT()          do {} while(0)
#define     LOG_WRITE(x,...)    do {} while(0)
#define     LOG_EXIT()          do {} while(0)

#endif


