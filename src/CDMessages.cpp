#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"
#include "utils.hpp"

#include "CDmessages.hpp"


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
	// Let the parent look after the SHOW/HIDE logic
	GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_CHK_DEBUG_LOG, BN_CLICKED));

	//CDialogEx::OnOK();
}


void CDmessages::OnCancel()
{
	// Let the parent look after the SHOW/HIDE logic
	GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_CHK_DEBUG_LOG, BN_CLICKED));

	//CDialogEx::OnCancel();
}


void CDmessages::OnBtnClickedAbortDownload()
{
	GetParent()->PostMessage(WM_TVP_ABORT_DOWNLOAD);
}

