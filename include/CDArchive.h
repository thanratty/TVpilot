#pragma once


// CDArchive dialog - NO SORTABLE COLUMNS IN THE LIST CONTROL!!!

class CDArchive : public CDialog
{
	DECLARE_DYNAMIC(CDArchive)

public:
	CDArchive(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDArchive();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ARCHIVE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void AppendRow(sShowListEntry* sle);
	void DeleteAllItems() { m_archivelist.DeleteAllItems(); }

	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnDblclkListArchive(NMHDR* pNMHDR, LRESULT* pResult);

public:
	CListCtrl	m_archivelist;

private:
	CPoint		m_click_point;

};
