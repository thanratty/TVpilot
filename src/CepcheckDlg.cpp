#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <regex>

#include "strsafe.h"
#include "pathcch.h"

#include "common.hpp"

#include "CDnewShow.hpp"
#include "CDShowZoom.hpp"
#include "CDInputBox.hpp"
#include "CdownloadManager.hpp"
#include "utils.hpp"

#include "CepcheckDlg.hpp"




constexpr short MIN_SPIN_DAYS = 1;
constexpr short MAX_SPIN_DAYS = 30;


STATIC UINT_PTR	TIMER_ID_ONE_MINUTE = 60;






// CepcheckDlg dialog constructor
//
CepcheckDlg::CepcheckDlg(CWnd* pParent /*=nullptr*/) :
	CDialog(IDD_EPCHECK, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}



void CepcheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tabctrl);
	DDX_Control(pDX, IDC_SPIN_DAYS_POST,  m_spin_post);
	DDX_Control(pDX, IDC_SPIN_DAYS_PRE,   m_spin_pre);
	DDX_Control(pDX, IDC_BTN_LOAD,        m_btn_load);
	DDX_Control(pDX, IDC_BTN_SAVE,        m_btn_save);
	DDX_Control(pDX, IDC_BTN_DOWNLOAD,    m_btn_download);
	DDX_Control(pDX, IDC_BTN_NEW_SHOW,    m_btn_new_show);
	DDX_Control(pDX, IDC_BTN_DELETE_SHOW, m_btn_delete_show);
	DDX_Check(pDX,   IDC_CHK_MISSED_ONLY, m_missed_only);
}




// ON_NOTIFY macro can give a false warning
#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CepcheckDlg, CDialog)
	ON_NOTIFY( TCN_SELCHANGE, IDC_TAB1,				&CepcheckDlg::OnTcnSelchangeTab1)
	ON_NOTIFY( UDN_DELTAPOS,  IDC_SPIN_DAYS_PRE,	&CepcheckDlg::OnDeltaPosSpinDays)
	ON_NOTIFY( UDN_DELTAPOS,  IDC_SPIN_DAYS_POST,	&CepcheckDlg::OnDeltaPosSpinDays)
	ON_BN_CLICKED( IDC_BTN_LOAD,					&CepcheckDlg::OnBtn_Load)
	ON_BN_CLICKED( IDC_BTN_SAVE,					&CepcheckDlg::OnBtn_Save)
	ON_BN_CLICKED( IDC_BTN_DOWNLOAD,				&CepcheckDlg::OnBtn_Download)
	ON_BN_CLICKED( IDC_BTN_ABORT_DOWNLOAD,			&CepcheckDlg::OnBtn_AbortDownload)
	ON_BN_CLICKED( IDC_BTN_DELETE_SHOW,				&CepcheckDlg::OnBtn_DeleteShow)
	ON_BN_CLICKED( IDC_BTN_NEW_SHOW,				&CepcheckDlg::OnBtn_NewShow)
	ON_BN_CLICKED( IDC_BTN_BREAK,					&CepcheckDlg::OnBtn_Break)
	ON_BN_CLICKED( IDC_BTN_EXPLORER,				&CepcheckDlg::OnBtn_Explorer)
	ON_BN_CLICKED( IDC_BTN_RESET_DAYS,				&CepcheckDlg::OnBtn_ResetDays)
	ON_BN_CLICKED( IDC_CHK_MISSED_ONLY,             &CepcheckDlg::OnBtn_ChkMissedOnly)
	ON_BN_CLICKED( IDC_CHK_DEBUG_LOG,               &CepcheckDlg::OnBtn_ShowLog)
	ON_MESSAGE( WM_TVP_DOWNLOAD_COMPLETE,			&CepcheckDlg::OnDownloadComplete)
	ON_MESSAGE( WM_TVP_DOWNLOAD_PING,				&CepcheckDlg::OnDownloadPing)
	ON_MESSAGE( WM_TVP_ZOOM_EPISODES,				&CepcheckDlg::OnZoomEpisodes)
	ON_MESSAGE( WM_TVP_LAUNCH_URL,					&CepcheckDlg::OnLaunchUrl)
	ON_MESSAGE( WM_TVP_SHOW_CONTEXT_MENU,			&CepcheckDlg::OnShowContextMenu)
	ON_MESSAGE( WM_TVP_SIGNAL_APP_EVENT,			&CepcheckDlg::OnSignalAppEvent)
	ON_MESSAGE( WM_TVP_THREAD_WAIT_FAIL,			&CepcheckDlg::OnThreadWaitFail)
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

#pragma warning( pop )



// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CepcheckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



/**
 * Initialise all dialogs,
 *
 */
BOOL CepcheckDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add a suffix to the dialog title bar if this is a debug build

#ifdef _DEBUG
	CString str;
	GetWindowText(str);
	str += L" [DEBUG]";
	SetWindowText(str);
