#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "curl/curl.h"
#include "libxml/HTMLparser.h"

#include "common.hpp"
#include "CepcheckDlg.h"
#include "utils.hpp"
#include "debugConsole.h"

#include "TVpilot.h"



// CepcheckApp

BEGIN_MESSAGE_MAP(CepcheckApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CepcheckApp construction

CepcheckApp::CepcheckApp()
{
	// Place all significant initialization in InitInstance
}


// The one and only CepcheckApp object

CepcheckApp theApp;


// CepcheckApp initialization

BOOL CepcheckApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));


	CURLcode cc = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (cc != CURLE_OK)
	{
		AfxMessageBox(L"Curl init failed", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// Initialise libxml2 library
	xmlInitParser();

	// Depending on config.h this creates a console for debug messages
	OpenDebugConsole();

	/**
	 * We used to need the CSV file for TVmaze show numbers but we now
	 * scrape that info from epguides along with the IMDB url.
	 */

#if DOWNLOAD_ALL_SHOWS_CSV==1
	ReadAllShowsCsvFile();
#endif


	CepcheckDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// Nothing special
	}
	else if (nResponse == IDCANCEL)
	{
		// Nothing special
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}



	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif


	xmlCleanupParser();
	curl_global_cleanup();

	CloseDebugConsole();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


