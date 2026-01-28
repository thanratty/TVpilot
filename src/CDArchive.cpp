#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "utils.hpp"

#include "CDarchive.hpp"







//
// Column numbers in the show archive list box
//
STATIC constexpr int COL_ARCHIVE_TITLE = 0;
STATIC constexpr int COL_ARCHIVE_NUMBER = 1;
STATIC constexpr int COL_ARCHIVE_LAST_DATE_STR = 2;
STATIC constexpr int COL_ARCHIVE_LAST_DATE_SORT = 3;
//
STATIC constexpr int COL_ARCHIVE_NUM_COLS = 4;





/**
 * Initialises the static const class member variable m_sort_map
 */
const tSortMap CDArchive::m_sort_map = {
	// 1st entry is used as the default sort order
	{ COL_ARCHIVE_TITLE, {
			{COL_ARCHIVE_TITLE,				AlphaCompareFunc  },
			{COL_ARCHIVE_LAST_DATE_SORT,	NumberCompareFunc }
		}},

	{ COL_ARCHIVE_NUMBER, {
			{COL_ARCHIVE_NUMBER,			NumberCompareFunc  },
			{COL_ARCHIVE_LAST_DATE_SORT,    NumberCompareFunc }
		}},

	{ COL_ARCHIVE_LAST_DATE_STR, {
			{COL_ARCHIVE_LAST_DATE_SORT,	NumberCompareFunc },
			{COL_ARCHIVE_TITLE,				AlphaCompareFunc  }
		}}
};







// CDarchive dialog

IMPLEMENT_DYNAMIC(CDArchive, CDialog)

CDArchive::CDArchive(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_ARCHIVE, pParent),
	cSortContext(&m_sort_map, &m_archivelist)
{
}

CDArchive::~CDArchive()
{
}

void CDArchive::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ARCHIVE, m_archivelist);
}


// ON_NOTIFY macro gives an annoying false warning
#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CDArchive, CDialog)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ARCHIVE, &CDArchive::OnColumnClick)
	ON_NOTIFY(NM_DBLCLK,       IDC_LIST_ARCHIVE, &CDArchive::OnDblclkListArchive)
END_MESSAGE_MAP()

#pragma warning( pop )









BOOL CDArchive::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_archivelist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_SORTASCENDING );

	// This dialog box lives in the tab control, so resize to that.
	CRect  rc;
	GetParent()->GetDlgItem(IDC_TAB1)->GetClientRect(rc);
	MoveWindow(&rc);
	// Now fit the list control to the dialog box
	m_archivelist.MoveWindow(&rc);

	// Make the column header bold
	SetListHeaderFont(&m_archivelist);

	/*
	 * Add four columns: "Title"  "Number" (of episodes) "Last" (airdate)  "Next" (airdate)
	 */

	LVCOLUMNW col = { 0 };
	col.fmt = LVCFMT_LEFT;
	col.mask = LVCF_FMT | LVCF_TEXT;
	col.cx = -1;

	col.pszText = L"Title";
	m_archivelist.InsertColumn(COL_ARCHIVE_TITLE, &col);
	col.pszText = L"Number";
	m_archivelist.InsertColumn(COL_ARCHIVE_NUMBER, &col);
	col.pszText = L"Last";
	m_archivelist.InsertColumn(COL_ARCHIVE_LAST_DATE_STR, &col);
	// 1 hidden date sort col
	col.pszText = L"D-Last";
	m_archivelist.InsertColumn(COL_ARCHIVE_LAST_DATE_SORT, &col);


	// Set column widths
	int listWidth = rc.Width();
	m_archivelist.SetColumnWidth(COL_ARCHIVE_TITLE, 35 * listWidth / 100);				// 35%
	m_archivelist.SetColumnWidth(COL_ARCHIVE_NUMBER, 10 * listWidth / 100);				// 10%
	m_archivelist.SetColumnWidth(COL_ARCHIVE_LAST_DATE_STR, 20 * listWidth / 100);		// 20%
	// Hidden col
	m_archivelist.SetColumnWidth(COL_ARCHIVE_LAST_DATE_SORT, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}





void CDArchive::AppendRow(sShowListEntry* sle)
{
	/**
	 *  Add a row to the Shows list
	 *
	 */
	CString ui_numeps;
	ui_numeps.Format(L"%u", sle->num_episodes);

	// Number of rows already in list (0 based) = new row/item number.
	int count = m_archivelist.GetItemCount();

	// Add a row of data
	int index = m_archivelist.InsertItem(count, sle->ui_title);
	m_archivelist.SetItemText(index, COL_ARCHIVE_NUMBER, ui_numeps);
	m_archivelist.SetItemText(index, COL_ARCHIVE_LAST_DATE_STR, sle->ui_last_airdate_string);
	m_archivelist.SetItemText(index, COL_ARCHIVE_LAST_DATE_SORT, sle->ui_last_airdate_sort);
	// Set the hash
	m_archivelist.SetItemData(index, sle->hash);

}




void CDArchive::OnCancel()
{
	LogMsgWindow(L"CDArchive::OnCancel() discarded");
	//CDialog::OnCancel();
}

void CDArchive::OnOK()
{
	LogMsgWindow(L"CDArchive::OnOK() discarded");
	//CDialog::OnOK();
}




void CDArchive::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	static sPopupContext		context;

	// Save screen co-ords
	context.click_point = point;

	// Make sure the mouse click is on a list item
	m_archivelist.ScreenToClient(&point);
	int index = m_archivelist.HitTest(point);
	if (index == -1)
		return;

	// It's a valid click! Get the top level dialog to handle the context menu. Pass in the hash & the screen co-ords address
	context.dialog_id  = IDD_ARCHIVE;
	context.list_index = index;
	context.show_hash  = m_archivelist.GetItemData(index);

	GetParent()->PostMessage(WM_TVP_SHOW_CONTEXT_MENU, reinterpret_cast<WPARAM>(&context));

}




/**
 * Double-click on a show to zoom it's episodes
 * 
 */
void CDArchive::OnDblclkListArchive(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pItem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	int row = pItem->iItem;

	if (row != -1)
	{
		DWORD hash = m_archivelist.GetItemData(row);
		GetParent()->PostMessage(WM_TVP_ZOOM_EPISODES, static_cast<WPARAM>(hash), 0);
	}

	*pResult = 0;
}



void CDArchive::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// This msg is always 'handled'
	*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int new_sort_col   = pNMLV->iSubItem;

	// Set the new sort column - if needed re-sort the list control
	if (SetSortColumn(new_sort_col) == true)
		SortList();
}

