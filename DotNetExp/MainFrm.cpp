// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "AboutDlg.h"
#include "View.h"
#include "MainFrm.h"
#include "DataTarget.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	return FALSE;
}

void CMainFrame::InitTarget(Target& t) {
}

void CMainFrame::InitTree() {
	m_ProcessesNode = m_tree.InsertItem(L"Processes", TVI_ROOT, TVI_LAST);
	m_DumpsNode = m_tree.InsertItem(L"Dump Files", TVI_ROOT, TVI_LAST);
	m_ProcessesNode.Expand(TVE_EXPAND);
	m_DumpsNode.Expand(TVE_EXPAND);
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CreateSimpleStatusBar();

	m_hWndClient = m_splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_tree.Create(m_splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN 
		| TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, 
		WS_EX_CLIENTEDGE);

	m_view.Create(m_splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	m_splitter.SetSplitterPanes(m_tree, m_view);
	m_splitter.SetSplitterExtendedStyle(SPLIT_FLATBAR);

	UpdateLayout();
	m_splitter.SetSplitterPosPct(25);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	InitTree();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) const {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnAttachToProcess(WORD, WORD, HWND, BOOL&) {
	return 0;
}

LRESULT CMainFrame::OnOpenDumpFile(WORD, WORD, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, L".dmp", nullptr, OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING,
		L"Dump Files (*.dmp)\0*.dmp\0All Files\0*.*\0", *this);
	if (dlg.DoModal() == IDOK) {
		auto dt = DataTarget::FromDumpFile(dlg.m_szFileName);
		if (dt == nullptr) {
			AtlMessageBox(*this, L"Failed to open dump file", IDS_TITLE);
			return 0;
		}
		Target target(std::move(dt));
		InitTarget(target);
		m_Targets.push_back(std::move(target));
	}

	return 0;
}


