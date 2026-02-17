#pragma once

/**
 * Both DEBUG and RELEASE
 */
#define         USE_TEST_DATAFILE               0       // Force use of alternative datafile for testing NB Both DEBUG & RELEASE
#define         KEEP_BUTTONS_ENABLED            0       // Keep all dialog buttons enabled for ease of testing/debugging
#define			CURL_MAX_TRIES                  3       // Max # curl download attempts including retries
#define			CURL_RETRY_DELAY                2000    // Delay 2 seconds after a Curl download error
#define         CURL_REPLY_TIMEOUT              8000    // Allow 8 seconds for https transaction to complete
#define         NAMED_OBJECTS                   1       // Events, Semaphores and Threds are created with names
#define         NUMBER_OF_DOWNLOAD_THREADS      15      // Number of worker threads created to do the actual downloading

 /**
  * DEBUG configuration only
  */
#define			SAVE_WEBPAGE_ON_ERROR		    0		// Creates file webpage.txt if there's an XML parse error [ DEBUG build only ]
#define         PAUSE_BEFORE_EXIT               0       // Waits for an 'OK' before closing. Allows copying console & log window contents etc. [DEBUG build only]
#define         ENABLE_CONSOLE_LOGGING          1       // Create a console window for log messages. Finer control in logging.hpp
#define         ECHO_MSG_WINDOW_TO_CONSOLE      1
#define         TRACE_APP_EVENTS                0       // Print all eAppevents in the message window [ DEBUG build only ]


/**
 * Release version
 */
#define         VERSION_NUMBER                  "1.1"





/**
 * Use different data file name for release and debug builds.
 *   Release build file is in the folder %APPDATA%\APP_NAME
 *   Debug build file is in the same folder as the executable.
 */
#define     TEST_DATAFILE_NAME          L"\\TV-Pilot-Test"
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



