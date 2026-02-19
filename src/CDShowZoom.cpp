#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;

#include "common.hpp"
#include "utils.hpp"

#include "CDShowZoom.hpp"





STATIC constexpr int COL_EPISODES_TITLE		= 0;
STATIC constexpr int COL_EPISODES_NUMBER	= 1;
STATIC constexpr int COL_EPISODES_DATE		= 2;
STATIC constexpr int COL_EPISODES_DATE_SORT	= 3;
//
STATIC constexpr int COL_EPISODES_NUM_COLS  = 4;




/**
 * Initialises the static const class member variable m_sort_map
 */
const tSortMap CDShowZoom::m_sort_map = {
	// 1st entry is used as the default sort order
	{ COL_EPISODES_DATE, {
			{COL_EPISODES_DATE_SORT,  NumberCompareFunc  },
			{COL_EPISODES_NUMBER,	  EpisodeCompareFunc }
		}},

	{ COL_EPISODES_TITLE, {
			{COL_EPISODES_TITLE,	  AlphaCompareFunc  },
			{COL_EPISODES_DATE_SORT,  NumberCompareFunc }
		}},

	{ COL_EPISODES_NUMBER, {
			{COL_EPISODES_NUMBER,	  EpisodeCompareFunc }
		}}
};





// CDShowZoom zoomed dialog

IMPLEMENT_DYNAMIC(CDShowZoom, CDialogEx)

CDShowZoom::CDShowZoom(CWnd* pParent, const show* pshow)
	: CDialogEx(IDD_SHOW_ZOOM, pParent),
	  cSortContext(&m_sort_map, &m_eplist),
	  m_pshow(pshow)
{
}

CDShowZoom::~CDShowZoom()
{
}

void CDShowZoom::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EPISODES_LIST, m_eplist);
}


// ON_NOTIFY macro gives an annoying compiler warning
#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CDShowZoom, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_EPISODES_LIST, &CDShowZoom::OnColumnClick)
END_MESSAGE_MAP()

#pragma warning( pop )




BOOL CDShowZoom::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_eplist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	// Make the column header bold
	SetListHeaderFont(&m_eplist);

	// Set the dialog box title = Name of the show
	sShowListEntry sle;
	CopyOutShowInfo(&sle, m_pshow);
	ShowListStringsToLocal(&sle);
	SetWindowText(sle.ui_title);

	/*
	 * Add four columns to the list box: "Title"  "Episode #"  "Airdate"  "Sort Daya" (hidden)
	 */
	LVCOLUMNW col = { 0 };
	col.fmt = LVCFMT_LEFT;
	col.mask = LVCF_FMT | LVCF_TEXT;
	col.cx = -1;

	// NB Same # columns must be catered for in the column click (ie sort) handler
	col.pszText = L"Title";
	m_eplist.InsertColumn(COL_EPISODES_TITLE, &col);
	col.pszText = L"Episode";
	m_eplist.InsertColumn(COL_EPISODES_NUMBER, &col);
	col.pszText = L"Air Date";
	m_eplist.InsertColumn(COL_EPISODES_DATE, &col);
	col.pszText = L"D-Date";
	m_eplist.InsertColumn(COL_EPISODES_DATE_SORT, &col);


	// Set the column widths
	CRect rc;
	m_eplist.GetClientRect(&rc);
	int listWidth = rc.Width();

	m_eplist.SetColumnWidth(COL_EPISODES_TITLE,   55 * listWidth / 100);
	m_eplist.SetColumnWidth(COL_EPISODES_NUMBER,  20 * listWidth / 100);
	m_eplist.SetColumnWidth(COL_EPISODES_DATE,    20 * listWidth / 100);
	// Zero width (ie hide) the date sort column
	m_eplist.SetColumnWidth(COL_EPISODES_DATE_SORT, 0);


	// Populate the list with this show's episodes
	for (const episode& ep : m_pshow->episodes)
	{
		// to_simple_string() gives YYYY-MMM-DD, re-arrange it to DD-MMM-YYYY
		CString ui_airdate_string(gregorian::to_simple_string(ep.ep_date).c_str());
		ui_airdate_string = ui_airdate_string.Right(2) + ui_airdate_string.Mid(4, 5) + ui_airdate_string.Left(4);

		CString ui_episode_number(ep.ep_num.c_str());
		CString ui_episode_title = CA2W(ep.ep_title.c_str(), CP_UTF8);

		std::string str = std::to_string(ep.ep_date.julian_day());
		CString ui_airdate_sort(str.c_str());

		// Number of rows already in list (0 based) = new row/item number.
		int count = m_eplist.GetItemCount();

		// Add a row of data
		int index = m_eplist.InsertItem(count, ui_episode_title);
		m_eplist.SetItemText(index, COL_EPISODES_NUMBER,    ui_episode_number);
		m_eplist.SetItemText(index, COL_EPISODES_DATE,      ui_airdate_string);
		m_eplist.SetItemText(index, COL_EPISODES_DATE_SORT, ui_airdate_sort);
	}


	// Set column headers to bold font
	LOGFONT logfont;
	m_eplist.GetHeaderCtrl()->GetFont()->GetLogFont(&logfont);
	logfont.lfWeight = FW_BOLD;
	m_headerFont.CreateFontIndirect(&logfont);
	m_eplist.GetHeaderCtrl()->SetFont(&m_headerFont, 1);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}



/**
 * Trap key presses in the zoomed show episode list.
 *   Apps/Context key pops up the context menu
 */
BOOL CDShowZoom::PreTranslateMessage(MSG* pMsg)
{

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->hwnd == m_eplist.GetSafeHwnd()))
	{
		int index = GetSelectedListItem(m_eplist);
		if (index != -1)
		{
			if (pMsg->wParam == VK_APPS)
			{
				CRect rect;
				m_eplist.GetItemRect(index, &rect, LVIR_LABEL);
				CPoint point(rect.CenterPoint());
				m_eplist.ClientToScreen(&point);

				OnContextMenu(nullptr, point);
				return TRUE;
			}
		}
	}

	// Message not handled in above traps. Pass it on to the default handler.
	return CDialog::PreTranslateMessage(pMsg);
}




void CDShowZoom::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// This msg is always 'handled'
	*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int new_sort_col = pNMLV->iSubItem;

	// Set the new sort column - if needed re-sort the list control
	if (SetSortColumn(new_sort_col) == true)
		SortList();
}




/**
 * Show a context menu for a show's zoomed popup episode list
 * The only option in the menu will be to copy the episode's title
 */
void CDShowZoom::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// Make sure the mouse click is on a list item
	m_eplist.ScreenToClient(&point);
	int index = m_eplist.HitTest(point);
	if (index == -1)
		return;
	ClientToScreen(&point);

	// Create a popup menu on the fly & show it
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING | MF_ENABLED, ID_MNU_COPY_EP_TITLE, L"Copy &Title");
	int selection = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, this);
	if (selection == 0)
		return;

	switch (selection)
	{
		case ID_MNU_COPY_EP_TITLE:
			CopyToClipboard(m_eplist.GetItemText(index, 0));
			break;

		default:
			LogMsgWin(L"Unknown menu option selected");
			break;
	}
}
