#include "pch.h"
#include "AssembliesTreeNode.h"
#include "SortHelper.h"

AssembliesTreeNode::AssembliesTreeNode(CTreeItem item, DataTarget* dt, DWORD64 ad)
	: TreeNodeBase(item), _dt(dt), _ad(ad) {
}

int AssembliesTreeNode::GetColumnCount() const {
	return 4;
}

CString AssembliesTreeNode::GetColumnInfo(int column, int& width, int& format) const {
	static const struct {
		PCWSTR header;
		int width;
		int format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Name", 350 },
		{ L"Address", 110, LVCFMT_RIGHT },
		{ L"AppDomain", 110, LVCFMT_RIGHT },
		{ L"Dynamic?", 60 },
	};

	auto& info = columns[column];
	width = info.width;
	format = info.format;
	return info.header;
}

int AssembliesTreeNode::GetRowCount() {
	return (int)_assemblies.size();
}

CString AssembliesTreeNode::GetColumnText(int row, int col) const {
	auto& item = _assemblies[row];
	CString text;

	switch (col) {
		case 0: return item.Name;
		case 1: text.Format(L"0x%llX", item.AssemblyPtr); break;
		case 2: text.Format(L"0x%llX", item.ParentDomain); break;
		case 3: return item.isDynamic ? L"Yes" : L"";
	}

	return text;
}

int AssembliesTreeNode::GetRowIcon(int row) const {
	return _assemblies[row].isDynamic ? 5 : 2;
}

bool AssembliesTreeNode::InitList() {
	_assemblies = _dt->EnumAssemblies(_ad);
	return true;
}

void AssembliesTreeNode::TermList() {
	_assemblies.clear();
}

void AssembliesTreeNode::SortList(int col, bool asc) {
	std::sort(_assemblies.begin(), _assemblies.end(), [&](auto& item1, auto& item2) {
		switch (col) {
			case 0: return SortHelper::SortStrings(item1.Name, item2.Name, asc);
			case 1: return SortHelper::SortNumbers(item1.AssemblyPtr, item2.AssemblyPtr, asc);
			case 2: return SortHelper::SortNumbers(item1.ParentDomain, item2.ParentDomain, asc);
			case 3: return SortHelper::SortNumbers(item1.isDynamic, item2.isDynamic, asc);
		}
		return false;
		});
}
