#pragma once

#include "TreeNodeBase.h"
#include "DataTarget.h"

class HeapTreeNode : public TreeNodeBase {
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

private:
	DataTarget* _dt;
	std::vector<HeapStatItem> _items;
	int _heap;
};

