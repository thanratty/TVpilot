#include "config.h"

#pragma warning( disable : 26812 )

#include "pch.h"

//--

#include "common.hpp"

#include "CDnewShow.hpp"


// CDnewShow dialog

IMPLEMENT_DYNAMIC(CDnewShow, CDialogEx)

CDnewShow::CDnewShow(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NEW_SHOW, pParent)
{
}

CDnewShow::~CDnewShow()
{
}

void CDnewShow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NEW_URL, m_new_url);
}


BEGIN_MESSAGE_MAP(CDnewShow, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDnewShow::OnBnClickedOk)
END_MESSAGE_MAP()


// CDnewShow message handlers


void CDnewShow::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


BOOL CDnewShow::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	HWND hWnd;
	GetDlgItem(IDC_NEW_URL, &hWnd);
	::PostMessage(hWnd, WM_SETFOCUS, 0, 0);
	PostMessage(WM_NEXTDLGCTL, (WPARAM) hWnd, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
