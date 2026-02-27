#pragma once

#include "config.h"

#include "resource.h"		// Need the resource symbols
#include "enum_flags.h"     // Define bitwose operators for enums

#include "logging.hpp"



#if (NAMED_OBJECTS==1)
#define     CREATE_EVENT(a,b,c,d)           CreateEvent(a,b,c,d)
#define     CREATE_SEMAPHORE(a,b,c,d)       CreateSemaphore(a,b,c,d)
#else
#define     CREATE_EVENT(a,b,c,d)           CreateEvent(a,b,c, NULL)
#define     CREATE_SEMAPHORE(a,b,c,d)       CreateSemaphore(a,b,c,NULL)
#endif



// Handy colours
//
#define		rgbRed			RGB(255,0,0)
#define		rgbGreen		RGB(0,255,0)
#define		rgbLime			RGB(0,128,0)
#define		rgbBlack		RGB(0,0,0)
#define		rgbTeal			RGB(0,128,128)
#define		rgbMagenta		RGB(255,0,255)
#define		rgbPurple		RGB(128,0,128)



// Define our own private Windows Messages, all prefixed with WM_TVP_
//
constexpr int WM_TVP_DOWNLOAD_COMPLETE	 = (WM_USER + 7000);
constexpr int WM_TVP_DOWNLOAD_PING			 = (WM_USER + 7001);
constexpr int WM_TVP_ZOOM_EPISODES			 = (WM_USER + 7002);
constexpr int WM_TVP_LAUNCH_URL				   = (WM_USER + 7003);
constexpr int WM_TVP_SHOW_CONTEXT_MENU	 = (WM_USER + 7004);
constexpr int WM_TVP_SIGNAL_APP_EVENT		 = (WM_USER + 7005);
constexpr int WM_TVP_ABORT_BTN_ENABLE    = (WM_USER + 7006);
constexpr int WM_TVP_ABORT_BTN_DISABLE   = (WM_USER + 7007);
constexpr int WM_TVP_ABORT_DOWNLOAD      = (WM_USER + 7008);
constexpr int WM_TVP_THREAD_WAIT_FAIL    = (WM_USER + 7009);
//
constexpr int WM_TVP_SCHED_EP_FLAGS_CHANGED  = (WM_USER + 8000);




constexpr UINT E_APP_OK = 0;
constexpr UINT E_APP_DATABASE = 1;



constexpr DWORD THREAD_EXIT_TIMEOUT = 4000;






/**
 * For a DEBUG build it's handy to have ALL symbols in the map file
 */
#ifdef _DEBUG
#define		STATIC
#else
#define		STATIC      static
#endif




 /**
  * Declare COMPILE_MESSAGE() macro for outputting compile time messages from the preprocessor
  */
#define COMPILE_MESSAGE(t) message(__FILE__ "(" __STRINGXXX(__LINE__) ") : " t)
#define __STRINGXXX(x) __STRINGYYY(x)
#define __STRINGYYY(x) #x