#endif


	// Center the text in the two counters
	GetDlgItem(IDC_PING_COUNT)->ModifyStyle(SS_LEFT, SS_CENTER);
	GetDlgItem(IDC_ERR_COUNT)->ModifyStyle(SS_LEFT, SS_CENTER);

	// Set the spin range limits & default values
	m_spin_post.SetRange( MIN_SPIN_DAYS, MAX_SPIN_DAYS );
	m_spin_post.SetPos( m_spin_post_val );
	m_spin_pre.SetRange( MIN_SPIN_DAYS, MAX_SPIN_DAYS );
	m_spin_pre.SetPos( m_spin_pre_val );
	UpdateSchedulePeriod();

	// Set the icon for this dialog. The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Add three tabs to the tab control.
	m_tabctrl.InsertItem(TAB_NUM_SHOWS,    L"Shows");
	m_tabctrl.InsertItem(TAB_NUM_SCHEDULE, L"Schedule");
	m_tabctrl.InsertItem(TAB_NUM_ARCHIVE,  L"Archive");

	// Create the client dialog boxes 
	m_dlgShows.Create(IDD_SHOWS, this);
	m_dlgSchedule.Create(IDD_SCHEDULE, this);
	m_dlgArchive.Create(IDD_ARCHIVE, this);

	// Use the tab control here as the master size. Size the show & schedule dialogs to fit it.
	CRect  DisplayArea;
	m_tabctrl.GetWindowRect(DisplayArea);
	m_tabctrl.GetParent()->ScreenToClient(DisplayArea);
	m_tabctrl.AdjustRect(FALSE, DisplayArea);
	m_dlgShows.MoveWindow(DisplayArea);
	m_dlgSchedule.MoveWindow(DisplayArea);
	m_dlgArchive.MoveWindow(DisplayArea);


	// Create the dialog box for debug messages
	m_dlgMessages.Create(IDD_MESSAGES, this);
	LogSetMsgWin(&m_dlgMessages.m_messages);

	std::ostringstream info;
	info << "Version "  VERSION_NUMBER  "  -  "  __DATE__;
	LogMsgWin(info.str().c_str());
	LogMsgWin(L"Data file : %s", m_data.Filename());

	// If this is a debug build, show the 'Break' & 'Logging' UI buttons config buttons.

#if defined(_DEBUG)

	GetDlgItem(IDC_BTN_BREAK)->EnableWindow();
	GetDlgItem(IDC_BTN_BREAK)->ShowWindow(SW_SHOW);

	m_dlgMessages.GetDlgItem(IDC_BTN_LOGGING)->EnableWindow();
	m_dlgMessages.GetDlgItem(IDC_BTN_LOGGING)->ShowWindow(SW_SHOW);

#else

	GetDlgItem(IDC_BTN_BREAK)->EnableWindow(0);
	GetDlgItem(IDC_BTN_BREAK)->ShowWindow(SW_HIDE);

	m_dlgMessages.GetDlgItem(IDC_BTN_LOGGING)->EnableWindow(0);
	m_dlgMessages.GetDlgItem(IDC_BTN_LOGGING)->ShowWindow(SW_HIDE);

#endif


	// Load in the database from disk
	if (m_data.LoadFile() == false)
	{
		AfxMessageBox(L"Can't open data file", MB_ICONERROR);
		PostQuitMessage(1);
		return FALSE;
	}

	// Populate the tabed dialogs' list controls
	UpdateShowList();
	UpdateScheduleList();
	UpdateArchiveList();

	// Put # Shows in the Show tab text
	UpdateTabTotals();

	// Start the minute timer to update the schedule list
	TIMER_ID_ONE_MINUTE = SetTimer(TIMER_ID_ONE_MINUTE, 60*1000, NULL);
	ASSERT(TIMER_ID_ONE_MINUTE);

	// Make the schedule tab the default
	m_tabctrl.SetCurFocus(TAB_NUM_SCHEDULE);
	m_dlgSchedule.ShowWindow(SW_SHOW);
	m_dlgShows.ShowWindow(SW_HIDE);
	m_dlgArchive.ShowWindow(SW_HIDE);

	// Move the main window to the center of the screen
	CenterWindow();

	// Set button enable/disable states to something sensible
	PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_APP_STARTED));

	// If we start with a new/empty database, all we can initally do is add a new show
	if (m_data.IsNewDataFile())
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_FILE_CREATED));

	m_dlm.SetMsgWin(m_hWnd);


	// return TRUE  unless you set the focus to a control
	return FALSE;
}




/**
 * Stop the <RETURN> key from closing the application
 */
void CepcheckDlg::OnOK()
{
	CONSOLE_PRINT(eLogFlags::INFO, L"CepcheckDlg::OnOK() discarded");
	// Don't call base class CDialog::OnOK();
}

/**
 * Stop the <CANCEL> key from closing the application if we're downloading
 */
void CepcheckDlg::OnCancel()
{
	if (m_dlm.DownloadInProgress())
	{
		AfxMessageBox(L"Download in progress. Abort it first.", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		return;
	}

	// Stop all the slotthreads & wait for them to exit
	m_dlm.TerminateSlotThreads();

	CDialog::OnCancel();
}

/**
 * As soon as the main dialog window is created, notity the model.
 */
int CepcheckDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_dlm.SetMsgWin(m_hWnd);

	return 0;
}




/**
 * Windows message handler for when a tab is clicked
 *
 */
void CepcheckDlg::OnTcnSelchangeTab1(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	int selectedTab = m_tabctrl.GetCurSel();

	// It's not enough to just SHOW the active tab, you must HIDE the inactive tabs too.
	m_dlgShows.ShowWindow(    (selectedTab == TAB_NUM_SHOWS)    ? SW_SHOW : SW_HIDE);
	m_dlgSchedule.ShowWindow( (selectedTab == TAB_NUM_SCHEDULE) ? SW_SHOW : SW_HIDE);
	m_dlgArchive.ShowWindow(  (selectedTab == TAB_NUM_ARCHIVE)  ? SW_SHOW : SW_HIDE);

	PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_TAB_CHANGED));

	// All done
	*pResult = 0;
}




