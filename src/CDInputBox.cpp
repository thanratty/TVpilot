#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <algorithm>

#include "common.hpp"

#include "CDInputBox.hpp"



// CDInputBox dialog

IMPLEMENT_DYNAMIC(CDInputBox, CDialogEx)

CDInputBox::CDInputBox(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INPUT_BOX, pParent)
{
}

CDInputBox::~CDInputBox()
{
}

void CDInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_INPUT, m_input);
}


BEGIN_MESSAGE_MAP(CDInputBox, CDialogEx)
END_MESSAGE_MAP()




BOOL CDInputBox::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetDlgItem(IDC_INPUT_PROMPT)->SetWindowText(m_prompt);
	SetWindowText(m_title);

	CEdit* ed = (CEdit*) GetDlgItem(IDC_EDT_INPUT);
	ed->SetSel(static_cast<DWORD>(-1));
	ed->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
				   // EXCEPTION: OCX Property Pages should return FALSE
}


void CDInputBox::OnOK()
{
	CDialogEx::OnOK();

	// Also build the std::string copy of the input
	m_input.MakeLower();
	m_input_str = CW2A(m_input, CP_UTF8);
}
