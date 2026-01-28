#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <regex>

#include "strsafe.h"
#include "pathcch.h"

#include "Resource.h"
#include "CDnewShow.hpp"
#include "CDShowZoom.hpp"
#include "CDInputBox.hpp"
#include "CdownloadManager.hpp"
#include "utils.hpp"
#include "debugConsole.h"
#include "logging.hpp"

#include "CepcheckDlg.hpp"





#define		MIN_SPIN_DAYS			1
#define		MAX_SPIN_DAYS			30


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
	DDX_Control(pDX, IDC_SPIN_DAYS_POST, m_spin_post);
	DDX_Control(pDX, IDC_SPIN_DAYS_PRE,  m_spin_pre);
	DDX_Check(pDX, IDC_CHK_MISSED_ONLY,  m_missed_only);
}




// ON_NOTIFY macro can give a false warning
#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CepcheckDlg, CDialog)
	ON_NOTIFY( TCN_SELCHANGE, IDC_TAB1,				&CepcheckDlg::OnTcnSelchangeTab1)
	ON_NOTIFY( UDN_DELTAPOS,  IDC_SPIN_DAYS_PRE,	&CepcheckDlg::OnDeltaPosSpinDays)
	ON_NOTIFY( UDN_DELTAPOS,  IDC_SPIN_DAYS_POST,	&CepcheckDlg::OnDeltaPosSpinDays)
	ON_BN_CLICKED( IDC_BTN_RESET_DAYS,				&CepcheckDlg::OnBtnClickedResetDays)
	ON_BN_CLICKED( IDC_BTN_LOAD,					&CepcheckDlg::OnBtnClickedLoad)
	ON_BN_CLICKED( IDC_BTN_SAVE,					&CepcheckDlg::OnBtnClickedSave)
	ON_BN_CLICKED( IDC_BTN_DOWNLOAD,				&CepcheckDlg::OnBtnClickedDownload)
	ON_BN_CLICKED( IDC_BTN_DELETE_SHOW,				&CepcheckDlg::OnBtnClickedDeleteShow)
	ON_BN_CLICKED( IDC_BTN_NEW_SHOW,				&CepcheckDlg::OnBtnClickedNewShow)
	ON_BN_CLICKED( IDC_BTN_BREAK,					&CepcheckDlg::OnBtnClickedBreak)
	ON_BN_CLICKED( IDC_BTN_EXPLORER,				&CepcheckDlg::OnBtnClickedExplorer)
	ON_BN_CLICKED(IDC_CHK_MISSED_ONLY,              &CepcheckDlg::OnBtnClickedChkMissedOnly)
	ON_BN_CLICKED(IDC_CHK_DEBUG_LOG,                &CepcheckDlg::OnBtnClickedChkDebugLog)
	ON_MESSAGE( WM_TVP_DOWNLOAD_COMPLETE,			&CepcheckDlg::OnDownloadComplete)
	ON_MESSAGE( WM_TVP_DOWNLOAD_PING,				&CepcheckDlg::OnDownloadPing)
	ON_MESSAGE( WM_TVP_SLOT_RELEASED,				&CepcheckDlg::OnSlotReleased)
	ON_MESSAGE( WM_TVP_ZOOM_EPISODES,				&CepcheckDlg::OnZoomEpisodes)
	ON_MESSAGE( WM_TVP_LAUNCH_URL,					&CepcheckDlg::OnLaunchUrl)
	ON_MESSAGE( WM_TVP_SHOW_CONTEXT_MENU,			&CepcheckDlg::OnShowContextMenu)
	ON_MESSAGE( WM_TVP_SIGNAL_APP_EVENT,			&CepcheckDlg::OnSignalAppEvent)
	ON_MESSAGE( WM_TVP_ABORT_DOWNLOAD,				&CepcheckDlg::OnAbortDownload)
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
	m_tabctrl.InsertItem(TAB_NUM_SHOWS, L"Shows");
	m_tabctrl.InsertItem(TAB_NUM_SCHEDULE, L"Schedule");
	m_tabctrl.InsertItem(TAB_NUM_ARCHIVE, L"Archive");

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
	SetMsgWindow(&m_dlgMessages.m_messages);


	// If this is a debug build, show the 'Break' UI button
#ifdef _DEBUG
	GetDlgItem(IDC_BTN_BREAK)->EnableWindow();
	GetDlgItem(IDC_BTN_BREAK)->ShowWindow(SW_SHOW);
