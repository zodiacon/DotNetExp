// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "View.h"
#include "TreeNodeBase.h"

CView::CView(IMainFrame* frame) : m_pFrame(frame), m_FilterBar(this) {
}

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
	if (m_CurrentNode && si != nullptr) {
		m_CurrentNode->SortList(si->SortColumn, si->SortAscending);
	}
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}

bool CView::IsSortable(int col) const {
	return m_CurrentNode && m_CurrentNode->CanSort(col);
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
	if (node == nullptr) {
		m_CurrentNode = nullptr;
		return;
	}
	auto sameNode = m_CurrentNode == node;

	if (m_CurrentNode && !sameNode)
		m_CurrentNode->TermList();

	if (!node->InitList())
		return;

	if (!sameNode) {
		m_pFilterBarCB = node->GetFilterBarCallback(nullptr);
		CReBarCtrl rb(m_hWndToolBar);
		rb.ShowBand(0, m_pFilterBarCB != nullptr);
		int columns = node->GetColumnCount();
		int width, format;
		for (int i = 0; i < columns; i++) {
			auto text = node->GetColumnInfo(i, width, format);
			m_List.InsertColumn(i, text, format, width);
		}
	}
	m_CurrentNode = node;
	m_List.SetItemCountEx(node->GetRowCount(), sameNode ? (LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL) : 0);
	DoSort(GetSortInfo(m_List));
}

void CView::Refresh() {
	CWaitCursor wait;
	Update(m_CurrentNode);
}

void CView::ApplyFilter(const CString& text) {
	ATLASSERT(m_pFilterBarCB);
	int count = m_pFilterBarCB->ApplyFilter(text);
	m_List.SetItemCountEx(count, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	m_FilterBar.Create(*this);
	AddSimpleReBarBand(m_FilterBar);
	CReBarCtrl rb(m_hWndToolBar);
	rb.ShowBand(0, FALSE);

	m_hWndClient = m_List.Create(*this, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		LVS_SINGLESEL | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);

	UINT icons[] = {
		IDI_PROCESSES, IDI_DB, IDI_ASSEMBLY, IDI_MODULE, IDI_TYPES, 
		IDI_ASM_DYNAMIC, IDI_FILE_DB, IDI_PROCESS, IDI_THREAD, IDI_THREAD_DEAD,
		IDI_SYNC_OPEN, IDI_SYNC_CLOSED, IDI_CLASS, IDI_STRUCT, IDI_TEXT,
		IDI_CLASS_ABSTRACT, IDI_ENUM, IDI_DELEGATE, IDI_INTERFACE, IDI_GRID,
		IDI_FREE, IDI_OBJECTS2, IDI_CONST, IDI_PROP
	};
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 16, 8);
	for (auto icon : icons) {
		images.AddIcon(AtlLoadIconImage(icon, 64, 16, 16));
	}
	m_List.SetImageList(images, LVSIL_SMALL);

	return 0;
}

LRESULT CView::OnFilter(WORD, WORD, HWND, BOOL&) {
	if (m_FilterBar.IsWindowVisible())
		m_FilterBar.SetEditFocus();
	return 0;
}

LRESULT CView::OnRightClick(int, LPNMHDR, BOOL&) {
	if (m_CurrentNode == nullptr)
		return 0;

	int index = m_List.GetSelectedIndex();
	POINT pt;
	::GetCursorPos(&pt);
	POINT pt2 = pt;
	m_List.ScreenToClient(&pt2);
	LVHITTESTINFO hti{};
	hti.pt = pt2;
	m_List.HitTest(&hti);
	auto mi = m_CurrentNode->GetListItemContextMenu(index, hti.iSubItem);
	if (mi.first) {
		CMenu menu;
		menu.LoadMenu(mi.first);
		auto cmd = (UINT)m_pFrame->ShowContextMenu(menu.GetSubMenu(mi.second), pt, TPM_RETURNCMD);
		if (cmd)
			m_CurrentNode->HandleCommand(cmd);
	}
	return 0;
}

