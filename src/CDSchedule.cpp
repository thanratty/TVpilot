#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;

#include "common.hpp"

#include "CepcheckDlg.hpp"
#include "utils.hpp"

#include "CDSchedule.hpp"





/**
 * Highlight colors for the schedule listcontrol
 */
#define		COLOR_GOT		rgbLime
#define		COLOR_NOT_GOT	rgbRed



 /**
   * Column numbers in  the schedule dialog listcontrol
   */
STATIC constexpr int COL_SCHED_SHOW = 0;
STATIC constexpr int COL_SCHED_EP_NUM = 1;
STATIC constexpr int COL_SCHED_DATE_STR = 2;
STATIC constexpr int COL_SCHED_TITLE = 3;
STATIC constexpr int COL_SCHED_DATE_SORT = 4;
STATIC constexpr int COL_SCHED_EP_FLAGS = 5;
//
STATIC constexpr int COL_SCHED_NUM_COLS = 6;





/**
 * Initialises the static const class member variable m_sort_map
 */
const tSortMap CDSchedule::m_sort_map = {
	// 1st entry is used as the default sort order
	{ COL_SCHED_DATE_STR, {
			{ COL_SCHED_DATE_SORT, NumberCompareFunc  },
			{ COL_SCHED_SHOW,      AlphaCompareFunc   },
			{ COL_SCHED_EP_NUM,    EpisodeCompareFunc }
		}},

	{ COL_SCHED_SHOW, {
			{ COL_SCHED_SHOW,	   AlphaCompareFunc   },
			{ COL_SCHED_EP_NUM,	   EpisodeCompareFunc }
		}},

	{ COL_SCHED_TITLE, {
			{ COL_SCHED_TITLE,	   AlphaCompareFunc   },
			{ COL_SCHED_SHOW,	   AlphaCompareFunc   },
			{ COL_SCHED_EP_NUM,    EpisodeCompareFunc }
		}}
};










// CDSchedule dialog

IMPLEMENT_DYNAMIC(CDSchedule, CDialog)

CDSchedule::CDSchedule(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SCHEDULE, pParent),
	  cSortContext(&m_sort_map, &m_schedlist)
{
}

CDSchedule::~CDSchedule()
{
}

void CDSchedule::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCHED_LIST, m_schedlist);
}


// ON_NOTIFY macro gives an annoying compiler warning
#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CDSchedule, CDialog)
	ON_MESSAGE(WM_TVP_SCHED_EP_FLAGS_CHANGED,		&CDSchedule::OnSchedEpFlagsChanged)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SCHED_LIST,  &CDSchedule::OnColumnClick)
	ON_NOTIFY(NM_DBLCLK,	   IDC_SCHED_LIST,  &CDSchedule::OnDblclkSchedList)
	ON_NOTIFY(NM_CUSTOMDRAW,   IDC_SCHED_LIST,  &CDSchedule::OnCustomdrawSchedList)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#pragma warning( pop )