#endif


	// Load the database from disk
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

	LOG_WRITE(eLogFlags::INFO, L"LOG: Init done");

	// return TRUE  unless you set the focus to a control
	return FALSE;
}




/**
 * Windows message handler for when a tab is clicked
 *
 */
void CepcheckDlg::OnTcnSelchangeTab1(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	int selectedTab = m_tabctrl.GetCurSel();

	m_dlgShows.ShowWindow(   (selectedTab == TAB_NUM_SHOWS)    ? SW_SHOW : SW_HIDE);
	m_dlgSchedule.ShowWindow((selectedTab == TAB_NUM_SCHEDULE) ? SW_SHOW : SW_HIDE);
	m_dlgArchive.ShowWindow( (selectedTab == TAB_NUM_ARCHIVE)  ? SW_SHOW : SW_HIDE);

	PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_TAB_CHANGED));

	// All done
	*pResult = 0;
}




/**
 * Load the database from disk
 *
 */
void CepcheckDlg::OnBtnClickedLoad()
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
void CepcheckDlg::OnBtnClickedSave()
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
 * Download/Update all episodes for all shows in the model
 *
 */
void CepcheckDlg::OnBtnClickedDownload()
{
	if (IDYES != AfxMessageBox(L"Confirm download all shows from epguides.com?", MB_YESNO | MB_APPLMODAL))
		return;

	// Reset & update on-screen counters
	m_ping_count = m_err_count = 0;
	UpdateOnscreenCounters();

	// Tell the data model to start the download everything & update everything
	if (m_data.DownloadAllShows())
	{
		// Set appropriate button states during download
		LogMsgWindow(L"Downloading all shows...");
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_STARTED));

		// Enable the 'Cancel Download' button in the massage box
		m_dlgMessages.GetDlgItem(IDC_BTN_ABORT_DOWNLOAD)->EnableWindow(TRUE);
	}
}




/**
 * Delete a show & all its episodes. Then update the display.
 * A show can only be deleted from the archive dialog.
 * 
 */
void CepcheckDlg::OnBtnClickedDeleteShow()
{
	// Can only delete when the Archived list is displayed, not the Schedule or Show list.
	if (m_tabctrl.GetCurSel() != TAB_NUM_ARCHIVE)
	{
		LogMsgWindow(L"Must be on 'Archive' tab to delete a show");
		MessageBeep(UINT_MAX);
		return;
	}

	// Can only delete one show at a time - multiple selections not allowed
	if (m_dlgArchive.m_archivelist.GetSelectedCount() != 1)
	{
		MessageBeep(UINT_MAX);
		AfxMessageBox(L"Can only delete one show at a time", MB_ICONERROR | MB_APPLMODAL | MB_OK);
		return;
	}

	// This looks like a strange function call sequence, but it is right - call GetFirst then GetNext
	POSITION pos = m_dlgArchive.m_archivelist.GetFirstSelectedItemPosition();
	int nItem = m_dlgArchive.m_archivelist.GetNextSelectedItem(pos);

	CString str;
	str.Format(L"Delete show '%s' ?", (LPCTSTR) m_dlgArchive.m_archivelist.GetItemText(nItem, 0));
	if (AfxMessageBox(str, MB_YESNO | MB_APPLMODAL) != IDYES)
		return;

	// Get the show hash from the list control & ask the model to delete it
	DWORD hash = m_dlgArchive.m_archivelist.GetItemData(nItem);
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
void CepcheckDlg::OnBtnClickedNewShow()
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

	// For consistant hash calculation strip leading www & ensure there;s a trailing slash
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
	if ( m_data.FindShow(new_url, eSHOWLIST::BOTH) != nullptr )
	{
		AfxMessageBox(L"That show is already in the database", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// Get things ready to start the download
	m_dlgShows.SaveTopIndex();
	m_ping_count = m_err_count = 0;
	UpdateOnscreenCounters();

	// Add it to the model
	DWORD hash = m_data.AddNewShow(new_url);


	// Start downloading the new show's info
	if (m_data.DownloadSingleShow(hash) == false)
	{
		AfxMessageBox(L"Add new show failed", MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_FAILED));
	}
	else
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_STARTED));

	// If there was no error, the model has successfully started a download thread for the new show
}




