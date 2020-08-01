#include "pch.h"
#include "Target.h"
#include "AssembliesTreeNode.h"
#include "ThreadsTreeNode.h"
#include "SyncBlocksTreeNode.h"
#include "ModulesTreeNode.h"
#include "TypesTreeNode.h"
#include "HeapTreeNode.h"
#include "StringsTreeNode.h"
#include "TargetTreeNode.h"

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
	SetItemNode(iroot, new TargetTreeNode(iroot, dt));
	auto adItem = iroot.InsertAfter(L"AppDomains", TVI_LAST, 9);
	for (auto& ad : dt->EnumAppDomains()) {
		auto item = adItem.InsertAfter(ad.Name, TVI_ROOT, 9);
		auto asmItem = item.InsertAfter(L"Assemblies", TVI_LAST, 2);
		SetItemNode(asmItem, new AssembliesTreeNode(asmItem, dt, ad.AppDomainPtr));

		InitAssemblies(asmItem, ad);
		asmItem.SortChildren(FALSE);	
	}
	auto node = iroot.InsertAfter(L"Assemblies", TVI_LAST, 2);
	SetItemNode(node, new AssembliesTreeNode(node, dt, 0));
	node = iroot.InsertAfter(L"Modules", TVI_LAST, 3);
	SetItemNode(node, new ModulesTreeNode(node, dt, 0));
	node = iroot.InsertAfter(L"Types", TVI_LAST, 4);
	SetItemNode(node, new TypesTreeNode(node, dt, 0));
	node = iroot.InsertAfter(L"Threads", TVI_LAST, 8);
	SetItemNode(node, new ThreadsTreeNode(node, dt));
	auto gcInfo = dt->GetGCInfo();
	node = iroot.InsertAfter(gcInfo.HeapCount > 1 ? L"All Heaps" : L"Heap", TVI_LAST, 10);
	SetItemNode(node, new HeapTreeNode(node, dt, -1));
	if (gcInfo.HeapCount > 1) {
		node = iroot.InsertAfter(L"Heaps", TVI_LAST, 10);
		CString text;
		for (DWORD i = 0; i < gcInfo.HeapCount; i++) {
			text.Format(L"Heap %u", i);
			auto heapNode = node.InsertAfter(text, TVI_LAST, 13);
			SetItemNode(heapNode, new HeapTreeNode(heapNode, dt, i));
		}
	}

	auto sbNode = iroot.InsertAfter(L"Sync Blocks", TVI_LAST, 11);
	SetItemNode(sbNode, new SyncBlocksTreeNode(sbNode, dt));
	sbNode = iroot.InsertAfter(L"Strings", TVI_LAST, 14);
	SetItemNode(sbNode, new StringsTreeNode(sbNode, dt));

	root.GetTreeView()->LockWindowUpdate(FALSE);
	iroot.Select();
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
	SetItemNode(root, new ModulesTreeNode(root, dt, assem.AssemblyPtr));
	for (auto& mi : dt->EnumModules(assem.AssemblyPtr)) {
		auto node = root.InsertAfter(mi.Name, TVI_LAST, 3);
		node = node.InsertAfter(L"Types", TVI_LAST, 4);
		SetItemNode(node, new TypesTreeNode(node, dt, mi.Address));
	}
}
