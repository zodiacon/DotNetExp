#include "pch.h"
#include "ModulesTreeNode.h"
#include "SortHelper.h"

static const struct {
	PCWSTR header;
	int width;
	int format = LVCFMT_LEFT;
} columns[] = {
	{ L"Name", 230 },
	{ L"Address", 120, LVCFMT_RIGHT },
	{ L"Assembly", 110, LVCFMT_RIGHT },
	{ L"File Name", 350 },
};

ModulesTreeNode::ModulesTreeNode(CTreeItem item, DataTarget* dt, CLRDATA_ADDRESS assembly) 
	: TreeNodeBase(item), _dt(dt), _assembly(assembly) {
}

int ModulesTreeNode::GetColumnCount() const {
	return _countof(columns);
}

CString ModulesTreeNode::GetColumnInfo(int column, int& width, int& format) const {
	auto& info = columns[column];
	width = info.width;
	format = info.format;
	return info.header;
}

int ModulesTreeNode::GetRowCount() {
	return (int)_items.size();
}

CString ModulesTreeNode::GetColumnText(int row, int col) const {
	auto& item = _items[row];
	CString text;

	switch (col) {
		case 0: return item.Name;
		case 1: text.Format(L"0x%llX", item.Address); break;
		case 2: text.Format(L"0x%llX", item.Assembly); break;
		case 3: return item.FileName;
	}
	return text;
}

bool ModulesTreeNode::InitList() {
	_items = _assembly == 0 ? _dt->EnumModules() : _dt->EnumModules(_assembly);
	return true;
}

void ModulesTreeNode::TermList() {
	_items.clear();
}

void ModulesTreeNode::SortList(int col, bool asc) {
	std::sort(_items.begin(), _items.end(), [&](const auto& m1, const auto& m2) {
		switch (col) {
			case 0: return SortHelper::SortStrings(m1.Name, m2.Name, asc);
			case 1: return SortHelper::SortNumbers(m1.Address, m2.Address, asc);
			case 2: return SortHelper::SortNumbers(m1.Assembly, m2.Assembly, asc);
			case 3: return SortHelper::SortStrings(m1.FileName, m2.FileName, asc);
		}
		return false;
		});
}

int ModulesTreeNode::GetRowIcon(int row) const {
	return 3;
}
