#include "pch.h"
#include "SyncBlocksTreeNode.h"
#include "SortHelper.h"

SyncBlocksTreeNode::SyncBlocksTreeNode(CTreeItem item, DataTarget* dt) : TreeNodeBase(item), _dt(dt) {
}

int SyncBlocksTreeNode::GetColumnCount() const {
	return 9;
}

CString SyncBlocksTreeNode::GetColumnInfo(int column, int& width, int& format) const {
	static const struct {
		PCWSTR header;
		int width;
		int format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Index", 70, LVCFMT_CENTER },
		{ L"Object", 120, LVCFMT_RIGHT },
		{ L"Type", 300 },
		{ L"Sync Block", 120, LVCFMT_RIGHT },
		{ L"Monitors Held", 90, LVCFMT_CENTER },
		{ L"Owner Thread", 120, LVCFMT_CENTER },
		{ L"Owner TID", 130, LVCFMT_RIGHT },
		{ L"Recursion", 80, LVCFMT_CENTER },
		{ L"COM Flags", 80, LVCFMT_RIGHT },
	};

	auto& info = columns[column];
	width = info.width;
	format = info.format;
	return info.header;
}

int SyncBlocksTreeNode::GetRowCount() {
	return (int)_items.size();
}

CString SyncBlocksTreeNode::GetColumnText(int row, int col) const {
	auto& item = _items[row];
	CString text;

	switch (col) {
		case 0: text.Format(L"%4u", item.Index); break;
		case 1: text.Format(L"0x%llX", item.Object); break;
		case 2: return _dt->GetObjectClassName(item.Object);
		case 3: text.Format(L"0x%llX", item.SyncBlockPointer); break;
		case 4: text.Format(L"%2u", item.MonitorHeld); break;
		case 5: 
			if (item.HoldingThread)
				text.Format(L"0x%llX", item.HoldingThread);
			break;

		case 6:
			if (item.HoldingThread) {
				auto td = _dt->GetThreadData(item.HoldingThread);
				text.Format(L"Managed ID: %u  OSID: %u", td.corThreadId, td.osThreadId);
			}
			break;
		case 7:
			if (item.HoldingThread)
				text.Format(L"%2u", item.Recursion);
			break;
		case 8: text.Format(L"0x%X", item.COMFlags); break;
		default: ATLASSERT(false);
	}
	return text;
}

bool SyncBlocksTreeNode::InitList() {
	_items = _dt->EnumSyncBlocks(false);
	return true;
}

void SyncBlocksTreeNode::TermList() {
	_items.clear();
}

void SyncBlocksTreeNode::SortList(int col, bool asc) {
	std::sort(_items.begin(), _items.end(), [&](const auto& sb1, const auto& sb2) {
		switch (col) {
			case 0: return SortHelper::SortNumbers(sb1.Index, sb2.Index, asc);
			case 1: return SortHelper::SortNumbers(sb1.Object, sb2.Object, asc);
			case 2: return SortHelper::SortStrings(_dt->GetObjectClassName(sb1.Object), _dt->GetObjectClassName(sb2.Object), asc);
			case 3: return SortHelper::SortNumbers(sb1.SyncBlockPointer, sb2.SyncBlockPointer, asc);
			case 4: return SortHelper::SortNumbers(sb1.MonitorHeld, sb2.MonitorHeld, asc);
			case 5: return SortHelper::SortNumbers(sb1.HoldingThread, sb2.HoldingThread, asc);
			case 6:
			{
				auto tid1 = sb1.HoldingThread ? _dt->GetThreadData(sb1.HoldingThread).corThreadId : 0;
				auto tid2 = sb1.HoldingThread ? _dt->GetThreadData(sb2.HoldingThread).corThreadId : 0;
				return SortHelper::SortNumbers(tid1, tid2, asc);
			}
			case 7: return SortHelper::SortNumbers(sb1.Recursion, sb2.Recursion, asc);
			case 8: return SortHelper::SortNumbers(sb1.COMFlags, sb2.COMFlags, asc);
		}
		return false;
		});
}

int SyncBlocksTreeNode::GetRowIcon(int row) const {
	return _items[row].HoldingThread == 0 ? 10 : 11;
}
