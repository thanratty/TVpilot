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
	DDX_Control(pDX, IDC_MESSAGES,			 m_messages);
	DDX_Control(pDX, IDC_BTN_LOGGING,		 m_btn_logging);
}


BEGIN_MESSAGE_MAP(CDmessages, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CLEAR,			&CDmessages::OnBtn_Clear)
	ON_BN_CLICKED(IDC_BTN_LOGGING,          &CDmessages::OnBtn_Logging)
END_MESSAGE_MAP()



BOOL CDmessages::OnInitDialog()
{
	CDialogEx::OnInitDialog();

#if !defined(_DEBUG) || (ENABLE_CONSOLE_LOGGING==0)
	m_btn_logging.EnableWindow(0);
	m_btn_logging.ShowWindow(SW_HIDE);
#endif

	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}



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
	GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_CHK_SHOW_LOG, BN_CLICKED));

	//CDialogEx::OnOK();
}


void CDmessages::OnCancel()
{
	// Let the parent look after the SHOW/HIDE logic
	GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_CHK_SHOW_LOG, BN_CLICKED));

	//CDialogEx::OnCancel();
}


void CDmessages::OnBtn_Logging()
{
#if defined(_DEBUG) && (ENABLE_CONSOLE_LOGGING==1)
	CDLogFlags	dlog(this);
	dlog.DoModal();
#endif
}


