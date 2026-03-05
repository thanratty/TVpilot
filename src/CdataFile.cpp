#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <iostream>
#include <fstream>

#include "common.hpp"

#include "CdataFile.hpp"





/** The data file is always in %APPDATALOCAL%\TVpilot
 */
#if defined(_DEBUG) && (USE_TEST_DATAFILE==1)
#pragma message ("!! --- DEBUG BUILD CONFIGURED TO USE TEST DATA FILE --- !!")
constexpr wchar_t DATAFILE_NAME[]{ TEST_DATAFILE_NAME };
#else
constexpr wchar_t DATAFILE_NAME[]{ RELEASE_DATAFILE_NAME };
#endif





/**
 *
 * Check the datafile exists, optionally create a new one if it doesn't.
 * 
 */
CdataFile::CdataFile()
{
    // Concat folder & filename into full path m_filename
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




/**
 *
 * Construct the full pathname of the data file in %APPDATALOCAL%\APP_NAME
 * and save in m_filename. If the folder doesn't exist, create it.
 * 
 */
void CdataFile::BuildFilename()
{
wchar_t  buffer[MAX_PATH + 1]{};
PWSTR    ppath{ nullptr };

    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &ppath);
    wcscpy_s(buffer, MAX_PATH, ppath);
    CoTaskMemFree(ppath);

    // Our folder is under %APPDATALOCAL%
    wcscat_s(buffer, MAX_PATH, L"\\");
    wcscat_s(buffer, MAX_PATH, APP_NAME);

    if (PathFileExists(buffer) == FALSE)
    {
        if (CreateDirectory(buffer, NULL) == FALSE)
        {
            AfxMessageBox(L"Can't create data folder. Aborting.", MB_ICONERROR | MB_OK | MB_APPLMODAL);
            AfxPostQuitMessage(1);
        }
    }

    m_filename = buffer;
    m_filename.append(DATAFILE_NAME);
}