/**
 * Load the database from disk
 *
 */
void CepcheckDlg::OnBtn_Load()
{
	m_data.LoadFile();

	// Reload the tabed dialogs
	UpdateShowList();
	UpdateScheduleList();
	UpdateArchiveList();

	PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_FILE_LOADED));

	AfxMessageBox(L"Shows loaded from disk OK", MB_ICONINFORMATION | MB_APPLMODAL | MB_OK);
}




/**
 * Save the database to disk
 *
 */
void CepcheckDlg::OnBtn_Save()
{
	if (!m_data.SaveFile())
		AfxMessageBox(L"Error saving data file!", MB_ICONERROR | MB_OK | MB_APPLMODAL );
	else
	{
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_FILE_SAVED));
		AfxMessageBox(L"Shows saved to disk OK", MB_ICONINFORMATION | MB_APPLMODAL | MB_OK);
	}
}




/**
 * 'Missed only' checkbox clicked in the top level dialog.
 * Notify the model of the new state & rebuild the schedule accordingly
 */
void CepcheckDlg::OnBtn_ChkMissedOnly()
{
	UpdateData();
	m_data.ShowMissedOnly(m_missed_only);

	UpdateScheduleList();
}




/**
 * Open up file explorer in the data file location
 *
 */
void CepcheckDlg::OnBtn_Explorer()
{
	m_data.OpenDataFileFolder();
}




/**
 * Break into the debugger
 *
 */
void CepcheckDlg::OnBtn_Break()
{
	AfxDebugBreak();
}




/**
 * Show/Hide the logging messages window
 *
 */
void CepcheckDlg::OnBtn_ShowLog()
{
	static BOOL visible = false;

	visible = !visible;

	auto chkbox = (CButton*) GetDlgItem(IDC_CHK_DEBUG_LOG);
	chkbox->SetCheck(visible);

	m_dlgMessages.ShowWindow((visible) ? SW_SHOW : SW_HIDE);
}




/**
 * Reset the +/- days interval for the schedule list to their defaults.
 *
 */
void CepcheckDlg::OnBtn_ResetDays()
{
	m_spin_pre_val  = DEFAULT_DAYS_PRE;
	m_spin_post_val = DEFAULT_DAYS_POST;

	m_spin_pre.SetPos(m_spin_pre_val);
	m_spin_post.SetPos(m_spin_post_val);

	UpdateSchedulePeriod();
	UpdateScheduleList();
}




/**
 * Delete a show & all its episodes. Then update the display.
 * A show can only be deleted from the archive dialog.
 * 
 */
void CepcheckDlg::OnBtn_DeleteShow()
{
	// Can only delete when the Archived list is displayed, not the Schedule or Show list.
	if (m_tabctrl.GetCurSel() != TAB_NUM_ARCHIVE)
	{
		LogMsgWin(L"Must be on 'Archive' tab to delete a show");
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	// This looks like a strange function call sequence, but it is right - call GetFirst then GetNext
	POSITION pos = m_dlgArchive.m_archivelist.GetFirstSelectedItemPosition();
	int nItem    = m_dlgArchive.m_archivelist.GetNextSelectedItem(pos);

	CString str;
	str.Format(L"Delete show '%s' ?", (LPCTSTR) m_dlgArchive.m_archivelist.GetItemText(nItem, 0));
	if (AfxMessageBox(str, MB_YESNO | MB_APPLMODAL) != IDYES)
		return;

	// Get the show hash from the list control & ask the model to delete it
	size_t hash = m_dlgArchive.m_archivelist.GetItemData(nItem);
	m_data.DeleteShow(hash);

	// Update the display
	UpdateArchiveList();
	PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_SHOW_DELETED));

	AfxMessageBox(L"Show deleted", MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
}




/**
 * Enter a URL for epguides.com & add a new show to the database
 *
 */
void CepcheckDlg::OnBtn_NewShow()
{
	// Popup the dialog box to enter the URL
	CDnewShow	dbox(this);
	INT_PTR retval = dbox.DoModal();
	if (retval == -1) {
		AfxMessageBox(L"Could not create dialog box!", MB_ICONERROR | MB_APPLMODAL | MB_OK);
		return;
	}
	if (retval == IDCANCEL)
		return;

	CString new_url = dbox.m_new_url.MakeLower().Trim();
	if (new_url.IsEmpty())
		return;

	// For consistant hash calculation strip leading www & ensure there's a trailing slash
	new_url.Replace(L"www.", L"");
	if (new_url.Right(1) != L"/")
		new_url += '/';

	// Validate the URL.
	std::wregex pattern(L"^https:\\/\\/(www\\.)?epguides\\.com\\/[^\\/]+([\\/]{1})$");
	std::wstring input(new_url);
	bool good_url = std::regex_match(input, pattern);

	if (!good_url)
	{
		AfxMessageBox(L"That is not a valid URL for a show on epguides.com", MB_ICONHAND | MB_APPLMODAL | MB_OK);
		return;
	}

	// Search both active & archive lists for the show
	if ( m_data.FindShow(new_url, eShowList::BOTH) != nullptr )
	{
		AfxMessageBox(L"That show is already in the database", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//
	// Get things ready to start the download
	//

	CW2A asciiz(new_url);
	m_new_show_hash = std::hash<std::string>()(asciiz.m_psz);

	m_ping_count    = m_err_count = 0;
	m_ping_expected = 1;
	UpdateOnscreenCounters();

	// Start downloading the new show's info
	m_dlm.DownloadShow(asciiz.m_psz);
	PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_STARTED));
}




