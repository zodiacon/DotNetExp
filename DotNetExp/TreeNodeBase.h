#pragma once

class TreeNodeBase abstract {
public:
	virtual ~TreeNodeBase() = default;

	CTreeItem GetTreeItem() const {
		return _item;
	}

protected:
	TreeNodeBase(CTreeItem item);

	virtual CString GetColumnHeader() const = 0;
	virtual int GetRowCount() = 0;
	virtual CString GetColumnText(int row, int col) const = 0;
	
private:
	CTreeItem _item;
};

