#pragma once

/**
 * Settings applied to Both DEBUG and RELEASE
 */
#define         KEEP_BUTTONS_ENABLED            0       // Keep load/save/download dialog buttons enabled for ease of testing/debugging
#define         CURL_MAX_TRIES                  3       // Max # curl download attempts including retries
#define         CURL_RETRY_DELAY                2000    // Delay 2 seconds after a Curl download error
#define         CURL_REPLY_TIMEOUT              8000    // Allow 8 seconds for https transaction to complete
#define         NAMED_OBJECTS                   1       // Events, Semaphores and Threds are created with names
#define         NUMBER_OF_DOWNLOAD_THREADS      15      // Number of worker threads created to do the actual downloading

 /**
  * Settings only active for DEBUG configuration
  */
#define         USE_TEST_DATAFILE               1       // Force use of alternative datafile for testing NB Both DEBUG & RELEASE
#define         ENABLE_CONSOLE_LOGGING          1       // Create a console window for log messages. Finer control in logging.hpp
#define         ECHO_MSG_WINDOW_TO_CONSOLE      1
#define         SAVE_WEBPAGE_ON_ERROR           0       // Creates file webpage.txt if there's an XML parse error [ DEBUG build only ]
#define         PAUSE_BEFORE_EXIT               0       // Waits for an 'OK' before closing. Allows copying console & log window contents etc. [DEBUG build only]
#define         TRACE_APP_EVENTS                0       // Print all eAppevents in the message window [ DEBUG build only ]


/**
 * Displayed release version
 */
#define         VERSION_NUMBER                  "2.0"





/**
 * Use different data file name for release and debug builds.
 *   RELEASE date file is in the folder %APPDATA%\APP_NAME
 *   DEBUG date file is in the same folder as the executable.
 */
#define     TEST_DATAFILE_NAME          L"\\TV-Pilot-Test"          // DEBUG build only if enabled above
#define     RELEASE_DATAFILE_NAME       L"\\TV-Pilot-Data"



/**
 * Name of the executable
 */
#define     APP_NAME                    L"TVpilot"


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



