#pragma once

#include <string>

// CDInputBox dialog

class CDInputBox : public CDialogEx
{
	DECLARE_DYNAMIC(CDInputBox)

public:
	CDInputBox(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDInputBox();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INPUT_BOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString		m_input{ L"" };
	std::string m_input_str{ "" };

	CString		m_prompt{ L"" };
	CString		m_title{ L"" };

	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
