#pragma once

#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"
using namespace boost;

#include "CsortContext.h"




// CDShows dialog

class CDShows : public CDialog, public cSortContext
{
	DECLARE_DYNAMIC(CDShows)

public:
	CDShows(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDShows();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHOWS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClickShowList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkShowList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

	virtual void OnCancel();
	virtual void OnOK();

	void	AppendRow(const sShowListEntry* sle);
	void	SaveTopIndex();
	void	RestoreTopIndex();
	void	DeleteAllItems() { m_showlist.DeleteAllItems(); }

private:
	int			m_top_index{ 0 };
	CPoint		m_click_point;

	CListCtrl	m_showlist;
	static const tSortMap m_sort_map;
};