/**
 * Clear then repopulate the Shows ClistCtrl from the database
 *
 */
void CepcheckDlg::UpdateShowList()
{
	sShowListEntry	sle;
	eGETACTION		action{ eGETACTION::GET_FIRST };

	// Get the dialog box to do a few things
	m_dlgShows.SaveTopIndex();
	m_dlgShows.DeleteAllItems();

	while (m_data.GetShow(eSHOWLIST::ACTIVE, action, &sle))
	{
		// Add this show to the list
		ShowListStringsToLocal(&sle);
		m_dlgShows.AppendRow(&sle);

		action = eGETACTION::GET_NEXT;
	}

	// Sort the list & redraw it
	m_dlgShows.SortList();
	m_dlgShows.RestoreTopIndex();
	m_dlgShows.Invalidate();

	// Update #shows in Show tab text
	UpdateTabTotals();
}




/**
 * Clear then repopulate the Schedule list control from the database
 *
 */
void CepcheckDlg::UpdateScheduleList()
{
	sScheduleListEntry		sle;
	eGETACTION				action{ eGETACTION::GET_FIRST };

	// Clear the CtrlList
	m_dlgSchedule.DeleteAllItems();

	while (m_data.GetFilteredEpisode(action, &sle))
	{
		ScheduleListStringsToLocal(&sle);
		m_dlgSchedule.AppendRow(&sle);

		action = eGETACTION::GET_NEXT;
	}

	m_dlgSchedule.SortList();
}




void CepcheckDlg::UpdateArchiveList()
{
	sShowListEntry	sle;
	eGETACTION		action { eGETACTION::GET_FIRST };

	// Clear the CtrlList
	m_dlgArchive.DeleteAllItems();

	while (m_data.GetShow(eSHOWLIST::ARCHIVE, action, &sle))
	{
		ShowListStringsToLocal(&sle);
		m_dlgArchive.AppendRow(&sle);

		action = eGETACTION::GET_NEXT;
	}

	m_dlgArchive.Invalidate();
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
 * Update the UI ping & error counter boxes
 *
 */
void CepcheckDlg::ResetOnscreenCounters(void)
{
	m_ping_count = m_err_count = 0;
	UpdateOnscreenCounters();
}




/**
 * Handler for WM_DOWNLOAD_COMPLETE message sent by the dispatch thread when all shows have completed downloading & been processed
 *
 */
afx_msg LRESULT CepcheckDlg::OnDownloadComplete(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );

	// Tell the model to check all received show data
	bool bDownloadErrors = m_data.DownloadComplete();

	if (bDownloadErrors || (m_err_count > 0))
	{
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_FAILED));
		AfxMessageBox(L"DOWNLOAD ERRORS FOUND!", MB_ICONERROR | MB_APPLMODAL | MB_OK);
		bDownloadErrors = false;
		m_err_count = 0;
	}
	else if (m_abort_download)
	{
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_ABORTED));
		AfxMessageBox(L"Download aborted", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		m_abort_download = false;
	}
	else
	{
		PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_DOWNLOAD_OK));
		AfxMessageBox(L"Download complete", MB_ICONINFORMATION | MB_APPLMODAL | MB_OK);
	}


	// Update the UI
	UpdateShowList();
	UpdateScheduleList();

	return 0;
}




/**
 * Handler for the WM_TVP_DOWNLOAD_PING message sent after a shows info has been downloaded.
 * This local function just updates the UI counters. Call into the model to actually process the response.
 *
 */
afx_msg LRESULT CepcheckDlg::OnDownloadPing(WPARAM slotnum, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// Bump ping counter and update the UI
	m_ping_count++;
	UpdateOnscreenCounters();

	// Notify the model
	m_data.OnDownloadPing(slotnum);

	return 0;
}




/**
 * Handler for the WM_TVP_SLOW_RELEASED message, sent from the thrRelase thread after the model has finished with the slot.
 * This local function just calls into the model.
 *
 */
afx_msg LRESULT CepcheckDlg::OnSlotReleased(WPARAM slotnum, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// Notify the model
	m_data.OnSlotReleased(slotnum);

	return 0;
}








/**
 * A row on the Shows or Schedule tab was double-clicked. Popup a modal dialog listing all episodes for that Show.
 *
 */
