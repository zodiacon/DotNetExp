#include "pch.h"
#include "ThreadsTreeNode.h"
#include "SortHelper.h"

ThreadsTreeNode::ThreadsTreeNode(CTreeItem item, DataTarget* dt) : TreeNodeBase(item), _dt(dt) {
}

int ThreadsTreeNode::GetColumnCount() const {
	return 6;
}

CString ThreadsTreeNode::GetColumnInfo(int column, int& width, int& format) const {
	static const struct {
		PCWSTR header;
		int width;
		int format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Managed ID", 80, LVCFMT_CENTER },
		{ L"OS ID", 120, LVCFMT_RIGHT },
		{ L"State", 90, LVCFMT_RIGHT },
		{ L"Locks", 60, LVCFMT_RIGHT },
		{ L"TEB", 110, LVCFMT_RIGHT },
		{ L"Details", 400 },
	};

	auto& info = columns[column];
	width = info.width;
	format = info.format;
	return info.header;
}

int ThreadsTreeNode::GetRowCount() {
	return (int)_items.size();
}

CString ThreadsTreeNode::GetColumnText(int row, int col) const {
	auto& data = _items[row];
	CString text;

	switch (col) {
		case 0: text.Format(L"%4d", data.corThreadId); break;
		case 1:
			if (data.osThreadId == 0)
				return L"";
			text.Format(L"%u (0x%X)", data.osThreadId, data.osThreadId); 
			break;
		case 2: text.Format(L"0x%X", data.state); break;
		case 3: text.Format(L"%u", data.lockCount); break;
		case 4: text.Format(L"0x%llX", data.teb); break;
		case 5: return ThreadStateToString(data.state);
	}

	return text;
}

CString ThreadsTreeNode::ThreadStateToString(DWORD state) {
	static const struct {
		DWORD state;
		PCWSTR text;
	} states[] = {
		{0x1, L"Abort Requested"},
		{0x2, L"GC Suspend Pending"},
		{0x4, L"User Suspend Pending"},
		{0x8, L"Debug Suspend Pending"},
		{0x10, L"GC On Transitions"},
		{0x20, L"Legal to Join"},
		{0x40, L"Yield Requested"},
		{0x80, L"Hijacked by GC"},
		{0x100, L"Blocking GC for Stack Overflow"},
		{0x200, L"Background"},
		{0x400, L"Unstarted"},
		{0x800, L"Dead"},
		{0x1000, L"CLR Owns"},
		{0x2000, L"CoInitialized"},
		{0x4000, L"STA"},
		{0x8000, L"MTA"},
		{0x10000, L"Reported Dead"},
		{0x20000, L"Fully initialized"},
		{0x40000, L"Task Reset"},
		{0x80000, L"Sync Suspended"},
		{0x100000, L"Debug Will Sync"},
		{0x200000, L"Stack Crawl Needed"},
		{0x400000, L"Suspend Unstarted"},
		{0x800000, L"Aborted"},
		{0x1000000, L"Thread Pool Worker"},
		{0x2000000, L"Interruptible"},
		{0x4000000, L"Interrupted"},
		{0x8000000, L"Completion Port"},
		{0x10000000, L"Abort Initiated"},
		{0x20000000, L"Finalized"},
		{0x40000000, L"Failed to Start"},
		{0x80000000, L"Detached"},
	};

	CString text;
	for (auto& item : states)
		if (item.state & state)
			text += item.text + CString(L", ");
	if (!text.IsEmpty())
		text = text.Left(text.GetLength() - 2);
	return text;
}

bool ThreadsTreeNode::InitList() {
	_items = _dt->EnumThreads(true);
	return true;
}

void ThreadsTreeNode::TermList() {
	_items.clear();
}

void ThreadsTreeNode::SortList(int col, bool asc) {
	std::sort(_items.begin(), _items.end(), [&](const auto& t1, const auto& t2) {
		switch (col) {
			case 0: return SortHelper::SortNumbers(t1.corThreadId, t2.corThreadId, asc);
			case 1: return SortHelper::SortNumbers(t1.osThreadId, t2.osThreadId, asc);
			case 2: case 5:
				return SortHelper::SortNumbers(t1.state, t2.state, asc);
			case 3: return SortHelper::SortNumbers(t1.lockCount, t2.lockCount, asc);
			case 4: return SortHelper::SortNumbers(t1.teb, t2.teb, asc);

		}
		return false;
		});
}

int ThreadsTreeNode::GetRowIcon(int row) const {
	return _items[row].osThreadId == 0 ? 9 : 8;
}
