#include "pch.h"
#include "TreeNodeBase.h"

TreeNodeBase::~TreeNodeBase() = default;

TreeNodeBase::TreeNodeBase(CTreeItem item) : _item(item) {
}
