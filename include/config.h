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
#define         SAVE_WEBPAGE_ON_ERROR           0       // Creates file webpage.txt if there's an XML parse error [ DEBUG only ]
#define         PAUSE_BEFORE_EXIT               1       // Wait for <RETURN> key in the console. Allows copying console window contents etc. [DEBUG only]
#define         TRACE_APP_EVENTS                0       // Print all eAppevents in the message window [ DEBUG only ]



/**
 * Displayed release version
 */
#define         VERSION_NUMBER                  "2.1"



/**
 * Use different filename for RELEASE and DEBUG builds depending
 * on USE_TEST_DATAFILE defined above.
 */
#define         TEST_DATAFILE_NAME              L"\\TV-Pilot-Test"          // DEBUG build only if enabled above
#define         RELEASE_DATAFILE_NAME           L"\\TV-Pilot-Data"



/**
 * Name of the executable
 */
#define         APP_NAME                        L"TVpilot"


