#pragma once

#include "Interfaces.h"

struct SimpleItem {
	CString Name, Value, Detail;
	int Icon{ 0 };
};

class TreeNodeBase abstract {
public:
	virtual ~TreeNodeBase();

	CTreeItem GetTreeItem() const {
		return _item;
	}

	virtual int GetColumnCount() const = 0;
	virtual CString GetColumnInfo(int column, int& width, int& format) const = 0;
	virtual int GetRowCount() = 0;
	virtual CString GetColumnText(int row, int col) const = 0;
	virtual int GetRowIcon(int row) const {
		return -1;
	}
	virtual bool InitList() = 0;
	virtual void TermList() {}
	virtual void SortList(int col, bool asc) {}
	virtual bool CanSort(int col) const {
		return true;
	}
	virtual IFilterBarCallback* GetFilterBarCallback(IFilterBar* fb) {
		return nullptr;
	}
	virtual std::pair<UINT, int> GetListItemContextMenu(int selectedItem) {
		return { 0, 0 };
	}
	virtual void HandleCommand(UINT cmd) {}
	virtual std::pair<UINT, int> GetTreeItemContextMenu() {
		return { 0, 0 };
	}

protected:
	TreeNodeBase(CTreeItem item);

private:
	CTreeItem _item;
};