BOOL CDSchedule::OnInitDialog()
{
	CDialog::OnInitDialog();

	DWORD style = m_schedlist.GetExtendedStyle();
	style |= (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SINGLEROW);
	m_schedlist.SetExtendedStyle(style);

	// This dialog box lives in the tab control, so resize to that.
	CRect  rc;
	GetParent()->GetDlgItem(IDC_TAB1)->GetClientRect(rc);
	MoveWindow(&rc);
	// Fit the list control to the dialog box
	m_schedlist.MoveWindow(&rc);

	// Make the column header bold
	SetListHeaderFont(&m_schedlist);

	// Add  columns to the list box: "Show"  "Episode #"  "Airdate"  "Title" are visible

	LVCOLUMNW col{ 0 };
	col.fmt  = LVCFMT_LEFT;
	col.mask = LVCF_FMT | LVCF_TEXT;
	col.cx   = -1;

	col.pszText = L"Show";
	m_schedlist.InsertColumn(COL_SCHED_SHOW, &col);
	col.pszText = L"Episode";
	m_schedlist.InsertColumn(COL_SCHED_EP_NUM, &col);
	col.pszText = L"Date";
	m_schedlist.InsertColumn(COL_SCHED_DATE_STR, &col);
	col.pszText = L"Title";
	m_schedlist.InsertColumn(COL_SCHED_TITLE, &col);
	col.pszText = L"D-sort";
	m_schedlist.InsertColumn(COL_SCHED_DATE_SORT, &col);
	col.pszText = L"EpFlags";
	m_schedlist.InsertColumn(COL_SCHED_EP_FLAGS, &col);

	// Set the individual column widths
	int listWidth = rc.Width();
	m_schedlist.SetColumnWidth(COL_SCHED_SHOW,     30 * listWidth / 100);
	m_schedlist.SetColumnWidth(COL_SCHED_EP_NUM,   10 * listWidth / 100);
	m_schedlist.SetColumnWidth(COL_SCHED_DATE_STR, 15 * listWidth / 100);
	m_schedlist.SetColumnWidth(COL_SCHED_TITLE,    45 * listWidth / 100);

	// Set to zero width (ie hide) the date sort column and episode flags
	m_schedlist.SetColumnWidth(COL_SCHED_DATE_SORT, 0);
	m_schedlist.SetColumnWidth(COL_SCHED_EP_FLAGS, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}




/**
 * Click on a column header to sort the list control entries on that column, alternate ascending/descending
 *
 */
void CDSchedule::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// We always 'handle' this message
	*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int new_sort_col   = pNMLV->iSubItem;

	// Set the new sort column - if needed re-sort the list control
	if (SetSortColumn(new_sort_col) == true)
		SortList();
}




/**
 * APPEND an episode entry to the schedule listcontrol
 * 
 * When first populated, episodes in the filter range are returned from the model in date order,
 * so use GetItemCount() to insert a new item at the bottom. This function presumes the first
 * column is the Show Title. Also, the 'Sorted' property must not be set in the resource editor.
 */
void CDSchedule::AppendRow(const sScheduleListEntry* gle)
{
	// Number if rows already in list (0 based) = new row/item number.
	int index = m_schedlist.GetItemCount();

	// Add a row of data
	index = m_schedlist.InsertItem(index, gle->ui_show_title);

	m_schedlist.SetItemText(index, COL_SCHED_EP_NUM,    gle->ui_episode_number);
	m_schedlist.SetItemText(index, COL_SCHED_DATE_STR,  gle->ui_airdate_string);
	m_schedlist.SetItemText(index, COL_SCHED_TITLE,     gle->ui_episode_title);
	m_schedlist.SetItemText(index, COL_SCHED_DATE_SORT, gle->ui_airdate_sort);
	// The show Hash is stored as the row's DWORD data item
	m_schedlist.SetItemData(index, gle->hash);
	// Episode flags stored in a hidden column
	CString flags;
	flags.Format(L"%d", gle->episode_flags);
	m_schedlist.SetItemText(index, COL_SCHED_EP_FLAGS, flags);
}




void CDSchedule::OnOK()
{
	WriteMessageLog(L"CDSchedule::OnOK() discarded");
	//CDialog::OnOK();
}


CString CDSchedule::GetEpisodeNumber(int index) const
{
	return m_schedlist.GetItemText(index, COL_SCHED_EP_NUM);
}


CString CDSchedule::GetEpisodeTitle(int index) const
{
	return m_schedlist.GetItemText(index, COL_SCHED_TITLE);
}


CString CDSchedule::GetEpisodeShow(int index) const
{
	return m_schedlist.GetItemText(index, COL_SCHED_SHOW);
}




/**
 * Double-click an episode in the list to popup a list of all that show's episodes
 */
