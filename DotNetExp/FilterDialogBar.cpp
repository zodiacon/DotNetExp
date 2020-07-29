#include "pch.h"
#include "FilterDialogBar.h"
#include "View.h"

CFilterDialogBar::CFilterDialogBar(CView* view) : m_pView(view), m_Edit(this, 1) {
}

void CFilterDialogBar::SetEditFocus() {
	m_Edit.SetFocus();
}

LRESULT CFilterDialogBar::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	((CButton)GetDlgItem(IDC_CLEAR)).SetIcon(AtlLoadIconImage(IDI_CANCEL, 0, 16, 16));
	((CButton)GetDlgItem(IDC_FILTER)).SetIcon(AtlLoadIconImage(IDI_FILTER, 0, 16, 16));

	m_Edit.SubclassWindow(GetDlgItem(IDC_TEXT));

	return 0;
}

LRESULT CFilterDialogBar::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1) {
		DoApplyFilter();
	}
	return 0;
}

LRESULT CFilterDialogBar::OnColorDialog(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CFilterDialogBar::OnEditKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	SetTimer(1, 300, nullptr);

	if (wParam == VK_ESCAPE) {
		m_Edit.SetWindowTextW(L"");
		return 0;
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CFilterDialogBar::OnApplyFilter(WORD, WORD, HWND, BOOL&) {
	DoApplyFilter();
	return 0;
}

void CFilterDialogBar::DoApplyFilter() {
	KillTimer(1);
	CString text;
	GetDlgItemText(IDC_TEXT, text);
	m_pView->ApplyFilter(text);
}

LRESULT CFilterDialogBar::OnClear(WORD, WORD, HWND, BOOL&) {
	SetDlgItemText(IDC_TEXT, L"");
	SetTimer(1, 300, nullptr);
	return 0;
}