/**
 * Clear then repopulate the Shows ClistCtrl from the database
 *
 */
void CepcheckDlg::UpdateShowList()
{
	sShowListEntry	sle;
	eGetAction		action{ eGetAction::GET_FIRST };

	// Get the dialog box to do a few things
	m_dlgShows.DeleteAllItems();

	while (m_data.GetShow(eShowList::ACTIVE, action, &sle))
	{
		// Add this show to the list
		ShowListStringsToLocal(&sle);
		m_dlgShows.AppendRow(&sle);

		action = eGetAction::GET_NEXT;
	}

	// Sort the list & redraw it
	m_dlgShows.SortList();

	// If a new show was added - make it visible & highlight it
	if (m_new_show_hash) {
		m_dlgShows.EnsureVisible(m_new_show_hash);
		m_new_show_hash = 0;
	}

	m_dlgShows.Invalidate();

	// Update # shows in Show tab text
	UpdateTabTotals();
}




/**
 * Clear then repopulate the Schedule list control from the database
 *
 */
void CepcheckDlg::UpdateScheduleList()
{
	sScheduleListEntry		sle;
	eGetAction				action{ eGetAction::GET_FIRST };

	// Clear the CtrlList
	m_dlgSchedule.DeleteAllItems();

	while (m_data.GetFilteredEpisode(action, &sle))
	{
		ScheduleListStringsToLocal(&sle);
		m_dlgSchedule.AppendRow(&sle);

		action = eGetAction::GET_NEXT;
	}

	m_dlgSchedule.SortList();
}




/**
 * Reload the 'Archived shows' list control from the database
 *
 */
void CepcheckDlg::UpdateArchiveList()
{
	sShowListEntry	sle;
	eGetAction		action { eGetAction::GET_FIRST };

	// Clear the CtrlList
	m_dlgArchive.DeleteAllItems();

	while (m_data.GetShow(eShowList::ARCHIVE, action, &sle))
	{
		ShowListStringsToLocal(&sle);
		m_dlgArchive.AppendRow(&sle);

		action = eGetAction::GET_NEXT;
	}

	m_dlgArchive.Invalidate();
}




/**
 * Update the text on the 'Shows' & 'Archive' tabs
 *
 */
void CepcheckDlg::UpdateTabTotals(void)
{
	// Put # Shows in the Show tab text
	CString str;
	str.Format(L"Shows (%d)", m_data.NumShows(eShowList::ACTIVE));

	TCITEM ltag;
	ltag.mask = TCIF_TEXT;
	ltag.pszText = str.LockBuffer();
	m_tabctrl.SetItem(TAB_NUM_SHOWS, &ltag);
	str.UnlockBuffer();

	str.Format(L"Archive (%d)", m_data.NumShows(eShowList::ARCHIVE));
	ltag.pszText = str.LockBuffer();
	m_tabctrl.SetItem(TAB_NUM_ARCHIVE, &ltag);
	str.UnlockBuffer();
}




/**
 * Update the UI ping & error counter boxes
 * 
 */
void CepcheckDlg::UpdateOnscreenCounters(void)
{
	CString		str;

	str.Format(L" %d", m_ping_count);
	SetDlgItemText(IDC_PING_COUNT, str);

	str.Format(L" %d", m_err_count);
	SetDlgItemText(IDC_ERR_COUNT, str);
}




/**
 * Check every minute if the day has changed. If so, update the schedule list.
 */
void CepcheckDlg::OnTimer(UINT_PTR nTimerID)
{
	static int day = 0;

	if (nTimerID == TIMER_ID_ONE_MINUTE)
	{
		// Only update the CListCtrl if the day has rolled over. GetDay() returns 1-31.
		int newday = CTime::GetCurrentTime().GetDay();
		if (newday != day)
		{
			day = newday;

			// Save (any) currently selected item & the list scroll position
			int nSelectedItem = m_dlgSchedule.GetSelectedItem();
			int nTopIndex = m_dlgSchedule.GetTopIndex();

			// Set the new date filter in the model & re-do the schedule list
			m_data.SetTodaysDate();
			UpdateScheduleList();

			// Restore any selection and scroll position
			if (nSelectedItem != -1)
				m_dlgSchedule.SetSelectedItem(nSelectedItem);
			m_dlgSchedule.SetTopIndex(nTopIndex);
		}
	}

	CDialog::OnTimer(nTimerID);
}




/**
 * Sends the +/- days count to the database and updates the on-screen values
 *
 */
void CepcheckDlg::UpdateSchedulePeriod(void)
{
	CString str;

	// Tell the model the date interval we're interested in
	m_data.SetDateInterval(m_spin_pre_val, m_spin_post_val);

	// Update the UI
	str.Format(L"- %2d", m_spin_pre_val);
	GetDlgItem(IDC_STATIC_DAYS_PRE)->SetWindowText(str);

	str.Format(L"+ %2d", m_spin_post_val);
	GetDlgItem(IDC_STATIC_DAYS_POST)->SetWindowText(str);
}




/**
 * A row on the Shows or Schedule tab was double-clicked. Popup a modal dialog listing all episodes for that Show.
 *
 */
