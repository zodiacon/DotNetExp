#include "pch.h"
#include "Target.h"
#include "AssembliesTreeNode.h"

Target::Target(std::unique_ptr<DataTarget> dt) : _dataTarget(std::move(dt)) {
}

Target::~Target() = default;

DataTarget* Target::GetDataTarget() {
	return _dataTarget.get();
}

CTreeItem Target::Init(PCWSTR name, CTreeItem root) {
	root.GetTreeView()->LockWindowUpdate();
	auto dt = GetDataTarget();
	auto iroot = root.InsertAfter(name, TVI_LAST, dt->IsDump() ? 6 : 7);
	auto adItem = iroot.InsertAfter(L"AppDomains", TVI_LAST, 9);
	for (auto& ad : dt->EnumAppDomains()) {
		auto item = adItem.InsertAfter(ad.Name, TVI_ROOT, 9);
		auto asmItem = item.InsertAfter(L"Assemblies", TVI_LAST, 2);
		SetItemNode(asmItem, new AssembliesTreeNode(asmItem, dt, ad.AppDomainPtr));

		InitAssemblies(asmItem, ad);
		asmItem.SortChildren(FALSE);	
	}
	auto node = iroot.InsertAfter(L"Threads", TVI_LAST, 8);
	auto heapsNode = iroot.InsertAfter(L"Heaps", TVI_LAST, 10);
	auto sbNode = iroot.InsertAfter(L"Sync Blocks", TVI_LAST, -1);

	root.GetTreeView()->LockWindowUpdate(FALSE);
	return iroot;
}

void Target::SetItemNode(CTreeItem item, TreeNodeBase* node) {
	item.SetData(reinterpret_cast<DWORD_PTR>(node));
}

void Target::InitAssemblies(CTreeItem root, const AppDomainInfo& ad) {
	auto dt = GetDataTarget();
	for (auto& assem : dt->EnumAssemblies(ad.AppDomainPtr)) {
		auto name = assem.Name;
		if (name.IsEmpty()) {
			ATLASSERT(assem.isDynamic);
			name.Format(L"(Dynamic) 0x%llX", assem.AssemblyPtr);
		}
		else {
			auto index = name.ReverseFind(L'\\');
			if (index >= 0)
				name = name.Mid(index + 1);
		}
		auto node = root.InsertAfter(name, TVI_LAST, assem.isDynamic ? 5 : 2);
		InitModules(node, assem);
	}
}

void Target::InitModules(CTreeItem root, const AssemblyInfo& assem) {
	auto dt = GetDataTarget();
	root = root.InsertAfter(L"Modules", TVI_LAST, 3);
	for (auto& mi : dt->EnumModules(assem.AssemblyPtr)) {
		auto node = root.InsertAfter(mi.Name, TVI_LAST, 3);
		node.InsertAfter(L"Types", TVI_LAST, 4);
	}
}