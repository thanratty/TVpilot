#pragma once
#include "config.h"

#include "CDShows.h"
#include "CDSchedule.h"
#include "CDArchive.h"
#include "CDmessages.h"
#include "model.hpp"
#include "utils.hpp"



// Which tab is which
//
static constexpr int TAB_NUM_SHOWS    = 0;
static constexpr int TAB_NUM_SCHEDULE = 1;
static constexpr int TAB_NUM_ARCHIVE  = 2;







// CepcheckDlg dialog
class CepcheckDlg : public CDialog
{
// Construction
public:
	CepcheckDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EPCHECK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	virtual BOOL    OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()

	// Windows Message Handlers
	afx_msg void	OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnDownloadComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadPing(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnZoomEpisodes(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLaunchUrl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSignalAppEvent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAbortDownload(WPARAM wParam, LPARAM lParam);

public:
	// Dialog control event handlers
	afx_msg void	OnBtnClickedLoad();
	afx_msg void	OnBtnClickedSave();
	afx_msg void	OnBtnClickedDownload();
	afx_msg void	OnBtnClickedDeleteShow();
	afx_msg void	OnBtnClickedAddShow();
	afx_msg void	OnBtnClickedMsgLog();
	afx_msg void	OnBtnClickedBreak();
	afx_msg void    OnBtnClickedChkMissedOnly();
	afx_msg void	OnBtnClickedResetDays();
	afx_msg void	OnBtnClickedToday();
	afx_msg void	OnBtnClickedExplorer();
	afx_msg void	OnDeltaPosSpinDays(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnTimer(UINT_PTR nIDEvent);


private:
			CSpinButtonCtrl m_spin_post;
			CSpinButtonCtrl m_spin_pre;
			CTabCtrl		m_tabctrl;
			CFont			m_headerFont;
			//
			CDShows			m_dlgShows;
			CDSchedule		m_dlgSchedule;
			CDArchive		m_dlgArchive;
			CDmessages		m_dlgMessages;
			model			m_data;
			//
			unsigned		m_ping_count{ 0 };
			unsigned		m_err_count{ 0 };
			int				m_spin_pre_val{ DEFAULT_DAYS_PRE };
			int				m_spin_post_val{ DEFAULT_DAYS_POST };
			BOOL			m_missed_only{ FALSE };
			bool			m_abort_download{ false };

	virtual void		OnOK();
	virtual void		OnCancel();
			void		UpdateTabTotals();
			void		UpdateOnscreenCounters();

			void		UpdateShowList();
			void		UpdateScheduleList();
			void		UpdateArchiveList();
			void		UpdateSchedulePeriod();
};
