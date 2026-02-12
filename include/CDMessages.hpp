#pragma once


// CDmessages dialog

class CDmessages : public CDialogEx
{
	DECLARE_DYNAMIC(CDmessages)

public:
	CDmessages(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDmessages();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MESSAGES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_messages;
	afx_msg void OnBtnClicked_Clear();
	afx_msg void OnBtnClicked_AbortDownload();
	afx_msg void OnBtn_Logging();

	virtual void OnOK();
	virtual void OnCancel();
};
