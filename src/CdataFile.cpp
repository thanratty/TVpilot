#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <iostream>
#include <fstream>

#include "common.hpp"

#include "CdataFile.hpp"





/** In a release build, the database is in %APPDATALOCAL%\TVpilot
 *  For debugging, the local database is in same folder as the executable
 */
#if USE_TEST_DATAFILE==0
constexpr wchar_t* DATAFILE_NAME = RELEASE_DATAFILE_NAME;
#else
#pragma message ("!! CONFIGURED FOR TEST DATA FILE")
constexpr wchar_t* DATAFILE_NAME = TEST_DATAFILE_NAME;
#endif





CdataFile::CdataFile()
{
    BuildFilename();

    if (PathFileExists(m_filename.c_str()) == TRUE)
        return;

    int yesno = AfxMessageBox(L"No database found. Do you want to create an new empty database?", MB_ICONQUESTION | MB_YESNO | MB_APPLMODAL);
    if (yesno == IDNO)
        ExitProcess( E_APP_DATABASE) ;

    m_IsNewFile = true;

    HANDLE result = CreateFile( m_filename.c_str(),                     // Filename
                                (GENERIC_READ | GENERIC_WRITE),         // Desired access
                                0,                                      // Share mode
                                NULL,                                   // Security attributes
                                CREATE_ALWAYS,                          // Creation disposition
                                FILE_ATTRIBUTE_NORMAL,                  // Flags
                                NULL);                                  // Template file

    if (result == INVALID_HANDLE_VALUE) {
        AfxMessageBox(L"Can't create database, Program will now terminate.", MB_ICONERROR | MB_OK | MB_APPLMODAL);
        ExitProcess( E_APP_DATABASE );
    }
    CloseHandle(result);

    AfxMessageBox(L"Empty database created", MB_OK | MB_APPLMODAL);
}




void CdataFile::BuildFilename()
{
wchar_t  buffer[MAX_PATH + 1] = {};

    PWSTR ppath = nullptr;

    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &ppath);
    wcscpy_s(buffer, MAX_PATH, ppath);
    CoTaskMemFree(ppath);

    // Our folder is under %APPDATALOCAL%
    wcscat_s(buffer, MAX_PATH, L"\\");
    wcscat_s(buffer, MAX_PATH, APP_NAME);

    if (PathFileExists(buffer) == FALSE)
        if (CreateDirectory(buffer, NULL) == FALSE)
        {
            AfxMessageBox(L"Can't create data folder. Aborting.", MB_ICONERROR | MB_OK | MB_APPLMODAL);
            AfxPostQuitMessage(1);
        }

    m_filename = buffer;
    m_filename.append(DATAFILE_NAME);
}


