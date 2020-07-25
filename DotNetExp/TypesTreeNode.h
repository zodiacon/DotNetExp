#pragma once
#include "TreeNodeBase.h"
#include "DataTarget.h"

class TypesTreeNode : public TreeNodeBase {
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

private:
	DataTarget* _dt;
	CLRDATA_ADDRESS _module;
	std::vector<MethodTableInfo> _items;
};

