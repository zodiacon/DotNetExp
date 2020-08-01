#pragma once
#include "TreeNodeBase.h"
#include "DataTarget.h"

class ObjectsTreeNode : public TreeNodeBase {
public:
	ObjectsTreeNode(CTreeItem item, DataTarget* dt, CLRDATA_ADDRESS mt);

	// Inherited via TreeNodeBase
	virtual int GetColumnCount() const override;
	virtual CString GetColumnInfo(int column, int& width, int& format) const override;
	virtual int GetRowCount() override;
	virtual CString GetColumnText(int row, int col) const override;
	virtual bool InitList() override;
	virtual std::pair<UINT, int> GetTreeItemContextMenu() override;
	virtual void HandleCommand(UINT cmd) override;

private:
	static int GetFormatFromType(CorElementType type);
	CString GetFieldValue(const ObjectInfo& info, int index) const;

private:
	DataTarget* _dt;
	CLRDATA_ADDRESS _mt;
	std::vector<ObjectInfo> _items;
	std::vector<FieldInfo> _fields;
};

