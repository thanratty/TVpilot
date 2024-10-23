#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"
#include "utils.hpp"

#include "CDmessages.h"


// CDmessages dialog

IMPLEMENT_DYNAMIC(CDmessages, CDialogEx)

CDmessages::CDmessages(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MESSAGES, pParent)
{
}

CDmessages::~CDmessages()
{
}

void CDmessages::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MESSAGES, m_messages);
}


BEGIN_MESSAGE_MAP(CDmessages, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CLEAR,			&CDmessages::OnBtnClickedClear)
	ON_BN_CLICKED(IDC_BTN_ABORT_DOWNLOAD,	&CDmessages::OnBtnClickedAbortDownload)
END_MESSAGE_MAP()


//
// CDmessages message handlers
//


void CDmessages::OnBtnClickedClear()
{
	GetDlgItem(IDC_MESSAGES)->SetWindowText(L"");
}


void CDmessages::OnOK()
{
	WriteMessageLog(L"OnOK() intercepted");

	// The 'Messages' button is a SHOW/HIDE toggle, so let the parent do it's thing
	GetParent()->PostMessageW(WM_COMMAND, MAKEWPARAM(IDC_BTN_MESSAGES, BN_CLICKED));

	//CDialogEx::OnOK();
}


void CDmessages::OnCancel()
{
	WriteMessageLog(L"OnCancel() intercepted");

	// The 'Messages' button is a SHOW/HIDE toggle, so let the parent do it's thing
	GetParent()->PostMessageW(WM_COMMAND, MAKEWPARAM(IDC_BTN_MESSAGES, BN_CLICKED));

	//CDialogEx::OnCancel();
}


void CDmessages::OnBtnClickedAbortDownload()
{
	GetParent()->PostMessage(WM_ABORT_DOWNLOAD);
}

