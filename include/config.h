#pragma once


#define         ENABLE_CONSOLE_WINDOW       0       // Create a console window for debug messages [DEBUG build only]
#define         PAUSE_BEFORE_EXIT           0       // Waits for an 'OK' before closing. Allows copying console & log window contents etc. [DEBUG build only]

#define         USE_LOCAL_DATAFILE          0       // Use alternative datafile in exe folder for debugging [CdataFile.cpp for filenames]
#define         SHOW_APP_EVENTS             0       // Print all appevents in the message window [ DEBUG build only ]
#define         KEEP_BUTTONS_ENABLED        0       // Keep all buttons enabled for ease of testing/debugging
#define			SAVE_WEBPAGE_ON_ERROR		0		// Creates file webpage.txt if there's an XML parse error [ DEBUG build only ]

#define			NUMBER_OF_CURL_TRIES        5       // ie once with four retries
#define			CURL_RETRY_DELAY            2000    // Delay 2 seconds after a Curl download error
#define         CURL_REPLY_TIMEOUT          8000    // Allow 8 seconds for https transaction to complete

#define         TRACE_SYNC_OBJECTS          0       // Must also have console window enabled to work [DEBUG build only]
#define         NAME_SYNC_OBJECTS           0       // Windows Events & Semaphores are created with names

#define         NUM_WORKER_THREADS          15      // Number of worker threads created to do the actual downloading




/**
 * Use different data file name for release and debug builds.
 *   Release build file is in the folder %APPDATA%\APP_NAME
 *   Debug build file is in the same folder as the executable.
 */
#define     LOCAL_DATAFILE_NAME         L"\\tvpilotlocal"
#define     RELEASE_DATAFILE_NAME       L"\\TV-Pilot-Data"

/**
 * Name of the executable
 */
#define     APP_NAME                    L"TVpilot"





#ifdef _DEBUG
#define		STATIC
#else
#define		STATIC      static
#endif




// Declare COMPILE_MESSAGE() macro for outputting compile time messages from the preprocessor
//
#define COMPILE_MESSAGE(t) message(__FILE__ "(" __STRINGXXX(__LINE__) ") : " t)
#define __STRINGXXX(x) __STRINGYYY(x)
#define __STRINGYYY(x) #x



