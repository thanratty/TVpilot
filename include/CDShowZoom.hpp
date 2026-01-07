#pragma once

#include "Cshow.hpp"
#include "CsortContext.hpp"



// CDShowZoom dialog

class CDShowZoom : public CDialogEx, public cSortContext
{
		DECLARE_DYNAMIC(CDShowZoom)

public:
		CDShowZoom(CWnd* pParent, const show* pshow);
		virtual ~CDShowZoom();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHOW_ZOOM };
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
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};
