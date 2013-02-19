#include "DlgAbout.h"

DlgAbout::DlgAbout(HWND hwnd)
{
	DialogBox( GetModuleHandle(NULL), MAKEINTRESOURCE( DIALOG_ABOUT ), hwnd, DlgAboutProc );
}

DlgAbout::~DlgAbout(void)
{
}

INT_PTR CALLBACK DlgAboutProc(
  _In_  HWND hwndDlg,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
)
{

	switch (uMsg)
	{
	case WM_INITDIALOG:
		char buffer[16];
		sprintf(buffer, "%d.%d", MAJOR_VERSION, MINOR_VERSION);
		
		SendMessage( GetDlgItem( hwndDlg, ABOUT_VERSION), WM_SETTEXT, 0, (LPARAM)buffer);
		return TRUE;

	case WM_CLOSE:
		EndDialog( hwndDlg, 0 );
		return TRUE;
	}
	return FALSE;
}