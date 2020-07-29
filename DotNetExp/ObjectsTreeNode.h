#pragma once
#include "TreeNodeBase.h"
#include "DataTarget.h"
#include "SortedFilteredVector.h"

class ObjectsTreeNode : public TreeNodeBase {
public:
	ObjectsTreeNode(CTreeItem item, DataTarget* dt, CLRDATA_ADDRESS mt);

	// Inherited via TreeNodeBase
	virtual int GetColumnCount() const override;
	virtual CString GetColumnInfo(int column, int& width, int& format) const override;
	virtual int GetRowCount() override;
	virtual CString GetColumnText(int row, int col) const override;
	virtual bool InitList() override;

private:
	DataTarget* _dt;
	CLRDATA_ADDRESS _mt;
	SortedFilteredVector<ObjectInfo> _items;
};

