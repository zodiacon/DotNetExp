// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"

#include "AboutDlg.h"
#include "View.h"
#include "MainFrm.h"
#include "DataTarget.h"
#include "ProcessSelectDlg.h"
#include "TreeNodeBase.h"

CMainFrame::CMainFrame() : m_view(this) {
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	return FALSE;
}

void CMainFrame::InitTree() {
	m_ProcessesNode = m_tree.InsertItem(L"Processes", 0, 0, TVI_ROOT, TVI_LAST);
	//auto node = new GenericTreeNode(m_ProcessesNode);
	//m_ProcessesNode.SetData(reinterpret_cast<DWORD_PTR>(node));
	m_DumpsNode = m_tree.InsertItem(L"Dump Files", 1, 1, TVI_ROOT, TVI_LAST);
	//node = new GenericTreeNode(m_ProcessesNode);
	//m_DumpsNode.SetData(reinterpret_cast<DWORD_PTR>(node));
	m_ProcessesNode.Expand(TVE_EXPAND);
	m_DumpsNode.Expand(TVE_EXPAND);
}

void CMainFrame::BuildTreeIcons(int size) {
	CImageList images;
	images.Create(size, size, ILC_COLOR32, 16, 8);

	UINT icons[] = {
		IDI_PROCESSES, IDI_DB, IDI_ASSEMBLY, IDI_MODULE, IDI_TYPES, 
		IDI_ASM_DYNAMIC, IDI_FILE_DB, IDI_PROCESS, IDI_THREAD, IDI_APPDOMAIN, 
		IDI_HEAP, IDI_SYNC_CLOSED, IDI_OBJECTS, IDI_HEAP2, IDI_TEXT,
		IDI_CLASS, IDI_OBJECTS2
	};
	for (auto icon : icons) {
		images.AddIcon(AtlLoadIconImage(icon, 64, size, size));
	}
	m_tree.SetImageList(images, TVSIL_NORMAL);
}

LRESULT CMainFrame::OnTreeItemRightClick(int, LPNMHDR, BOOL&) {
	auto item = m_tree.GetSelectedItem();
	if (item == nullptr)
		return 0;

	auto node = reinterpret_cast<TreeNodeBase*>(item.GetData());
	if (node) {
		auto context = node->GetTreeItemContextMenu();
		if (context.first) {
			CMenu menu;
			menu.LoadMenu(context.first);
			POINT pt;
			::GetCursorPos(&pt);
			auto cmd = (UINT)ShowContextMenu(menu.GetSubMenu(context.second), pt, TPM_RETURNCMD);
			if (cmd)
				node->HandleCommand(cmd);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnTreeItemChanged(int, LPNMHDR, BOOL&) {
	auto item = m_tree.GetSelectedItem();
	if(m_CurrentNode != item)
		m_view.Reset();
	m_CurrentNode = item;
	if (item == nullptr)
		return 0;

	SetTimer(1, 300, nullptr);
	return 0;
}

LRESULT CMainFrame::OnTreeItemDeleted(int, LPNMHDR hdr, BOOL&) {
	auto tv = (NMTREEVIEW*)hdr;
	auto node = reinterpret_cast<TreeNodeBase*>(tv->itemOld.lParam);
	if (node) {
		if (node->GetTreeItem() == m_CurrentNode) {
			m_CurrentNode = nullptr;
			m_view.Update(nullptr);
		}
		node->GetTreeItem().SetData(0);
		delete node;
	}
	return 0;
}

LRESULT CMainFrame::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1) {
		KillTimer(1);
		auto node = reinterpret_cast<TreeNodeBase*>(m_CurrentNode.GetData());
		if (node)
			m_view.Update(node);
	}

	return 0;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.AttachMenu(GetMenu());
	SetMenu(nullptr);
	m_CmdBar.m_bAlphaImages = true;
	InitCommandBar();

	CToolBarCtrl tb;
	auto hWndToolBar = tb.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE, 0, ATL_IDW_TOOLBAR);
	InitToolBar(tb);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, nullptr, TRUE);

	CReBarCtrl(m_hWndToolBar).LockBands(TRUE);
	CreateSimpleStatusBar();

	m_hWndClient = m_splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_tree.Create(m_splitter, rcDefault, nullptr, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, 
		WS_EX_CLIENTEDGE, IDC_TREE);
	m_tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER | TVS_EX_RICHTOOLTIP, 0);

	m_view.Create(m_splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	BuildTreeIcons(16);

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
	CProcessSelectDlg dlg;
	if (dlg.DoModal() == IDOK) {
		CString name;
		auto pid = dlg.GetSelectedProcess(name);
		auto dt = DataTarget::FromProcessId(pid);
		if (dt == nullptr) {
			AtlMessageBox(*this, L"Failed to attach to process", IDS_TITLE, MB_ICONERROR);
			return 0;
		}
		Target target(std::move(dt));
		name.Format(L"%s (%u)", name, pid);
		auto node = target.Init(name, m_ProcessesNode);
		node.EnsureVisible();
		node.Expand(TVE_EXPAND);
		m_Targets.push_back(std::move(target));
	}
	return 0;
}