afx_msg LRESULT CepcheckDlg::OnZoomEpisodes(WPARAM wParam, [[ maybe_unused ]] LPARAM lParam )
{
	DWORD hash = static_cast<DWORD>(wParam);
	const show* pshow = m_data.FindShow(hash, eShowList::BOTH);

	if (pshow == nullptr) {
		LogMsgWin(L"OnZoomEpisodes() show not found");
		AfxMessageBox(L"Can't find show!", MB_ICONERROR | MB_APPLMODAL | MB_OK);
	}
	else
	{
		// Popup the Zoom window
		CDShowZoom dlg(this, pshow);
		dlg.DoModal();
	}

	return TRUE;
}




/**
 * Open up a URL in the default web browser
 *
 * wParam = hash
 * lParam = popup menu selection ID
 *
 */
afx_msg LRESULT CepcheckDlg::OnLaunchUrl(WPARAM wParam, LPARAM lParam)
{
	size_t hash    = static_cast<size_t>(wParam);
	auto selection = static_cast<unsigned>(lParam);

	const show* ashow = m_data.FindShow(hash, eShowList::BOTH);
	if (ashow == nullptr)
		return 0;



	CString url(L"https://thetvdb.com/search?query=");
	switch(selection)
	{
		case ID_MNU_TVMAZE_GO:		url = ashow->tvmaze_url.c_str();		break;
		case ID_MNU_EPGUIDES_GO:	url = ashow->epguides_url.c_str();		break;
		case ID_MNU_IMDB_GO:		url = ashow->imdb_url.c_str();			break;
		case ID_MNU_THETVDB_GO:		url = ashow->thetvdb_url.c_str();		break;

		case ID_MNU_THETVDB_SEARCH:
			url += ashow->title.c_str();
			url.Replace(' ', '+');
			break;

		default:
			break;
	}


	if (url.GetLength() > 0)
	{
		HINSTANCE h = ::ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
		if (reinterpret_cast<INT_PTR>(h) <= 32) {
			CString msg;
			msg.Format(L"ShellExecute returned %08X", (INT_PTR) h);
			LogMsgWin(msg);
			AfxMessageBox(L"Can't open web browser!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		}
	}

	return 0;
}




/**
 * One of the child dialogs has asked for a context menu.
 * 
 */
afx_msg LRESULT CepcheckDlg::OnShowContextMenu(WPARAM wParam, [[ maybe_unused ]] LPARAM lParam )
{
	sPopupContext* pcontext = reinterpret_cast<sPopupContext*>(wParam);

	size_t hash  = pcontext->show_hash;
	CPoint point = pcontext->click_point;
	
	show* pshow  = m_data.FindShow(hash, eShowList::BOTH);
	if (pshow == nullptr)
	{
		AfxMessageBox(L"Show not found for context menu.", MB_ICONEXCLAMATION | MB_OK | MB_APPLMODAL );
		LogMsgWin(L"CepcheckDlg::OnShowContextMenu(): hash not found");
		return 0;
	}

	

	// The context popup is stored in a resource
	CMenu menu;
	menu.LoadMenu( IDR_MNU_URL );

	// All of the dialog context menus have these options for URLs enabled
	menu.EnableMenuItem(ID_MNU_EPGUIDES_GO, (pshow->epguides_url.length() > 0) ? MF_ENABLED : MF_GRAYED);
	menu.EnableMenuItem(ID_MNU_THETVDB_GO,  (pshow->thetvdb_url.length() > 0)  ? MF_ENABLED : MF_GRAYED);
	menu.EnableMenuItem(ID_MNU_TVMAZE_GO,   (pshow->tvmaze_url.length() > 0)   ? MF_ENABLED : MF_GRAYED);
	menu.EnableMenuItem(ID_MNU_IMDB_GO,     (pshow->imdb_url.length() > 0)     ? MF_ENABLED : MF_GRAYED);


	// Adjust the context menu for whichever dialog asked for it
	if (pcontext->dialog_id == IDD_SCHEDULE)
	{
		// Add episode highlight menu entries
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_GOT_IT,		L"&Got It");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_NOT_GOT_IT,	L"&Not Got It");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_CLEAR,		L"Clear Flags");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_REFRESH_SHOW, L"Refresh Show");
		menu.GetSubMenu(0)->AppendMenu(MF_SEPARATOR);
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_SHOW_TITLE,		L"Copy Show &Title");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_SHOW_TITLE_NUM,	L"&Copy Show Title + Number");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_EP_TITLE,		L"Copy &Episode Title");

		// Enable them appropriately
		menu.EnableMenuItem(ID_MNU_GOT_IT,     (pcontext->ep_flags & episodeflags::EP_FL_GOT)     ? MF_GRAYED : MF_ENABLED);
		menu.EnableMenuItem(ID_MNU_NOT_GOT_IT, (pcontext->ep_flags & episodeflags::EP_FL_NOT_GOT) ? MF_GRAYED : MF_ENABLED);
		menu.EnableMenuItem(ID_MNU_CLEAR, ((pcontext->ep_flags & episodeflags::EP_FL_NOT_GOT) | (pcontext->ep_flags & episodeflags::EP_FL_GOT)) ? MF_ENABLED : MF_GRAYED);
	}
	else if (pcontext->dialog_id == IDD_SHOWS)
	{
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_SHOW_TITLE, L"Copy Show Title");
		menu.GetSubMenu(0)->AppendMenu(MF_SEPARATOR);
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_ARCHIVE, L"Archive");
		menu.EnableMenuItem(ID_MNU_ARCHIVE, (pshow->flags & showflags::SH_FL_ARCHIVED) ? MF_GRAYED : MF_ENABLED);
	}
	else if (pcontext->dialog_id == IDD_ARCHIVE)
	{
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_UNARCHIVE, L"Unarchive");
		menu.EnableMenuItem(ID_MNU_UNARCHIVE, (pshow->flags & showflags::SH_FL_ARCHIVED) ? MF_ENABLED : MF_GRAYED);
	}
	else
	{
		LogMsgWin(L"Unrecognised context for right mouse button.");
	}



	//
	// Activate the context menu as a popup
	//
	int selection = menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, this);
	if (selection == 0)
		return 0;

	bool url_edited       = false;
	bool ep_flags_changed = false;

	switch (selection)
	{
		case ID_MNU_TVMAZE_GO:
		case ID_MNU_EPGUIDES_GO:
		case ID_MNU_IMDB_GO:
		case ID_MNU_THETVDB_GO:
		case ID_MNU_THETVDB_SEARCH:
			PostMessageW(WM_TVP_LAUNCH_URL, hash, selection);
			break;

		case ID_MNU_EPGUIDES_EDIT:
			url_edited = EditUrl(L"epguides", pshow->epguides_url);
			break;
		case ID_MNU_TVMAZE_EDIT:
			url_edited = EditUrl(L"TVMaze",   pshow->tvmaze_url);
			break;
		case ID_MNU_IMDB_EDIT:
			url_edited = EditUrl(L"IMDB",     pshow->imdb_url);
			break;
		case ID_MNU_THETVDB_EDIT:
			url_edited = EditUrl(L"TheTVDB",  pshow->thetvdb_url);
			break;

		case ID_MNU_ARCHIVE:
			// Tell the model to archive the show
			if (m_data.ArchiveShow(hash) == true)
			{
				UpdateShowList();
				UpdateScheduleList();
				UpdateArchiveList();
				PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_ARCHIVE_CHANGED));
			}
			else
				AfxMessageBox(L"Error. Active show not found! Reload database?", MB_ICONERROR | MB_OK | MB_APPLMODAL);
			break;
		
		case ID_MNU_UNARCHIVE:
			if (m_data.UnarchiveShow(hash) == true)
			{
				UpdateShowList();
				UpdateScheduleList();
				UpdateArchiveList();
				PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_ARCHIVE_CHANGED));
			}
			else
				AfxMessageBox(L"Error. Archive show not found! Reload database?", MB_ICONERROR | MB_OK | MB_APPLMODAL);
			break;


		// These next four cases are for the Schedule dialog only
		case ID_MNU_CLEAR:
			pcontext->ep_flags = episodeflags::EP_FL_NONE;
			ep_flags_changed = m_data.EpisodeFlagsChange(pcontext);
			break;
		case ID_MNU_GOT_IT:
			pcontext->ep_flags &= ~episodeflags::EP_FL_NOT_GOT;
			pcontext->ep_flags |= episodeflags::EP_FL_GOT;
			ep_flags_changed = m_data.EpisodeFlagsChange(pcontext);
			break;
		case ID_MNU_NOT_GOT_IT:
			pcontext->ep_flags &= ~episodeflags::EP_FL_GOT;
			pcontext->ep_flags |= episodeflags::EP_FL_NOT_GOT;
			ep_flags_changed = m_data.EpisodeFlagsChange(pcontext);
			break;

		case ID_MNU_REFRESH_SHOW:
			RefreshShow(hash);
			break;

		// Copy the show title or episode title from the Schedule List to the system clipboard
		case ID_MNU_COPY_SHOW_TITLE:
			// Already have pshow, no need to query the dialog directly like for ID_MNU_COPY_EPISODE_TITLE
			CopyToClipboard(pshow->title);
			break;
		case ID_MNU_COPY_EP_TITLE:
			CopyToClipboard(m_dlgSchedule.GetEpisodeTitle(pcontext->list_index));
			break;
		case ID_MNU_COPY_SHOW_TITLE_NUM:
			{
			CString show  = m_dlgSchedule.GetEpisodeShow(pcontext->list_index);
			CString epnum = m_dlgSchedule.GetEpisodeNumber(pcontext->list_index);

			int iPos = 0;
			CString series  = (CString("00") + epnum.Tokenize(L"-", iPos)).Right(2);
			CString episode = (CString("00") + epnum.Tokenize(L"-", iPos)).Right(2);

			CString str;
			str.Format(L"%s s%se%s", (LPCTSTR) show, (LPCTSTR) series, (LPCTSTR)episode);
			CopyToClipboard(str);
			}
			break;

		default:
			LogMsgWin(L"OnShowContextMenu(): Unhandled menu selection");
			return 1;
			break;
	}


	if (url_edited == true)
	{
		// Enable the Save/Load buttons if need be
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_URL_EDITED));
	}

	if (ep_flags_changed == true)
	{
		// Get the schedule dialog to update its list control entry
		m_dlgSchedule.PostMessage(WM_TVP_SCHED_EP_FLAGS_CHANGED, reinterpret_cast<WPARAM>(pcontext));
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_EP_FLAGS_CHANGED));
	}

	return 0;
}




