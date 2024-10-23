#pragma once

#include "resource.h"		// main symbols



#if (NAME_SYNC_OBJECTS==1)
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



// Define our own private Windows Messages
//
constexpr int WM_DOWNLOAD_COMPLETE		 = (WM_USER + 7000);
constexpr int WM_DOWNLOAD_PING			 = (WM_USER + 7001);
constexpr int WM_ZOOM_EPISODES			 = (WM_USER + 7002);
constexpr int WM_LAUNCH_URL				 = (WM_USER + 7003);
constexpr int WM_SHOW_CONTEXT_MENU		 = (WM_USER + 7004);
constexpr int WM_SIGNAL_APP_EVENT		 = (WM_USER + 7005);
constexpr int WM_ABORT_DOWNLOAD          = (WM_USER + 7006);
//
constexpr int WM_SCHED_EP_FLAGS_CHANGED  = (WM_USER + 8000);



// Status and return codes
//
constexpr UINT E_THREAD_OK = 0;
constexpr UINT E_THREAD_CURL = 1;
constexpr UINT E_THREAD_XML = 2;
constexpr UINT E_THREAD_PARSE = 3;
constexpr UINT E_THREAD_DOC_ERR = 4;

constexpr UINT E_APP_OK = 0;
constexpr UINT E_APP_DATABASE = 1;


