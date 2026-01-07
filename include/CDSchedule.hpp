#pragma once

#include "CsortContext.hpp"



// A sensible default os +/- two days to display
//


constexpr unsigned DEFAULT_DAYS_PRE  = 2;
constexpr unsigned DEFAULT_DAYS_POST = 2;



// CDSchedule dialog

class CDSchedule : public CDialog, public cSortContext
{
		DECLARE_DYNAMIC(CDSchedule)

public:
		CDSchedule(CWnd* pParent = nullptr);
		virtual ~CDSchedule();

// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_SCHEDULE };
#endif

protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		afx_msg LRESULT OnSchedEpFlagsChanged(WPARAM wParam, LPARAM );

		DECLARE_MESSAGE_MAP()
public:
		virtual BOOL OnInitDialog();
		virtual BOOL PreTranslateMessage(MSG* pMsg);

		afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnDblclkSchedList(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnCustomdrawSchedList(NMHDR* pNMHDR, LRESULT* pResult);

		void AppendRow(const sScheduleListEntry* gle);
		CString GetEpisodeTitle(int index) const;
		CString GetEpisodeNumber(int index) const;
		CString GetEpisodeShow(int index) const;

		inline void DeleteAllItems()
		{
			m_schedlist.DeleteAllItems();
		}

		inline int GetSelectedItem() const
		{
			return m_schedlist.GetNextItem(-1, LVNI_SELECTED);
		}

		void SetSelectedItem(int nItem)
		{
			m_schedlist.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
			m_schedlist.SetSelectionMark(nItem);
		}

		inline int GetTopIndex() const
		{
			return m_schedlist.GetTopIndex();
		}

		void SetTopIndex(int nIndex)
		{
			m_schedlist.EnsureVisible(m_schedlist.GetItemCount() - 1, TRUE);		// Scroll down to the bottom
			m_schedlist.EnsureVisible(nIndex, FALSE);								// Scroll back up just enough to show desired item on top
		}


private:
		virtual void OnOK();


private:
		CListCtrl				 m_schedlist;
		static const tSortMap	 m_sort_map;
};



