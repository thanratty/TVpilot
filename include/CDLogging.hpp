#pragma once
#include "afxdialogex.h"

#include "logging.hpp"

// CDlogging dialog

class CDLogging : public CDialog
{
	DECLARE_DYNAMIC(CDLogging)

public:
	CDLogging(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDLogging();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGGING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();

	afx_msg void OnChkBoxClicked(UINT nID);
	afx_msg void OnBtnClicked_LogAll();
	afx_msg void OnBtnClicked_LogNone();

private:
	eLogFlags	m_temp_flags{ 0 };
	void		UpdateUI();
};
