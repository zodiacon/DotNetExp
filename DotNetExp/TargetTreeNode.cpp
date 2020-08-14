#include "pch.h"
#include "TargetTreeNode.h"

static const struct {
	PCWSTR header;
	int width;
	int format = LVCFMT_LEFT;
} columns[] = {
	{ L"Name", 150 },
	{ L"Value", 300, },
	{ L"Details", 200 }
};

static PCWSTR names[] = { 
	L"Process ID", L"Path", L"Start Time", L"GC Type", L"Heaps",
	L"Managed Threads (Live)",
};

TargetTreeNode::TargetTreeNode(CTreeItem item, DataTarget* dt) : TreeNodeBase(item), _dt(dt) {
}

int TargetTreeNode::GetColumnCount() const {
	return _countof(columns);
}

CString TargetTreeNode::GetColumnInfo(int column, int& width, int& format) const {
	auto& info = columns[column];
	width = info.width;
	format = info.format;
	return info.header;
}

int TargetTreeNode::GetRowCount() {
	return (int)_items.size();
}

CString TargetTreeNode::GetColumnText(int row, int col) const {
	auto& item = _items[row];
	switch (col) {
		case 0: return item.Name;
		case 1: return item.Value;
		case 2: return item.Detail;
	}
	ATLASSERT(false);
	return L"";
}

bool TargetTreeNode::InitList() {
	_items.clear();
	_items.reserve(_countof(names));

	{
		SimpleItem item;
		item.Name = names[0];
		item.Value.Format(L"%u (0x%X)", _dt->GetProcessId(), _dt->GetProcessId());
		_items.push_back(item);
	}
	{
		SimpleItem item;
		item.Name = names[1];
		item.Value = _dt->GetProcessPathName();
		_items.push_back(item);
	}
	{
		SimpleItem item;
		item.Name = names[2];
		item.Value = CTime(_dt->GetProcessStartTime()).Format(L"%c");
		_items.push_back(item);
	}
	{
		SimpleItem item;
		item.Name = names[3];
		item.Value = _dt->GetGCInfo().bServerMode ? L"Server" : L"Workstation";
		_items.push_back(item);
	}
	{
		SimpleItem item;
		item.Name = names[4];
		item.Value.Format(L"%u", _dt->GetGCInfo().HeapCount);
		_items.push_back(item);
	}
	{
		SimpleItem item;
		item.Name = names[5];
		auto ts = _dt->GetThreadsStats();
		item.Value.Format(L"%u (%u)", ts.threadCount, ts.threadCount - ts.deadThreadCount);
		_items.push_back(item);
	}
	return true;
}

bool TargetTreeNode::CanSort(int col) const {
	return col == 0;
}

int TargetTreeNode::GetRowIcon(int row) const {
	return 23;
}
