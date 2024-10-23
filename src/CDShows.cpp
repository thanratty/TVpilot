#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <string>

#include "resource.h"
#include "CDShows.h"
#include "CepcheckDlg.h"

#include "utils.hpp"





//
// Column numbers in the show list box
//
STATIC constexpr int COL_SHOW_TITLE          = 0;
STATIC constexpr int COL_SHOW_NUMBER         = 1;
STATIC constexpr int COL_SHOW_LAST_DATE_STR  = 2;
STATIC constexpr int COL_SHOW_NEXT_DATE_STR  = 3;
STATIC constexpr int COL_SHOW_LAST_DATE_SORT = 4;
STATIC constexpr int COL_SHOW_NEXT_DATE_SORT = 5;
//
STATIC constexpr int COL_SHOW_NUM_COLS       = 6;





/**
 * Initialises the static const class member variable m_sort_cols
 */
const tSortMap CDShows::m_sort_map = {
	// 1st entry is used as the default sort order
	{ COL_SHOW_TITLE, {
			{ COL_SHOW_TITLE,			AlphaCompareFunc  }
		}},

	{ COL_SHOW_NUMBER, {
			{ COL_SHOW_NUMBER,			NumberCompareFunc },
			{ COL_SHOW_TITLE,			AlphaCompareFunc  }
		}},

	{ COL_SHOW_LAST_DATE_STR, {
			{ COL_SHOW_LAST_DATE_SORT,	NumberCompareFunc },
			{ COL_SHOW_TITLE,			AlphaCompareFunc  }
		}},

	{ COL_SHOW_NEXT_DATE_STR, {
			{ COL_SHOW_NEXT_DATE_SORT,	NumberCompareFunc },
			{ COL_SHOW_TITLE,			AlphaCompareFunc  }
		}}
};






IMPLEMENT_DYNAMIC(CDShows, CDialog)

CDShows::CDShows(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SHOWS, pParent),
	  cSortContext(&m_sort_map, &m_showlist)
{
}

CDShows::~CDShows()
{
}

void CDShows::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHOW_LIST, m_showlist);
}

// ON_NOTIFY macro gives an annoying compiler warning
#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CDShows, CDialog)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SHOW_LIST,  &CDShows::OnColumnClickShowList)
	ON_NOTIFY(NM_DBLCLK,       IDC_SHOW_LIST,  &CDShows::OnDblClkShowList)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#pragma warning( pop )






/**
 *  Initialise everything in the dialog
 *
 */
BOOL CDShows::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_showlist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_SORTASCENDING );

	// This dialog box lives in the tab control, so resize to that.
	CRect  rc;
	GetParent()->GetDlgItem(IDC_TAB1)->GetClientRect(rc);
	MoveWindow(&rc);
	// Now fit the list control to the dialog box
	m_showlist.MoveWindow(&rc);

	// Make the column header bold
	SetListHeaderFont(&m_showlist);

	/*
	 * Add columns: "Title"  "Number" (of episodes) "Last" (airdate)  "Next" (airdate).
	 * Two hidded columsn for date sorting contain Boost gregorian day numbers.
	 */

	LVCOLUMNW col{ 0 };
	col.fmt = LVCFMT_LEFT;
	col.mask = LVCF_FMT | LVCF_TEXT;
	col.cx = -1;

	col.pszText = L"Title";
	m_showlist.InsertColumn(COL_SHOW_TITLE, &col);
	col.pszText = L"Number";
	m_showlist.InsertColumn(COL_SHOW_NUMBER, &col);
	col.pszText = L"Last";
	m_showlist.InsertColumn(COL_SHOW_LAST_DATE_STR, &col);
	col.pszText = L"Next";
	m_showlist.InsertColumn(COL_SHOW_NEXT_DATE_STR, &col);
	// 2 hidden date sort cols
	col.pszText = L"D-Last";
	m_showlist.InsertColumn(COL_SHOW_LAST_DATE_SORT, &col);
	col.pszText = L"D-Next";
	m_showlist.InsertColumn(COL_SHOW_NEXT_DATE_SORT, &col);


	// Set column widths
	int listWidth = rc.Width();
	m_showlist.SetColumnWidth(COL_SHOW_TITLE, 5*listWidth / 10);				// 50%
	m_showlist.SetColumnWidth(COL_SHOW_NUMBER, 1*listWidth / 10);				// 10%
	m_showlist.SetColumnWidth(COL_SHOW_LAST_DATE_STR, 2*listWidth / 10);		// 20%
	m_showlist.SetColumnWidth(COL_SHOW_NEXT_DATE_STR, 2*listWidth / 10);		// 20%
	// Two hidden cols
	m_showlist.SetColumnWidth(COL_SHOW_LAST_DATE_SORT, 0);
	m_showlist.SetColumnWidth(COL_SHOW_NEXT_DATE_SORT, 0);

	// Return TRUE unless you set the focus to a control

	return TRUE;
}