void CDSchedule::OnDblclkSchedList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pItem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int row = pItem->iItem;

	if (row != -1)
	{
		DWORD hash = m_schedlist.GetItemData(row);
		GetParent()->PostMessageW(WM_TVP_ZOOM_EPISODES, static_cast<WPARAM>(hash));
	}

	*pResult = 0;
}




/**
 * CPoint parameter is in screen co-ordinates. Ask the parent to show the context menu for the Schedule list.
 */
void CDSchedule::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	static sPopupContext	context;

	// Save screen co-ords
	context.click_point = point;

	// Make sure the mouse click is on a list item
	m_schedlist.ScreenToClient(&point);
	int index = m_schedlist.HitTest(point);
	if (index == -1)
		return;

	// It's a valid click! Get the top level dialog to handle the context menu.
	context.dialog_id   = IDD_SCHEDULE;
	context.pList       = &m_schedlist;
	context.list_index  = index;
	context.show_hash   = m_schedlist.GetItemData(index);
	context.ep_num      = m_schedlist.GetItemText(index, COL_SCHED_EP_NUM);

	CString      str = m_schedlist.GetItemText(index, COL_SCHED_EP_FLAGS);
	context.ep_flags = static_cast<episodeflags>(_ttoi(str));

	GetParent()->PostMessage(WM_TVP_SHOW_CONTEXT_MENU, reinterpret_cast<WPARAM>(&context));
}




/**
 * Handler for WM_NOTIFY NM_CUSTOMDRAW message.
 * Intercept this message to allow row colouring in the listbox control
 */
void CDSchedule::OnCustomdrawSchedList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW*  pNMCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	// If we don't handle the dwDrawStage do the default action
	*pResult = CDRF_DODEFAULT;

	switch (pNMCD->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;

		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
			//DWORD hash = pNMCD->nmcd.lItemlParam;
			int index = pNMCD->nmcd.dwItemSpec;

			CString      str = m_schedlist.GetItemText(index, COL_SCHED_EP_FLAGS);
			episodeflags efl = static_cast<episodeflags>(_ttoi(str));

			// Set the colour that matches the episode's flags
			if (efl & episodeflags::EP_FL_GOT)
				pNMCD->clrText = COLOR_GOT;
			else if (efl & episodeflags::EP_FL_NOT_GOT)
				pNMCD->clrText = COLOR_NOT_GOT;
			else
				pNMCD->clrText = rgbBlack;
			break;
	}
}




/**
 * If an episode's flags are changed in the popup context menu, this is how the dialog is notified.
 */
afx_msg LRESULT CDSchedule::OnSchedEpFlagsChanged(WPARAM wParam, LPARAM )
{
	sPopupContext const * const pcontext = reinterpret_cast<sPopupContext*>(wParam);

	CString strFlags;
	strFlags.Format(L"%d", pcontext->ep_flags);
	m_schedlist.SetItemText(pcontext->list_index, COL_SCHED_EP_FLAGS, strFlags);

	return 0;
}




/**
 * Trap key presses in the schedule list.
 *   <RETURN> key zooms a shows episode
 *   Apps/Context key pops up the context menu
 */
BOOL CDSchedule::PreTranslateMessage(MSG* pMsg)
{

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->hwnd == m_schedlist.GetSafeHwnd()))
	{
		int index = GetSelectedListItem(m_schedlist);
		if (index != -1)
		{
			if (pMsg->wParam == VK_RETURN)
			{
				DWORD hash = m_schedlist.GetItemData(index);
				GetParent()->PostMessageW(WM_TVP_ZOOM_EPISODES, hash);
				return TRUE;
			}
			else if (pMsg->wParam == VK_APPS)
			{
				CRect rect;
				m_schedlist.GetItemRect(index, &rect, LVIR_LABEL);
				CPoint point(rect.CenterPoint());
				m_schedlist.ClientToScreen(&point);

				OnContextMenu(nullptr, point);
				return TRUE;
			}
		}
	}

	// Message not handled in above traps. Pass it on to the default handler.
	return CDialog::PreTranslateMessage(pMsg);
}

