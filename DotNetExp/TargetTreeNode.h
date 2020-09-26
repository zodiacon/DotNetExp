#pragma once

#include "TreeNodeBase.h"
#include "DataTarget.h"

class TargetTreeNode : public TreeNodeBase {
public:
	TargetTreeNode(CTreeItem item, DataTarget* dt);

	// Inherited via TreeNodeBase
	virtual int GetColumnCount() const override;
	virtual CString GetColumnInfo(int column, int& width, int& format) const override;
	virtual int GetRowCount() override;
	virtual CString GetColumnText(int row, int col) const override;
	virtual bool InitList() override;
	virtual bool CanSort(int col) const override;
	virtual int GetRowIcon(int row) const override;
	virtual NodeType GetNodeType() const override {
		return NodeType::Target;
	}

	DataTarget* GetDataTarget() const {
		return _dt;
	}

private:
	DataTarget* _dt;
	std::vector<SimpleItem> _items;
};

