#pragma once

#include "DataTarget.h"

class TreeNodeBase;

class Target {
public:
	Target(std::unique_ptr<DataTarget> dt);
	Target(Target&&) = default;
	Target& operator=(Target&&) = default;
	~Target();

	DataTarget* GetDataTarget();

	CTreeItem Init(PCWSTR name, CTreeItem root);

private:
	void SetItemNode(CTreeItem item, TreeNodeBase* node);
	void InitAssemblies(CTreeItem root, const AppDomainInfo& ad);
	void InitModules(CTreeItem root, const AssemblyInfo& assem);

private:
	std::unique_ptr<DataTarget> _dataTarget;
};

