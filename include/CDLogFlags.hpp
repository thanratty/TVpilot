#pragma once
#include "afxdialogex.h"

#include "logging.hpp"



// CDLogFlags dialog

class CDLogFlags : public CDialog
{
	DECLARE_DYNAMIC(CDLogFlags)

public:
	CDLogFlags(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDLogFlags();

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
