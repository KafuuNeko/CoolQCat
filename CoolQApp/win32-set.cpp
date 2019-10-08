#include "stdafx.h"
#include "win32-set.h"

using namespace std;

bool windowsExist = false;

void LoadWindows_MenuSet()
{
	if (windowsExist) {
		return;
	}
	windowsExist = true;
	WNDCLASS wndcls;
	wndcls.cbClsExtra = 0;
	wndcls.cbWndExtra = 0;
	wndcls.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndcls.lpfnWndProc = WinSunProc;
	wndcls.lpszClassName = "MenuWindows";
	wndcls.lpszMenuName = NULL;
	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wndcls);

	int  cx = GetSystemMetrics(SM_CXFULLSCREEN);
	int  cy = GetSystemMetrics(SM_CYFULLSCREEN);

	int height = 150;
	int width = 250;
	HWND hwnd;
	hwnd = CreateWindow(wndcls.lpszClassName, "设置", WS_SYSMENU, cx / 2 - width / 2, cy / 2 - height / 2, width, height, NULL, NULL, NULL, NULL); //建立一个窗口

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	windowsExist = false;
}

LRESULT CALLBACK WinSunProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	int wmId, wmEvent;
	HWND hwndEditDistinct = NULL;
	ostringstream os;
	char *tempStr;

	switch (uMsg) {
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDB_BUTTON_SAVE:
			TCHAR sztitle[32];
			GetDlgItemText(hwnd, IDB_EDIT_DISTINCT, (LPSTR)sztitle, 32);

			os << sztitle;
			WriteConfig("image", "w", os.str().c_str(), "config.ini");

			MessageBoxA(hwnd, TEXT("您已成功保存设置"), TEXT("信息"), 64);

			break;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		break;

	case WM_CREATE:

		CreateWindow(TEXT("button"), TEXT("保存"), WS_CHILD | WS_VISIBLE, 150, 80, 75, 25, hwnd, (HMENU)IDB_BUTTON_SAVE, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

		CreateWindow("static", TEXT("清晰度："), WS_CHILD | WS_VISIBLE | WS_BORDER,
			5, 20, 100, 25, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		
		tempStr = ReadConfig("image","w","600","config.ini");
		hwndEditDistinct = CreateWindow("edit", TEXT(tempStr), WS_CHILD | WS_VISIBLE | WS_BORDER,
			100, 20, 130, 25, hwnd, (HMENU)IDB_EDIT_DISTINCT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		delete[] tempStr;

		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);  //销毁窗口
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

