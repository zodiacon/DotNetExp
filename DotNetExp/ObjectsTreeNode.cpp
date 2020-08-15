#include "pch.h"
#include "ObjectsTreeNode.h"
#include "SortHelper.h"
#include <algorithm>
#include "resource.h"

static const struct {
	PCWSTR header;
	int width;
	int format = LVCFMT_LEFT;
} columns[] = {
	{ L"Address", 120 },
	{ L"Size", 60, LVCFMT_RIGHT },
};

static const struct {
	PCWSTR header;
	int width;
	int format = LVCFMT_RIGHT;
} arrayColumns[] = {
	{ L"Rank", 50 },
	{ L"Length", 80 },
};

ObjectsTreeNode::ObjectsTreeNode(CTreeItem item, DataTarget* dt, CLRDATA_ADDRESS mt) : TreeNodeBase(item), _dt(dt), _mt(mt) {
}

int ObjectsTreeNode::GetColumnCount() const {
	return _countof(columns) + (int)_fields.size() + (_isArray ? _countof(arrayColumns) : 0);
}

CString ObjectsTreeNode::GetColumnInfo(int column, int& width, int& format) const {
	if (column < _countof(columns)) {
		auto& info = columns[column];
		width = info.width;
		format = info.format;
		return info.header;
	}

	if (_isArray && column < _countof(columns) + _countof(arrayColumns)) {
		auto& info = arrayColumns[column - _countof(columns)];
		width = info.width;
		format = info.format;
		return info.header;
	}

	width = 100;
	auto& field = _fields[column - _countof(columns) - (_isArray ? 1 : 0)];
	format = GetFormatFromType(field.Type);
	return field.Name + L"(" + _dt->GetMethodTableInfo(field.MTOfType).Name + L")";
}

int ObjectsTreeNode::GetRowCount() {
	return (int)_items.size();
}

CString ObjectsTreeNode::GetColumnText(int row, int col) const {
	auto& item = _items[row];
	CString text;

	switch (col) {
		case 0: text.Format(L"0x%llX", item.Address); break;
		case 1: text.Format(L"%llu", item.Size); break;
		case 2:
			if (_isArray) {
				text.Format(L"%u", item.dwRank);
				break;
			}
			__fallthrough;
		case 3:
			if (_isArray) {
				text.Format(L"%llu", item.dwNumComponents);
				break;
			}

			__fallthrough;
		default:
			return GetFieldValue(item, col - _countof(columns) - (_isArray ? _countof(arrayColumns) : 0));
	}

	return text;
}

bool ObjectsTreeNode::InitList() {
	CWaitCursor wait;
	_items.clear();
	_items.reserve(256);

	_isArray = _dt->GetMethodTableInfo(_mt).BaseName == L"System.Array";
	
	_dt->EnumObjects([&](auto& obj) {
		if (obj.MethodTable == _mt)
			_items.push_back(obj);
		return true;
		});

	if (!_items.empty()) {
		_fields = _dt->GetFieldsOfType(_items[0].MethodTable, GetFieldsFlags::Instance | GetFieldsFlags::Inherited | GetFieldsFlags::CompilerGenerated);
	}

	return true;
}

std::pair<UINT, int> ObjectsTreeNode::GetTreeItemContextMenu() {
	return { IDR_CONTEXT, 1 };
}

void ObjectsTreeNode::HandleCommand(UINT cmd) {
	switch (cmd) {
		case ID_OBJECTNODE_CLOSE:
			GetTreeItem().Delete();
			break;
	}
}

void ObjectsTreeNode::SortList(int col, bool asc) {
	std::sort(_items.begin(), _items.end(), [&](const auto& i1, const auto& i2) {
		switch (col) {
			case 0: return SortHelper::SortNumbers(i1.Address, i2.Address, asc);
			case 1: return SortHelper::SortNumbers(i1.Size, i2.Size, asc);
			case 2: return SortHelper::SortNumbers(i1.dwRank, i2.dwRank, asc);
			case 3: return SortHelper::SortNumbers(i1.dwNumComponents, i2.dwNumComponents, asc);
		}
		return false;
		});
}

bool ObjectsTreeNode::CanSort(int col) const {
	return col < 2 + (_isArray ? _countof(arrayColumns) : 0);
}

int ObjectsTreeNode::GetFormatFromType(CorElementType type) {
	switch (type) {
		case ELEMENT_TYPE_I1:
		case ELEMENT_TYPE_U1:
		case ELEMENT_TYPE_I2:
		case ELEMENT_TYPE_U2:
		case ELEMENT_TYPE_I4:
		case ELEMENT_TYPE_U4:
		case ELEMENT_TYPE_I8:
		case ELEMENT_TYPE_U8:
		case ELEMENT_TYPE_R4:
		case ELEMENT_TYPE_R8:
		case ELEMENT_TYPE_U:
		case ELEMENT_TYPE_I:
			return LVCFMT_RIGHT;
	}
	return LVCFMT_LEFT;
}

