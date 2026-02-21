#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"

#include "CDLogFlags.hpp"
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
	ON_BN_CLICKED(IDC_BTN_CLEAR,			&CDmessages::OnBtn_Clear)
	ON_BN_CLICKED(IDC_BTN_ABORT_DOWNLOAD,	&CDmessages::OnBtn_AbortDownload)
	ON_BN_CLICKED(IDC_BTN_LOGGING,          &CDmessages::OnBtn_Logging)
	ON_MESSAGE(WM_TVP_ABORT_BTN_ENABLE,		&CDmessages::OnMsg_AbortEnable)
	ON_MESSAGE(WM_TVP_ABORT_BTN_DISABLE,	&CDmessages::OnMsg_AbortDisable)
END_MESSAGE_MAP()


//
// CDmessages message handlers
//


void CDmessages::OnBtn_Clear()
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


void CDmessages::OnBtn_AbortDownload()
{
	GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BTN_ABORT_DOWNLOAD, BN_CLICKED));
}


void CDmessages::OnBtn_Logging()
{
#if defined(_DEBUG)
	CDLogFlags	dlog(this);
	dlog.DoModal();
#endif
}


LRESULT CDmessages::OnMsg_AbortEnable(WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;

	GetDlgItem(IDC_BTN_ABORT_DOWNLOAD)->EnableWindow();
	return 0;
}

LRESULT CDmessages::OnMsg_AbortDisable(WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;

	GetDlgItem(IDC_BTN_ABORT_DOWNLOAD)->EnableWindow(FALSE);
	return 0;
}

