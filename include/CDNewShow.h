#pragma once


// CDnewShow dialog

class CDnewShow : public CDialogEx
{
	DECLARE_DYNAMIC(CDnewShow)

public:
	CDnewShow(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDnewShow();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NEW_SHOW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_new_url{ L"https://epguides.com/" };

	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
