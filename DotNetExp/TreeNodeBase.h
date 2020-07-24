#pragma once

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

protected:
	TreeNodeBase(CTreeItem item);

private:
	CTreeItem _item;
};

