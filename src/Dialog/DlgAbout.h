#pragma once
#include <windows.h>
#include <cstdio>
#include "../resource.h"
#include "../version.h"

class DlgAbout
{
public:
	DlgAbout(HWND hwnd);
	~DlgAbout(void);
};

INT_PTR CALLBACK DlgAboutProc(
  _In_  HWND hwndDlg,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
);