/**
 *  Add a row to the Shows list
 *
 */
void CDShows::AppendRow( const sShowListEntry *sle)
{
	CString ui_numeps;
	ui_numeps.Format(L"%u", sle->num_episodes);

	// Number of rows already in list (0 based) = new row/item number.
	int index = m_showlist.GetItemCount();

	// Add a row of data
	index = m_showlist.InsertItem(index, sle->ui_title);

	m_showlist.SetItemText(index, COL_SHOW_NUMBER, ui_numeps);
	m_showlist.SetItemText(index, COL_SHOW_LAST_DATE_STR,  sle->ui_last_airdate_string);
	m_showlist.SetItemText(index, COL_SHOW_NEXT_DATE_STR,  sle->ui_next_airdate_string);
	m_showlist.SetItemText(index, COL_SHOW_LAST_DATE_SORT, sle->ui_last_airdate_sort);
	m_showlist.SetItemText(index, COL_SHOW_NEXT_DATE_SORT, sle->ui_next_airdate_sort);
	// Set the hash
	m_showlist.SetItemData(index, sle->hash);
}




/**
 *  Clicking on a column header toggle the list sort order by that column
 *
 */
void CDShows::OnColumnClickShowList(NMHDR* pNMHDR, LRESULT* pResult)
{
	// We will always 'handle' this message
	*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int new_sort_col   = pNMLV->iSubItem;

	// Set the new sort column - if needed re-sort the list control
	if (SetSortColumn(new_sort_col) == true)
		SortList();
}




/**
 *  Stop <ESCAPE> closing the dialog
 *
 */
void CDShows::OnCancel()
{
	WriteMessageLog(L"CDShows::OnCancel() discarded");
	//CDialog::OnCancel();
}




/**
 *  Stop <RETURN> closing the dialog
 *
 */
void CDShows::OnOK()
{
	WriteMessageLog(L"CDShows::OnOK() discarded");
	//CDialog::OnOK();
}




/**
 *  Save the index of the topmost visible iterm in the list.
 *  TODO This Save & the Restore could be done better!
 *
 */
void CDShows::SaveTopIndex()
{
	m_top_index = m_showlist.GetTopIndex();
}




/**
 *  Scroll the list so the previously topmost item is visible again
 *
 */
void CDShows::RestoreTopIndex()
{
	m_showlist.EnsureVisible(m_top_index, FALSE);
}




/**
 *  Double click a show to zoom-in on all its episodes
 *
 */
void CDShows::OnDblClkShowList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pItem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	int row = pItem->iItem;
	if (row != -1)
	{
		DWORD hash = m_showlist.GetItemData(row);
		GetParent()->PostMessage(WM_ZOOM_EPISODES, static_cast<WPARAM>(hash));
	}

	*pResult = 0;
}




void CDShows::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	static sPopupContext		context;

	// Save screen co-ords
	context.click_point = point;

	// Make sure the mouse click is on a list item
	m_showlist.ScreenToClient(&point);
	int index = m_showlist.HitTest(point);
	if (index == -1)
		return;

	// It's a valid click! Get the top level dialog to handle the context menu. Pass in the hash & the screen co-ords address
	context.dialog_id  = IDD_SHOWS;
	context.list_index = index;
	context.show_hash  = m_showlist.GetItemData(index);

	GetParent()->PostMessage(WM_SHOW_CONTEXT_MENU, reinterpret_cast<WPARAM>(&context));
}



