#pragma once

#include "Cshow.h"
#include "CsortContext.h"



// CDepisodes dialog

class CDepisodes : public CDialogEx, public cSortContext
{
		DECLARE_DYNAMIC(CDepisodes)

public:
		CDepisodes(CWnd* pParent, const show* pshow);
		virtual ~CDepisodes();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EPISODES };
#endif

protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		DECLARE_MESSAGE_MAP()

private:
		const show*				m_pshow{ nullptr };
		CFont					m_headerFont;
		CListCtrl				m_eplist;

static	const tSortMap			m_sort_map;


public:
		virtual BOOL OnInitDialog();
		afx_msg void OnColumnClickEpisodesList(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};
