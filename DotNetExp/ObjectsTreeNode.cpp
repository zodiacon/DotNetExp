#include "pch.h"
#include "ObjectsTreeNode.h"

ObjectsTreeNode::ObjectsTreeNode(CTreeItem item, DataTarget* dt, CLRDATA_ADDRESS mt) : TreeNodeBase(item), _dt(dt), _mt(mt) {
}

int ObjectsTreeNode::GetColumnCount() const {
    return 0;
}

CString ObjectsTreeNode::GetColumnInfo(int column, int& width, int& format) const {
    return CString();
}

int ObjectsTreeNode::GetRowCount() {
    return 0;
}

CString ObjectsTreeNode::GetColumnText(int row, int col) const {
    return CString();
}

bool ObjectsTreeNode::InitList() {
    return false;
}
