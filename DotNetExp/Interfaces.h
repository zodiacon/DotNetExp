#pragma once

struct IFilterBarCallback {
	virtual int ApplyFilter(const CString& text) = 0;
};

struct IFilterBar {
	virtual void Clear() = 0;
};

struct IMainFrame {
	virtual BOOL ShowContextMenu(HMENU hMenu, const POINT& pt, DWORD flags = 0) = 0;
};