/**
 * Handle a WM_SIGNAL_APP_EVENT message
 * 
 */
afx_msg LRESULT CepcheckDlg::OnSignalAppEvent(WPARAM wParam, [[ maybe_unused ]] LPARAM lParam )
{
	eAppevent event = static_cast<eAppevent>(wParam);

#if (TRACE_APP_EVENTS==1) && defined(_DEBUG)
	LogMsgWin(L"eAppevent %u", event);
#endif

	CONSOLE_PRINT(eLogFlags::APP_EVENT, L"eAppevent %u\n", event);

	// Have a few useful values handy
	unsigned numActiveShows  = m_data.NumShows(eShowList::ACTIVE);
	unsigned numArchiveShows = m_data.NumShows(eShowList::ARCHIVE);
	int selectedTab = m_tabctrl.GetCurSel();

	switch (event)
	{
		case eAppevent::AE_APP_STARTED:
			m_btn_load.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_save.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		case eAppevent::AE_DOWNLOAD_STARTED:
			m_dlgMessages.PostMessage(WM_TVP_ABORT_BTN_ENABLE);
			//
			m_btn_load.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_save.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_download.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_new_show.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_delete_show.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		case eAppevent::AE_DOWNLOAD_OK:
			m_dlgMessages.PostMessage(WM_TVP_ABORT_BTN_DISABLE);
			//
			m_btn_load.EnableWindow();
			m_btn_save.EnableWindow();
			m_btn_download.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			// Also set tab appropriate buttons
			PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_TAB_CHANGED));
			break;

		case eAppevent::AE_DOWNLOAD_ABORTED:
			m_dlgMessages.PostMessage(WM_TVP_ABORT_BTN_DISABLE);
			//
			m_btn_load.EnableWindow();
			m_btn_save.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_download.EnableWindow();
			break;

		case eAppevent::AE_DOWNLOAD_FAILED:
			m_dlgMessages.PostMessage(WM_TVP_ABORT_BTN_DISABLE);
			//
			m_btn_load.EnableWindow();
			m_btn_save.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_download.EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			break;

		case eAppevent::AE_FILE_LOADED:
			m_btn_load.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_save.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_download.EnableWindow();
			break;

		case eAppevent::AE_FILE_SAVED:
			m_btn_save.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		case eAppevent::AE_FILE_CREATED:
			m_btn_load.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_save.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_download.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			m_btn_new_show.EnableWindow();
			m_btn_delete_show.EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		// Can only happen on Schedule dialog
		case eAppevent::AE_EP_FLAGS_CHANGED:
			m_btn_load.EnableWindow();
			m_btn_save.EnableWindow();
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			break;

		case eAppevent::AE_URL_EDITED:
		case eAppevent::AE_SHOW_ADDED:
		case eAppevent::AE_SHOW_REFRESHED:
		case eAppevent::AE_SHOW_DELETED:
			m_btn_load.EnableWindow();
			m_btn_save.EnableWindow();
			m_btn_download.EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_TAB_CHANGED));
			break;

		case eAppevent::AE_ARCHIVE_CHANGED:
			m_btn_load.EnableWindow();
			m_btn_save.EnableWindow();
			break;

		case eAppevent::AE_TAB_CHANGED:
			if (selectedTab == TAB_NUM_SHOWS) {
				m_btn_new_show.EnableWindow();
				m_btn_delete_show.EnableWindow(0);
				m_btn_download.EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			}
			else if (selectedTab == TAB_NUM_SCHEDULE) {
				m_btn_new_show.EnableWindow(0);
				m_btn_delete_show.EnableWindow(0);
				m_btn_download.EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			}
			else if (selectedTab == TAB_NUM_ARCHIVE)
			{
				m_btn_new_show.EnableWindow(0);
				m_btn_delete_show.EnableWindow();
				m_btn_download.EnableWindow((numArchiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			}
			break;

		default:
			LogMsgWin(L"Unhandled eAppevent: %u", event);
			break;
	}

	return 0;
}




