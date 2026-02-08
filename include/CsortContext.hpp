#pragma once


#include <unordered_map>
#include <vector>

#include "utils.hpp"




// Any sortable column in a CListCtrl has a type-appropriate compare function to order/sort it.
typedef int	(*COMPARE_FUNC)(const CString& item1, const CString& item2, bool ascending);


// Assosciates a column with a suitable sort function
typedef struct sortmapentry_tag {
	int				col_num;		// List control col to sort on (may be a hidden col)
	COMPARE_FUNC	fnCompare;		// Compare function to use for this column
} sSortMapEntry;


// Each clickable sort column in a list control has a vector of one or more columns used for the actual sort.
typedef std::unordered_map<int, std::vector<sSortMapEntry>> tSortMap;








/**
 * Used in the Show & Schedule dialogs to control sort order when a CListCtrl column header is clicked
 *
 */
class cSortContext
{

private:

	int					m_sort_col;
	bool				m_sort_ascending{ true };
	const tSortMap*		m_sort_map;
	CListCtrl*			m_list_control;


public:

	/**
	 * The constructor saves the assosciated CListCtrl and its sort map
	 */
	cSortContext( const tSortMap* sm, CListCtrl* lc) 
		: m_sort_map( sm ),
		  m_list_control( lc )
	{
		// Set the initial sort column to the first entry in the sort map
		m_sort_col = m_sort_map->begin()->first;
	}



	/**
	 * Called when a column header is clicked.
	 * If we clicked on the same column, invert the column's sort order.
	 * If we clicked on a different column, sort on the new column ascending.
	 * Return true if the sort column # or order changed
	 */
	bool SetSortColumn(int column)
	{
		// Not a column we can sort on? Just return.
		if (m_sort_map->find(column) == m_sort_map->end())
			return false;

		if (column == m_sort_col)
		{
			// Same column clicked - invert sort order
			m_sort_ascending ^= true;
		}
		else
		{
			// New column. Sort ascending.
			m_sort_ascending = true;
			m_sort_col = column;
		}

		// Indicate column or sort order has changed
		return true;
	}


	void SortList()
	{
		m_list_control->SortItemsEx(ListSortFunc, reinterpret_cast<LPARAM>(this));
	}



private:

	inline CListCtrl* GetListControl() const
	{
		return m_list_control;
	}


	// Will always find a map entry as it has been previously validated in SetSortColumn()
	inline const std::vector<sSortMapEntry>& GetColumnSortMap() const
	{
		return m_sort_map->find(m_sort_col)->second;
	}


	/**
	 * Sort two entries in a CListCtrl
	 * 
	 */
	static int CALLBACK ListSortFunc(LPARAM index1,		// Item 1 list index
									 LPARAM index2,		// Item 2 list index
									 LPARAM context)	// ptr to an instance of this class
	{
		cSortContext const * const sort_context = reinterpret_cast<cSortContext*>(context);

		const std::vector<sSortMapEntry>& SortMap = sort_context->GetColumnSortMap();
		bool ascending  = sort_context->m_sort_ascending;

		// Compare the two list entries by each column in the sort list till one entry is 'bigger' than the other
		for (const sSortMapEntry& sme : SortMap)
		{
			const CString item1 = sort_context->GetListControl()->GetItemText(index1, sme.col_num);
			const CString item2 = sort_context->GetListControl()->GetItemText(index2, sme.col_num);

			int sort_result = sme.fnCompare(item1, item2, ascending);
			if (sort_result != 0)
				return sort_result;
		}

		// If we drop out the bottom then the two entries were equal for all comparisons
		return 0;
	}

};


