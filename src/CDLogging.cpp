#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <array>

#include "common.hpp"
#include "utils.hpp"
#include "logging.hpp"

#include "CDLogging.hpp"




// I tried to make this & the declaration const but the compiler didn't like anything I tried!
extern std::array<sLogFlagDef, NUM_LOG_FLAGS>  log_flags;

// Dialog control ID of the check boxes we create
constexpr UINT32 BTN_BASE_ID = 100;

// Large anough to hold a pointer to each newly minted button
STATIC CButton* bptrs[NUM_LOG_FLAGS]{};




// CDlogging dialog

IMPLEMENT_DYNAMIC(CDLogging, CDialog)

CDLogging::CDLogging(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LOGGING, pParent)
{
}


CDLogging::~CDLogging()
{
	for (unsigned i = 0; i < NUM_LOG_FLAGS; i++) {
		if (bptrs[i] != nullptr) {
			delete bptrs[i];
			bptrs[i] = nullptr;
		}
	}
}


void CDLogging::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDLogging, CDialog)
	ON_COMMAND_RANGE(BTN_BASE_ID, (BTN_BASE_ID+NUM_LOG_FLAGS-1), &CDLogging::OnChkBoxClicked)
	ON_BN_CLICKED(IDC_BTN_LOG_ALL,		&CDLogging::OnBtnClicked_LogAll)
	ON_BN_CLICKED(IDC_BTN_LOG_NONE,		&CDLogging::OnBtnClicked_LogNone)
END_MESSAGE_MAP()







/**
 * Create a checkbox for each LOG option flag & position them in the dialog.
 *
 */
BOOL CDLogging::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Only mod a local copy until 'OK' is pressed.
	m_temp_flags = GetLogFlags();


	CRect rect;
	GetClientRect(&rect);

	int box_width  = rect.Width();
	int box_height = rect.Height();

	int offset = box_height/20;		// Top margin
	int instep = box_width/8;		// Left margin

	int b_width  = box_width / 4;
	int b_height = 30;

	int step     = b_height;		// Gap between buttons


	for (unsigned btn_num = 0; btn_num < log_flags.size(); btn_num++)
	{
		// Buttons are deleted in the dialog destructor
		bptrs[btn_num] = new CButton();
		bptrs[btn_num]->Create(	log_flags[btn_num].description, 
								WS_CHILD | WS_VISIBLE | BS_CHECKBOX, 
								CRect(instep, offset + btn_num*step, instep + b_width, offset + btn_num*step + b_height), 
								this, 
								BTN_BASE_ID + btn_num);
	}

	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}




void CDLogging::OnCancel()
{
	if (AfxMessageBox(L"Discard all changes?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
		return;

	CDialog::OnCancel();
}

void CDLogging::OnOK()
{
	SetLogFlags(m_temp_flags);

	CDialog::OnOK();
}




afx_msg void CDLogging::OnChkBoxClicked(UINT nID)
{
	unsigned btn_index = nID - BTN_BASE_ID;

	unsigned chk_state = bptrs[btn_index]->GetCheck();
	chk_state = (chk_state == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED;
	bptrs[btn_index]->SetCheck(chk_state);

	eLogFlags mask = log_flags[btn_index].mask;

	if (chk_state == BST_CHECKED)
		m_temp_flags |= mask;
	else
		m_temp_flags &= ~mask;
}



afx_msg void CDLogging::OnBtnClicked_LogAll()
{
	m_temp_flags = eLogFlags::ALL;
	UpdateUI();
}

afx_msg void CDLogging::OnBtnClicked_LogNone()
{
	m_temp_flags = eLogFlags::NONE;
	UpdateUI();
}


void CDLogging::UpdateUI()
{
	int state;

	for (unsigned btn_num = 0; btn_num < log_flags.size(); btn_num++)
	{
		state = flags(m_temp_flags & log_flags[btn_num].mask) ? BST_CHECKED : BST_UNCHECKED;
		bptrs[btn_num]->SetCheck(state);
	}

}