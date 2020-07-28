#include "pch.h"
#include "FilterDialogBar.h"

LRESULT CFilterDialogBar::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	((CButton)GetDlgItem(IDC_CLEAR)).SetIcon(AtlLoadIconImage(IDI_CANCEL, 0, 16, 16));
	((CButton)GetDlgItem(IDC_FILTER)).SetIcon(AtlLoadIconImage(IDI_FILTER, 0, 16, 16));

	return 0;
}

LRESULT CFilterDialogBar::OnColorDialog(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}