LRESULT CMainFrame::OnOpenDumpFile(WORD, WORD, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, L".dmp", nullptr, OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING,
		L"Dump Files (*.dmp)\0*.dmp\0All Files\0*.*\0", *this);
	if (dlg.DoModal() == IDOK) {
		auto dt = DataTarget::FromDumpFile(dlg.m_szFileName);
		if (dt == nullptr) {
			AtlMessageBox(*this, L"Failed to open dump file", IDS_TITLE, MB_ICONERROR);
			return 0;
		}
		Target target(std::move(dt));
		auto node = target.Init(dlg.m_szFileName, m_DumpsNode);
		node.EnsureVisible();
		node.Expand(TVE_EXPAND);
		m_Targets.push_back(std::move(target));
	}

	return 0;
}

LRESULT CMainFrame::OnLargeTreeIcons(WORD, WORD, HWND, BOOL&) {
	m_TreeIconSize = 40 - m_TreeIconSize;
	BuildTreeIcons(m_TreeIconSize);
	UISetCheck(ID_VIEW_LARGETREEICONS, m_TreeIconSize == 24);

	return 0;
}

LRESULT CMainFrame::OnFileClose(WORD, WORD, HWND, BOOL&) {
	
	return 0;
}

LRESULT CMainFrame::OnViewRefresh(WORD, WORD, HWND, BOOL&) {
	m_view.Refresh();

	return 0;
}

BOOL CMainFrame::ShowContextMenu(HMENU hMenu, const POINT& pt, DWORD flags) {
	return m_CmdBar.TrackPopupMenu(hMenu, flags, pt.x, pt.y);
}

void CMainFrame::InitCommandBar() {
	struct {
		UINT id, icon;
	} cmds[] = {
		{ ID_FILE_ATTACHTOPROCESS, IDI_PROCESS_ATTACH },
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ ID_FILE_OPEN, IDI_OPEN },
		{ ID_TYPE_VIEWOBJECTS, IDI_OBJECTS2 },
	};
	for (auto& cmd : cmds)
		m_CmdBar.AddIcon(AtlLoadIcon(cmd.icon), cmd.id);
}

void CMainFrame::InitToolBar(CToolBarCtrl& tb, int size) {
	CImageList tbImages;
	tbImages.Create(size, size, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);

	struct {
		UINT id;
		int image;
		int style = BTNS_BUTTON;
	} buttons[] = {
		{ ID_FILE_ATTACHTOPROCESS, IDI_PROCESS_ATTACH },
		{ ID_FILE_OPEN, IDI_OPEN },
		{ 0 },
		{ ID_VIEW_REFRESH, IDI_REFRESH },
	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			int image = tbImages.AddIcon(AtlLoadIconImage(b.image, 0, size, size));
			tb.AddButton(b.id, b.style, TBSTATE_ENABLED, image, nullptr, 0);
		}
	}
}