afx_msg LRESULT CepcheckDlg::OnZoomEpisodes(WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER( lParam );

	DWORD hash = static_cast<DWORD>(wParam);
	const show* pshow = m_data.FindShow(hash, eSHOWLIST::BOTH);

	if (pshow == nullptr) {
		LogMsgWindow(L"OnZoomEpisodes() show not found");
		AfxMessageBox(L"Can't find show!", MB_ICONERROR | MB_APPLMODAL | MB_OK);
	}
	else
	{
		CDShowZoom dlg(this, pshow);
		dlg.DoModal();
	}

	return TRUE;
}




/**
 * Update the text on the 'Shows' & 'Archive tabs
 *
 */
void CepcheckDlg::UpdateTabTotals(void)
{
	// Put # Shows in the Show tab text
	CString str;
	str.Format(L"Shows (%d)", m_data.NumShows(eSHOWLIST::ACTIVE));

	TCITEM ltag;
	ltag.mask = TCIF_TEXT;
	ltag.pszText = str.LockBuffer();
	m_tabctrl.SetItem(TAB_NUM_SHOWS, &ltag);
	str.UnlockBuffer();

	str.Format(L"Archive (%d)", m_data.NumShows(eSHOWLIST::ARCHIVE));
	ltag.pszText = str.LockBuffer();
	m_tabctrl.SetItem(TAB_NUM_ARCHIVE, &ltag);
	str.UnlockBuffer();
}




/**
 * Display a URL in the default browser
 *
 * wParam = hash
 * lParam = popup menu selection ID
 *
 */
afx_msg LRESULT CepcheckDlg::OnLaunchUrl(WPARAM wParam, LPARAM lParam)
{
	DWORD hash = static_cast<DWORD>(wParam);
	unsigned selection = static_cast<unsigned>(lParam);

	const show* ashow = m_data.FindShow(hash, eSHOWLIST::BOTH);
	if (ashow == nullptr)
		return 0;

	CString url;

	if (selection == ID_MNU_TVMAZE_GO) {
		url = ashow->tvmaze_url.c_str();
	}
	else if (selection == ID_MNU_EPGUIDES_GO) {
		url = ashow->epguides_url.c_str();
	}
	else if (selection == ID_MNU_IMDB_GO) {
		url = ashow->imdb_url.c_str();
	}
	else if (selection == ID_MNU_THETVDB_GO) {
		url = ashow->thetvdb_url.c_str();
	}
	else if (selection == ID_MNU_THETVDB_SEARCH)
	{
		url = CString(L"https://thetvdb.com/search?query=") + CString(ashow->title.c_str());
		url.Replace(' ', '+');
	}

	if (url.GetLength() > 0)
	{
		HINSTANCE h = ::ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
		if ((INT_PTR)h <= 32) {
			CString msg;
			msg.Format(L"ShellExecute returned % d", (INT_PTR)h);
			LogMsgWindow(msg);
			AfxMessageBox(L"Can't open web browser!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		}
	}

	return 0;
}




/**
 * One of the child dialogs has asked for a context menu.
 * 
 */
afx_msg LRESULT CepcheckDlg::OnShowContextMenu(WPARAM wParam, LPARAM /* lParam */)
{
	sPopupContext* pcontext = reinterpret_cast<sPopupContext*>(wParam);

	DWORD  hash  = pcontext->show_hash;
	CPoint point = pcontext->click_point;
	
	show* pshow  = m_data.FindShow(hash, eSHOWLIST::BOTH);
	if (pshow == nullptr)
	{
		AfxMessageBox(L"Show not found for context menu.", MB_ICONEXCLAMATION | MB_OK | MB_APPLMODAL );
		LogMsgWindow(L"CepcheckDlg::OnShowContextMenu(): hash not found");
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
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_GOT_IT, L"&Got It");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_NOT_GOT_IT, L"&Not Got It");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_CLEAR, L"&Clear Flags");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_REFRESH_SHOW, L"Refresh Show");
		menu.GetSubMenu(0)->AppendMenu(MF_SEPARATOR);
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_SHOW_TITLE, L"Copy &Show Title");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_SHOW_TITLE_NUM, L"Copy Show Title + Number");
		menu.GetSubMenu(0)->AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_EP_TITLE, L"Copy Episode &Title");

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
		CStringW s{ L"Unrecognised context for right mouse button." };
		AfxMessageBox(s, MB_ICONEXCLAMATION | MB_OK | MB_APPLMODAL);
		LogMsgWindow(s);
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
			url_edited = EditUrl_Epguides(pshow);
			break;
		case ID_MNU_TVMAZE_EDIT:
			url_edited = EditUrl_TVmaze(pshow);
			break;
		case ID_MNU_IMDB_EDIT:
			url_edited = EditUrl_IMDB(pshow);
			break;
		case ID_MNU_THETVDB_EDIT:
			url_edited = EditUrl_TheTVDB(pshow);
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
			// Reset ping & download counter
			ResetOnscreenCounters();
			if (m_data.DownloadSingleShow(hash) == false)
				AfxMessageBox(L"Refresh show failed", MB_ICONERROR | MB_APPLMODAL | MB_OK);
			else
				LogMsgWindow(L"Refreshing show...");
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
			LogMsgWindow(L"OnShowContextMenu(): Unhandled menu selection");
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
 * 'Missed only' checkbox clicked in the top level dialog.
 * Notify the model of the new state & rebuild the schedule accordingly
 */
