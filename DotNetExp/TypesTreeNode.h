#pragma once

#include "TreeNodeBase.h"
#include "DataTarget.h"
#include "SortedFilteredVector.h"

class TypesTreeNode : public TreeNodeBase, public IFilterBarCallback {
public:
	TypesTreeNode(CTreeItem item, DataTarget* dt, CLRDATA_ADDRESS module);

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

	// Inherited via IFilterBarCallback
	virtual int ApplyFilter(const CString& text) override;

private:
	DataTarget* _dt;
	CLRDATA_ADDRESS _module;
	SortedFilteredVector<MethodTableInfo> _items;

};

