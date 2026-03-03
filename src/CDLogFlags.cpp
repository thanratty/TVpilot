#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <array>

#include "common.hpp"

#include "utils.hpp"

#include "CDLogFlags.hpp"




// I tried to make this & the declaration const but the compiler didn't like anything I tried!
extern std::array<sLogFlagDef, NUM_LOG_FLAGS>  log_flags;

// Dialog control ID of the check boxes we create
constexpr UINT32 BTN_BASE_ID = 100;

// Large anough to hold a pointer to each newly minted button
STATIC CButton* bptrs[ NUM_LOG_FLAGS ]{};




// CDLogFlags dialog

IMPLEMENT_DYNAMIC(CDLogFlags, CDialog)

CDLogFlags::CDLogFlags(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LOGGING, pParent)
{
}


CDLogFlags::~CDLogFlags()
{
	for (unsigned i = 0; i < NUM_LOG_FLAGS; i++) {
		if (bptrs[i] != nullptr) {
			delete bptrs[i];
			bptrs[i] = nullptr;
		}
	}
}


void CDLogFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDLogFlags, CDialog)
	ON_COMMAND_RANGE(BTN_BASE_ID, (BTN_BASE_ID+NUM_LOG_FLAGS-1),	&CDLogFlags::OnChkBoxClicked)
	ON_BN_CLICKED(IDC_BTN_LOG_ALL,									&CDLogFlags::OnBtnClicked_LogAll)
	ON_BN_CLICKED(IDC_BTN_LOG_NONE,									&CDLogFlags::OnBtnClicked_LogNone)
	ON_WM_SIZE()
END_MESSAGE_MAP()




static constexpr unsigned CHKBOX_HEIGHT = 30;



/**
 * Create a checkbox for each LOG option flag & position them in the dialog.
 *
 */
BOOL CDLogFlags::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Only change a local copy until 'OK' is pressed.
	m_temp_flags = GetLogFlags();


	//
	// Add the four buttons to a layout manager so they're repositioned when the dialog is resized
	//

	EnableDynamicLayout();

	auto manager = GetDynamicLayout();
	auto moveSettings = CMFCDynamicLayout::MoveHorizontalAndVertical(100,100);
	auto sizeSettings = CMFCDynamicLayout::SizeNone();

	manager->Create(this);
	manager->AddItem(IDOK, moveSettings, sizeSettings);
	manager->AddItem(IDCANCEL, moveSettings, sizeSettings);
	manager->AddItem(IDC_BTN_LOG_ALL, moveSettings, sizeSettings);
	manager->AddItem(IDC_BTN_LOG_NONE, moveSettings, sizeSettings);


	//
	// Resize the window depending on the number of checkboxes to display
	//

	CRect myWRect;
	GetWindowRect(&myWRect);
	ScreenToClient(&myWRect);

	myWRect.bottom = myWRect.top + (CHKBOX_HEIGHT*log_flags.size()) + 6*CHKBOX_HEIGHT;

	ClientToScreen(myWRect);
	SetWindowPos(&wndTop,
				 myWRect.left,    myWRect.top,
				 myWRect.Width(), myWRect.Height(), 
				 SWP_NOMOVE );


	//
	// Add the checkboxes to the client area, spaced appropriately.
	//

	CRect rect;
	GetClientRect(&rect);

	int dialog_width  = rect.Width();
	int dialog_height = rect.Height();

	int top_margin    = dialog_height / 20;
	int left_indent   = dialog_width / 4;
	int btn_width     = dialog_width / 4;


	for (unsigned btn_num = 0; btn_num < log_flags.size(); btn_num++)
	{
		// Buttons are deleted in the dialog's destructor
		bptrs[btn_num] = new CButton();
		bptrs[btn_num]->Create(	log_flags[btn_num].description, 
								WS_CHILD | WS_VISIBLE | BS_CHECKBOX, 
								CRect(left_indent, top_margin + btn_num*CHKBOX_HEIGHT, left_indent + btn_width, top_margin + (btn_num+1)*CHKBOX_HEIGHT),
								this, 
								BTN_BASE_ID + btn_num);
	}

	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
}




void CDLogFlags::OnCancel()
{
	if (m_edited)
		if (AfxMessageBox(L"Discard any changes?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
			return;

	CDialog::OnCancel();
}


void CDLogFlags::OnOK()
{
	SetLogFlags(m_temp_flags);

	CDialog::OnOK();
}




afx_msg void CDLogFlags::OnChkBoxClicked(UINT nID)
{
	unsigned btn_index = nID - BTN_BASE_ID;

	m_edited = true;

	unsigned chk_state = bptrs[btn_index]->GetCheck();
	chk_state = (chk_state == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED;
	bptrs[btn_index]->SetCheck(chk_state);

	eLogFlags mask = log_flags[btn_index].mask;

	if (chk_state == BST_CHECKED)
		m_temp_flags |= mask;
	else
		m_temp_flags &= ~mask;
}



afx_msg void CDLogFlags::OnBtnClicked_LogAll()
{
	m_temp_flags = eLogFlags::ALL;
	m_edited = true;

	UpdateUI();
}

afx_msg void CDLogFlags::OnBtnClicked_LogNone()
{
	m_temp_flags = eLogFlags::NONE;
	m_edited = true;

	UpdateUI();
}


void CDLogFlags::UpdateUI() const
{
	for (unsigned btn_num = 0; btn_num < log_flags.size(); btn_num++)
	{
		int state = flags(m_temp_flags & log_flags[btn_num].mask) ? BST_CHECKED : BST_UNCHECKED;
		bptrs[btn_num]->SetCheck(state);
	}
}




afx_msg void CDLogFlags::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	ResizeDynamicLayout();
}