void CepcheckDlg::OnBtnClickedChkMissedOnly()
{
	UpdateData();
	m_data.ShowMissedOnly(m_missed_only);

	UpdateScheduleList();
}




/**
 * Show/Hide the logging messages window
 *
 */
void CepcheckDlg::OnBtnClickedChkDebugLog()
{
static BOOL visible = false;

	visible = !visible;

	CButton* chkbox = (CButton*) GetDlgItem(IDC_CHK_DEBUG_LOG);
	chkbox->SetCheck(visible);

	m_dlgMessages.ShowWindow((visible) ? SW_SHOW : SW_HIDE);

	LOG_WRITE(eLogFlags::INFO, L"LOG: BOING %u", visible);

}




/**
 * Reset the +/- days interval for the schedule list to their defaults.
 *
 */
void CepcheckDlg::OnBtnClickedResetDays()
{
	m_spin_pre_val  = DEFAULT_DAYS_PRE;
	m_spin_post_val = DEFAULT_DAYS_POST;

	m_spin_pre.SetPos(m_spin_pre_val);
	m_spin_post.SetPos(m_spin_post_val);

	UpdateSchedulePeriod();
	UpdateScheduleList();
}




/**
 * Handle a WM_SIGNAL_APP_EVENT message
 * 
 */
