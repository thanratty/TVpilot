#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"
#include "CDlogging.hpp"
#include "utils.hpp"
#include "logging.hpp"


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
	ON_BN_CLICKED(IDC_BTN_CLEAR,			&CDmessages::OnBtnClicked_Clear)
	ON_BN_CLICKED(IDC_BTN_ABORT_DOWNLOAD,	&CDmessages::OnBtnClicked_AbortDownload)
	ON_BN_CLICKED(IDC_BTN_LOGGING,          &CDmessages::OnBtn_Logging)
END_MESSAGE_MAP()


//
// CDmessages message handlers
//


void CDmessages::OnBtnClicked_Clear()
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


void CDmessages::OnBtnClicked_AbortDownload()
{
	GetParent()->PostMessage(WM_TVP_ABORT_DOWNLOAD);
}


void CDmessages::OnBtn_Logging()
{
	CDLogging	dlog(this);
	dlog.DoModal();
}

