// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "View.h"
#include "TreeNodeBase.h"

BOOL CView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

CString CView::GetColumnText(HWND, int row, int col) const {
	return m_CurrentNode ? m_CurrentNode->GetColumnText(row, col) : L"";
}

int CView::GetRowImage(int row) const {
	return m_CurrentNode->GetRowIcon(row);
}

void CView::DoSort(const SortInfo* si) {
	if (m_CurrentNode) {
		m_CurrentNode->SortList(si->SortColumn, si->SortAscending);
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}
}

void CView::Reset() {
	m_List.LockWindowUpdate();
	m_List.SetItemCount(0);
	while (m_List.DeleteColumn(0))
		;
	m_List.LockWindowUpdate(FALSE);
	ClearSort(m_List);
}

void CView::Update(TreeNodeBase* node) {
	if (m_CurrentNode)
		m_CurrentNode->TermList();

	if (!node->InitList())
		return;

	int columns = node->GetColumnCount();
	int width, format;
	for (int i = 0; i < columns; i++) {
		auto text = node->GetColumnInfo(i, width, format);
		m_List.InsertColumn(i, text, format, width);
	}

	m_CurrentNode = node;
	m_List.SetItemCount(node->GetRowCount());
}

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		LVS_SINGLESEL | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);

	UINT icons[] = {
		IDI_PROCESSES, IDI_DB, IDI_ASSEMBLY, IDI_MODULE, IDI_TYPES, 
		IDI_ASM_DYNAMIC, IDI_FILE_DB, IDI_PROCESS, IDI_THREAD, IDI_THREAD_DEAD,
		IDI_SYNC_OPEN, IDI_SYNC_CLOSED, IDI_CLASS, IDI_STRUCT, IDI_TEXT
	};
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 16, 8);
	for (auto icon : icons) {
		images.AddIcon(AtlLoadIconImage(icon, 64, 16, 16));
	}
	m_List.SetImageList(images, LVSIL_SMALL);

	return 0;
}