afx_msg LRESULT CepcheckDlg::OnSignalAppEvent(WPARAM wParam, LPARAM /* lParam */ )
{
	eAppevent event = static_cast<eAppevent>(wParam);

#if (TRACE_APP_EVENTS==1) && defined(_DEBUG)
	CString _msg;
	_msg.Format(L"eAppevent: %d", event);
	LogMsgWindow(_msg);
#endif

	// Have a few useful values handy
	unsigned numActiveShows  = m_data.NumShows(eSHOWLIST::ACTIVE);
	unsigned numArchiveShows = m_data.NumShows(eSHOWLIST::ARCHIVE);
	int selectedTab = m_tabctrl.GetCurSel();

	switch (event)
	{
		case eAppevent::AE_APP_STARTED:
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		case eAppevent::AE_DOWNLOAD_STARTED:
			m_dlgMessages.GetDlgItem(IDC_BTN_ABORT_DOWNLOAD)->EnableWindow(TRUE);

			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_NEW_SHOW)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_DELETE_SHOW)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		case eAppevent::AE_DOWNLOAD_OK:
			m_dlgMessages.GetDlgItem(IDC_BTN_ABORT_DOWNLOAD)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(1);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(1);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			// Also set tab appropriate buttons
			PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_TAB_CHANGED));
			break;

		case eAppevent::AE_DOWNLOAD_ABORTED:
			m_dlgMessages.GetDlgItem(IDC_BTN_ABORT_DOWNLOAD)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(1);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(0);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(1);
			break;

		case eAppevent::AE_DOWNLOAD_FAILED:
			m_dlgMessages.GetDlgItem(IDC_BTN_ABORT_DOWNLOAD)->EnableWindow(FALSE);

			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(1);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			break;

		case eAppevent::AE_FILE_LOADED:
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(1);
			break;

		case eAppevent::AE_FILE_SAVED:
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		case eAppevent::AE_FILE_CREATED:
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			GetDlgItem(IDC_BTN_NEW_SHOW)->EnableWindow(1);
			GetDlgItem(IDC_BTN_DELETE_SHOW)->EnableWindow(0 | KEEP_BUTTONS_ENABLED);
			break;

		// Can only happen on Schedule dialog
		case eAppevent::AE_EP_FLAGS_CHANGED:
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(1);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(1);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			break;

		// TODO - check if these are needed?
		case eAppevent::AE_URL_EDITED:
		case eAppevent::AE_SHOW_ADDED:
		case eAppevent::AE_SHOW_REFRESHED:
		case eAppevent::AE_SHOW_DELETED:
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(1);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(1);
			GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			PostMessage(WM_TVP_SIGNAL_APP_EVENT, static_cast<WPARAM>(eAppevent::AE_TAB_CHANGED));
			break;

		case eAppevent::AE_ARCHIVE_CHANGED:
			GetDlgItem(IDC_BTN_LOAD)->EnableWindow(1);
			GetDlgItem(IDC_BTN_SAVE)->EnableWindow(1);
			break;

		case eAppevent::AE_TAB_CHANGED:
			if (selectedTab == TAB_NUM_SHOWS) {
				GetDlgItem(IDC_BTN_NEW_SHOW)->EnableWindow(1);
				GetDlgItem(IDC_BTN_DELETE_SHOW)->EnableWindow(0);
				GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			}
			else if (selectedTab == TAB_NUM_SCHEDULE) {
				GetDlgItem(IDC_BTN_NEW_SHOW)->EnableWindow(0);
				GetDlgItem(IDC_BTN_DELETE_SHOW)->EnableWindow(0);
				GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow((numActiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			}
			else if (selectedTab == TAB_NUM_ARCHIVE)
			{
				GetDlgItem(IDC_BTN_NEW_SHOW)->EnableWindow(0);
				GetDlgItem(IDC_BTN_DELETE_SHOW)->EnableWindow(1);
				GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow((numArchiveShows > 0) ? 1 : (0 | KEEP_BUTTONS_ENABLED));
			}
			break;

		default:
			CString str;
			str.Format(L"Unhandled eAppevent: %d", event);
			LogMsgWindow(str);
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
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	UINT ctrlId = pNMUpDown->hdr.idFrom;
	LRESULT retval = 1;

	int new_val = pNMUpDown->iPos + pNMUpDown->iDelta;

	// Don't go less than min allowed
	if (new_val >= MIN_SPIN_DAYS)
	{
		if (ctrlId == IDC_SPIN_DAYS_PRE)
			m_spin_pre_val = new_val;
		else
			m_spin_post_val = new_val;

		retval = 0;
		UpdateSchedulePeriod();
		UpdateScheduleList();
	}

	*pResult = retval;
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
 * Open up file explorer in the data file location
 *
 */
void CepcheckDlg::OnBtnClickedExplorer()
{
	m_data.OpenDataFileFolder();
}




/**
 * Break into the debugger
 *
 */
void CepcheckDlg::OnBtnClickedBreak()
{
	AfxDebugBreak();
}

/**
 * Stop the <RETURN> key from closing the application
 */
void CepcheckDlg::OnOK()
{
	LogMsgWindow(L"CepcheckDlg::OnOK() discarded");
	//CDialog::OnOK();
}

/**
 * Stop the <CANCEL> key from closing the application if we're downloading
 */
void CepcheckDlg::OnCancel()
{
	if (m_data.DownloadInProgress())
	{
		AfxMessageBox(L"Download in progress!", MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		return;
	}

	/*
	 * This gives you a chance to copy msgs out of console or message log windows
	 */

#if (PAUSE_BEFORE_EXIT==1) && defined(_DEBUG)
	while (::MessageBox(NULL, L"About to close - press OK", APP_NAME, MB_OK) != IDOK);
#endif

	CDialog::OnCancel();
}

/**
 * As soon as the main dialog window is created, notity the model.
 */
int CepcheckDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_data.SetMsgWindow(m_hWnd);

	return 0;
}

/**
 * Sent by the CDMessages dialog when the 'Cancel Download' button is pressed
 */
afx_msg LRESULT CepcheckDlg::OnAbortDownload( WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	// Gets reset by the WM_DOWNLOAD_COMPLETE handler
	m_abort_download = true;

	m_data.AbortDownload();

	return 0;
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
			int nTopIndex     = m_dlgSchedule.GetTopIndex();

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

