#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include "common.hpp"
#include "utils.hpp"
#include "logging.hpp"

#include "CDLogging.hpp"


// CDlogging dialog

IMPLEMENT_DYNAMIC(CDLogging, CDialog)

CDLogging::CDLogging(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LOGGING, pParent)
{

}

CDLogging::~CDLogging()
{
}

void CDLogging::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDLogging, CDialog)
	ON_COMMAND_RANGE(100, 110, &CDLogging::OnChkBoxClicked)
END_MESSAGE_MAP()


#define	NUM_BUTTONS 5


CButton* bptrs[ NUM_BUTTONS ];


// CDlogging message handlers

BOOL CDLogging::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	GetClientRect(&rect);

	int box_width  = rect.Width();
	int box_height = rect.Height();


	int offset = box_height/20;		// top margin
	int instep = box_width/8;		// left margin

	int b_width  = box_width / 4;
	int b_height = 30;

	int step     = b_height;		// gap between buttons



//	unsigned num_buttons = 0;

	for (unsigned i = 0; i < NUM_BUTTONS; i++)
	{
		CString caption;
		caption.Format(L"Option %u", i);

		bptrs[i] = new CButton();
		bptrs[i]->Create(caption, WS_CHILD | WS_VISIBLE | BS_CHECKBOX, CRect(instep, offset + i*step, instep + b_width, offset + i*step + b_height), this, 100+i);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}




#include "CDLogging.hpp"

void CDLogging::OnCancel()
{
	for (unsigned i = 0; i < NUM_BUTTONS; i++)
		delete bptrs[i];

	CDialog::OnCancel();
}

void CDLogging::OnOK()
{
	for (unsigned i = 0; i < NUM_BUTTONS; i++)
		delete bptrs[i];

	CDialog::OnOK();
}




afx_msg void CDLogging::OnChkBoxClicked(UINT nID)
{
	LOG_PRINT(eLogFlags::INFO, L"Button Id %u\n", nID);

	unsigned index = nID - 100;
	int state;

	state = bptrs[index]->GetCheck();

	state = (state == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED;

	bptrs[index]->SetCheck(state);

}