/**
 * A Spin control is about to change. Return non-zero to prevent the change.
 *
 */
void CepcheckDlg::OnDeltaPosSpinDays(NMHDR* pNMHDR, LRESULT* pResult)
{
	const LPNMUPDOWN	pNMUpDown = reinterpret_cast<const LPNMUPDOWN>(pNMHDR);
	UINT				ctrlId = pNMUpDown->hdr.idFrom;
	LRESULT				retval = 1;

	int new_val = pNMUpDown->iPos + pNMUpDown->iDelta;

	// Don't go less than min allowed
	if (new_val >= MIN_SPIN_DAYS)
	{
		if (ctrlId == IDC_SPIN_DAYS_PRE)
			m_spin_pre_val  = new_val;
		else
			m_spin_post_val = new_val;


		retval = 0;
		UpdateSchedulePeriod();
		UpdateScheduleList();
	}

	*pResult = retval;
}




/**
 * Sent by the CDMessages dialog when the 'Cancel Download' button is pressed
 */
void CepcheckDlg::OnBtn_AbortDownload()
{
	LogMsgWin(L"Aborting Download!");

	// Gets reset by the WM_DOWNLOAD_COMPLETE handler
	m_abort_download = true;
	m_dlm.AbortDownload();

	// End of the 'abort download' is checked for in the ping handler
}




/**
 * Download from the internet. Update all shows / episodes.
 *
 * Return:  true    Download started
 *          false   Error, or nothing to download
 *
 */
void CepcheckDlg::OnBtn_Download()
{
	// No shows to download?
	if (m_data.NumActiveShows() == 0) {
		AfxMessageBox(L"No shows in database!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		return;
	}

	// Already downloading?
	if (m_dlm.DownloadInProgress()) {
		AfxMessageBox(L"Download already in progress!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		return;
	}

	if (IDYES != AfxMessageBox(L"Confirm download all shows from epguides.com?", MB_YESNO | MB_APPLMODAL))
		return;

	// Reset counters
	m_ping_count    = m_err_count = 0;
	m_ping_expected = m_data.NumActiveShows();

	PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_STARTED));

	sShowListEntry  sle;
	eGetAction action = eGetAction::GET_FIRST;
	while (m_data.GetShow(eShowList::ACTIVE, action, &sle))
	{
		m_dlm.DownloadShow(sle.epguides_url);
		action = eGetAction::GET_NEXT;
	}

	LogMsgWin(L"Download started");
}




