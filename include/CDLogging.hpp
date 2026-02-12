#pragma once
#include "afxdialogex.h"


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
};
