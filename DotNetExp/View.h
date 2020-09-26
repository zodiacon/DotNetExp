// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VirtualListView.h"
#include "FilterDialogBar.h"

class TreeNodeBase;
struct IFilterBarCallback;
struct IMainFrame;

class CView : 
	public CFrameWindowImpl<CView, CWindow, CControlWinTraits>,
	public CVirtualListView<CView> {
public:
	using BaseClass = CFrameWindowImpl<CView, CWindow, CControlWinTraits>;

	DECLARE_WND_CLASS(NULL)

	CView(IMainFrame* frame, bool dynamic = false);

	BOOL PreTranslateMessage(MSG* pMsg);
	void OnFinalMessage(HWND) override;
	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(int row) const;
	void DoSort(const SortInfo* si);
	bool IsSortable(int col) const;

	void Reset();
	void Update(TreeNodeBase* node);
	void Refresh();
	void ApplyFilter(const CString& text);

	BEGIN_MSG_MAP(CView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnRightClick)
		CHAIN_MSG_MAP(CVirtualListView<CView>)
		CHAIN_MSG_MAP(BaseClass)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_FILTER, OnFilter)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRightClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	void InitToolBar(CToolBarCtrl& tb);

private:
	IMainFrame* m_pFrame;
	TreeNodeBase* m_CurrentNode{ nullptr };
	CListViewCtrl m_List;
	CFilterDialogBar m_FilterBar;
	IFilterBarCallback* m_pFilterBarCB{ nullptr };
	bool m_Dynamic;
};