CString ObjectsTreeNode::GetFieldValue(const ObjectInfo& info, int index) const {
	CString result;

	auto& field = _fields[index];
	ATLASSERT(!field.bIsContextLocal && !field.bIsStatic && !field.bIsThreadLocal);
	// instance fields only
	auto addr = field.dwOffset + sizeof(PVOID) + info.Address;
	switch (field.Type) {
		case ELEMENT_TYPE_I4:
		{
			auto value = _dt->ReadVirtual<int>(addr);
			result.Format(L"%d (0x%X)", value, value);
			break;
		}
		case ELEMENT_TYPE_I1:
		{
			auto value = _dt->ReadVirtual<int8_t>(addr);
			result.Format(L"%d (0x%X)", (int)value, (int)value);
			break;
		}
		case ELEMENT_TYPE_U1:
		{
			auto value = _dt->ReadVirtual<uint8_t>(addr);
			result.Format(L"%d (0x%X)", (unsigned)value, (unsigned)value);
			break;
		}
		case ELEMENT_TYPE_I2:
		{
			auto value = _dt->ReadVirtual<int16_t>(addr);
			result.Format(L"%d (0x%X)", (int)value, (int)value);
			break;
		}
		case ELEMENT_TYPE_U2:
		{
			auto value = _dt->ReadVirtual<uint16_t>(addr);
			result.Format(L"%d (0x%X)", (unsigned)value, (unsigned)value);
			break;
		}
		case ELEMENT_TYPE_I8:
		{
			auto value = _dt->ReadVirtual<long long>(addr);
			result.Format(L"%lld (0x%llX)", value, value);
			break;
		}
		case ELEMENT_TYPE_U4:
		{
			auto value = _dt->ReadVirtual<DWORD>(addr);
			result.Format(L"%u (0x%X)", value, value);
			break;
		}
		case ELEMENT_TYPE_U8:
		{
			auto value = _dt->ReadVirtual<unsigned long long>(addr);
			result.Format(L"%llu (0x%llX)", value, value);
			break;
		}
		case ELEMENT_TYPE_R4:
		{
			auto value = _dt->ReadVirtual<float>(addr);
			result.Format(L"%f", value);
			break;
		}
		case ELEMENT_TYPE_R8:
		{
			auto value = _dt->ReadVirtual<double>(addr);
			result.Format(L"%lf", value);
			break;
		}
		case ELEMENT_TYPE_I:
		{
			auto value = sizeof(PVOID) == 4 ? _dt->ReadVirtual<int>(addr) : _dt->ReadVirtual<int64_t>(addr);
			result.Format(sizeof(PVOID) == 4 ? L"0x%X" : L"0x%llX", value);
			break;
		}
		case ELEMENT_TYPE_U:
		case ELEMENT_TYPE_PTR:
		{
			auto value = sizeof(PVOID) == 4 ? _dt->ReadVirtual<DWORD>(addr) : _dt->ReadVirtual<DWORD64>(addr);
			result.Format(sizeof(PVOID) == 4 ? L"0x%X" : L"0x%llX", value);
			break;
		}
		case ELEMENT_TYPE_CHAR:
		{
			auto value = _dt->ReadVirtual<WCHAR>(addr);
			result.Format(L"%d ('%c')", (int)value, iswprint(value) ? value : L'.');
			break;
		}
		case ELEMENT_TYPE_BOOLEAN:
		{
			auto value = _dt->ReadVirtual<bool>(addr);
			result = value ? L"True" : L"False";
			break;
		}
		case ELEMENT_TYPE_STRING:
		{
			caseString:
			auto address = _dt->ReadVirtual<DWORD64>(addr);
			if (address == 0)
				return L"(null)";
			WCHAR value[128];
			auto len = _dt->ReadVirtual<DWORD>(address + sizeof(PVOID));
			if (len >= _countof(value))
				len = _countof(value) - 1;
			_dt->ReadVirtualBuffer(address + sizeof(PVOID) + sizeof(DWORD), value, (len + 1) * 2);
			result.Format(L"0x%llX \"%s\"", address, value);
			break;
		}
		case ELEMENT_TYPE_CLASS:
		{
			if (_dt->IsStringType(field.MTOfType))
				goto caseString;
			auto value = _dt->ReadVirtual<PVOID>(addr);
			if (value == 0)
				return L"(null)";
			result.Format(L"0x%p", value);
			break;
		}
		case ELEMENT_TYPE_SZARRAY:
		case ELEMENT_TYPE_ARRAY:
		{
			int zz = 9;
			break;
		}
		case ELEMENT_TYPE_VALUETYPE:
		{
			result.Format(L"0x%llX (VT)", addr);
			break;
		}
		default:
			result.Format(L"Type: %d", field.Type);
			break;
	}

	return result;
}
