#pragma once

#include "TreeNodeBase.h"
#include "DataTarget.h"
#include "SortedFilteredVector.h"

class HeapTreeNode : public TreeNodeBase, public IFilterBarCallback {
public:
	HeapTreeNode(CTreeItem item, DataTarget* dt, int heap);

	// Inherited via TreeNodeBase
	virtual int GetColumnCount() const override;
	virtual CString GetColumnInfo(int column, int& width, int& format) const override;
	virtual int GetRowCount() override;
	virtual CString GetColumnText(int row, int col) const override;
	virtual bool InitList() override;
	virtual void TermList() override;
	virtual void SortList(int col, bool asc) override;
	virtual int GetRowIcon(int row) const override;
	virtual IFilterBarCallback* GetFilterBarCallback(IFilterBar* fb);
	virtual std::pair<UINT, int> GetListItemContextMenu(int selectedItem);
	virtual void HandleCommand(UINT cmd) override;

	// Inherited via IFilterBarCallback
	virtual int ApplyFilter(const CString& text) override;

private:
	DataTarget* _dt;
	SortedFilteredVector<HeapStatItem> _items;
	int _heap;
	int _selected;
};