/**
 * Download a show's information from the URL and create a new database entry.
 *
 */
bool CepcheckDlg::RefreshShow(size_t hash)
{
	// Already downloading?
	if (m_dlm.DownloadInProgress()) {
		AfxMessageBox(L"Download already in progress!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		return false;
	}


	show* pShow = m_data.FindShow(hash, eShowList::ACTIVE);
	if (pShow == nullptr) {
		AfxMessageBox(L"RefreshShow() : Can't find show", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		return false;
	}

	pShow->state |= showstate::SH_ST_WAITING;

	// Setup the ping counters
	m_ping_expected = 1;
	m_ping_count    = 0;

	m_dlm.DownloadShow(pShow->epguides_url);
	LogMsgWin("Refreshing show '%s'", pShow->title.c_str());	
	return true;
}




/**
* If all shows are downloaded *OR* the download has been sucessfully
* aborted, send WM_DOWNLOAD_COMPLETE to the main dialog window.
*
*/
void CepcheckDlg::CheckDownloadComplete()
{
	if (m_abort_download)
	{
		// More slots still active? Wait for them to complete & ping
		if (m_dlm.DownloadInProgress())
			return;
		else {
			PostMessage(WM_TVP_DOWNLOAD_COMPLETE);
		}
	}

	// We're not mid-abort. Just check the ping count
	if (m_ping_expected == m_ping_count)
	{
		PostMessage(WM_TVP_DOWNLOAD_COMPLETE);
	}
}




/**
 * Handler for the WM_TVP_DOWNLOAD_PING message sent after a shows info has been downloaded.
 * 
 * One slot has completed it's download. Update the show information in the model.
 * Existing episode flags must be copied over as that's not in the reveived data
 *
 */
afx_msg LRESULT CepcheckDlg::OnDownloadPing(WPARAM slotnum, [[ maybe_unused ]] LPARAM lParam)
{
	// Bump ping counter and update the UI
	m_ping_count++;
	UpdateOnscreenCounters();

	const show& resultShow = m_dlm.GetSlotShow(slotnum);
	// If this is for a new show, there will be no database entry for it.
	show* originalShow = m_data.FindShow(resultShow.hash, eShowList::ACTIVE);

	
	auto slotstate = m_dlm.GetSlotState(slotnum);
	if (slotstate != eSlotState::SS_RESULTS_READY)
	{
		if (originalShow)
			originalShow->state |= (showstate::SH_ST_UPDATE_FAILED | resultShow.state);

		LogMsgWin(L"ERROR! Download ping on slot %u, results not ready : %u", slotnum, slotstate);
	}
	else
	{
		m_dlm.SetSlotState(slotnum, eSlotState::SS_RESULTS_PROCESSING);

		if ((originalShow) && (m_new_show_hash==0))
			m_data.UpdateShow(resultShow);
		else if (!originalShow && (m_new_show_hash != 0))
			m_data.AddNewShow(resultShow);
		else
			LogMsgWin("OnDownloadPing() : Unexpected showstate");

		m_dlm.SetSlotState(slotnum, eSlotState::SS_RESULTS_PROCESSED);
	}

	// Reset the slot and mark it available
	m_dlm.ReleaseSlot(slotnum);

	// Time to send WM_DOWNLOAD_COMPLETE ? Also checks for the 'download abort' flag.
	CheckDownloadComplete();

	return 0;
}




/**
 * Handler for WM_DOWNLOAD_COMPLETE message
 *
 */
afx_msg LRESULT CepcheckDlg::OnDownloadComplete([[maybe_unused]] WPARAM slotnum,
												[[maybe_unused]] LPARAM lParam)
{
	LogMsgWin(L"Download complete");

	if (m_abort_download)
	{
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_ABORTED));
		AfxMessageBox(L"Download aborted.", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		m_dlm.ClearAbortCondition();
		m_abort_download = false;
	}
	else if (m_err_count > 0)
	{
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_FAILED));
		AfxMessageBox(L"DOWNLOAD ERRORS FOUND!", MB_ICONERROR | MB_APPLMODAL | MB_OK);
		m_err_count = 0;
	}
	else
	{
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_OK));
		AfxMessageBox(L"Download complete.", MB_ICONINFORMATION | MB_APPLMODAL | MB_OK);
	}

	// Update the model & the UI
	m_data.BuildEpisodeList();
	UpdateShowList();
	UpdateScheduleList();

	return 0;
}




/**
 * The download thread encountered a wait error while waiting for a request event.
 * We can requeue or cancel that slot's download.
 * 
 */
afx_msg LRESULT CepcheckDlg::OnThreadWaitFail(WPARAM slotnum, LPARAM wait_result)
{
	LogMsgWin("Wait failed slot %u, error %08X. Requeuing \"%s\".", slotnum, wait_result, m_dlm.GetSlotShow(slotnum).epguides_url.c_str());

	// No need to adjust ping/error counters if we retry it.

	auto& url = m_dlm.GetSlotShow(slotnum).epguides_url;
	m_dlm.ReleaseSlot(slotnum);
	m_dlm.DownloadShow(url);

	return 0;
}
