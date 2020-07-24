// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VirtualListView.h"

class TreeNodeBase;

class CView : 
	public CFrameWindowImpl<CView, CWindow, CControlWinTraits>,
	public CVirtualListView<CView> {
public:
	using BaseClass = CFrameWindowImpl<CView, CWindow, CControlWinTraits>;

	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg);
	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(int row) const;
	void DoSort(const SortInfo* si);

	void Reset();
	void Update(TreeNodeBase* node);

	BEGIN_MSG_MAP(CView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CView>)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	TreeNodeBase* m_CurrentNode{ nullptr };
	CListViewCtrl m_List;
};
