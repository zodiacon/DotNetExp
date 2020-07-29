#include "pch.h"
#include "StringsTreeNode.h"
#include "SortHelper.h"

static const struct {
	PCWSTR header;
	int width;
	int format = LVCFMT_LEFT;
} columns[] = {
	{ L"Address", 120, LVCFMT_RIGHT },
	{ L"Length", 80, LVCFMT_RIGHT },
	{ L"Value", 450 },
};

StringsTreeNode::StringsTreeNode(CTreeItem item, DataTarget* dt) : TreeNodeBase(item), _dt(dt) {
}

int StringsTreeNode::GetColumnCount() const {
	return _countof(columns);
}

CString StringsTreeNode::GetColumnInfo(int column, int& width, int& format) const {
	auto& info = columns[column];
	width = info.width;
	format = info.format;
	return info.header;
}

int StringsTreeNode::GetRowCount() {
	return (int)_items.FilteredSize();
}

CString StringsTreeNode::GetColumnText(int row, int col) const {
	auto& item = _items[row];
	CString text;

	switch (col) {
		case 0: text.Format(L"0x%llX", item.Address); break;
		case 1: text.Format(L"%llu", (item.Size - 3 * sizeof(PVOID)) / sizeof(WCHAR) - 1); break;
		case 2: return item.StringValue;
	}
	return text;
}

bool StringsTreeNode::InitList() {
	CWaitCursor wait;
	_items.clear();
	_items.reserve(4096);
	_dt->EnumObjects([&](auto& info) {
		if (info.ObjectType == OBJ_STRING) {
			info.StringValue = _dt->GetObjectString(info.Address);
			_items.push_back(info);
		}
		return true;
		});
	return true;
}

void StringsTreeNode::TermList() {
	_items.clear();
}

void StringsTreeNode::SortList(int col, bool asc) {
	_items.Sort([&](auto& t1, auto& t2) {
		switch (col) {
			case 0: return SortHelper::SortNumbers(t1.Address, t2.Address, asc);
			case 1: return SortHelper::SortNumbers(t1.Size, t2.Size, asc);
			case 2: return SortHelper::SortStrings(t1.StringValue, t2.StringValue, asc);
		}
		return false;
		});
}

int StringsTreeNode::GetRowIcon(int row) const {
	return 14;
}

IFilterBarCallback* StringsTreeNode::GetFilterBarCallback(IFilterBar* fb) {
	return this;
}

int StringsTreeNode::ApplyFilter(const CString& text) {
	if (text.IsEmpty())
		_items.Filter(nullptr);
	else {
		CString ltext(text);
		ltext.MakeLower();
		_items.Filter([&](auto& info) -> bool {
			CString copy(info.StringValue);
			copy.MakeLower();
			return copy.Find(ltext) >= 0;
			});
	}
	return (int)_items.FilteredSize();
}
