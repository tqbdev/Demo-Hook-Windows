// Hook.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Hook.h"
#include <wchar.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

NOTIFYICONDATA iconTray;
BOOL enabled = FALSE;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void doInstallHook(HWND);
void doRemoveHook(HWND);
bool checkEnabled();
void changeState(bool state);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//# Allow only one instance to run
	CreateMutex(0, 0, L"MouseHook");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_HOOK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = szWindowClass;
	wcex.hbrBackground = GetSysColorBrush(COLOR_3DFACE);

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	doInstallHook(hWnd);

	iconTray.cbSize = sizeof(NOTIFYICONDATA);
	iconTray.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ENABLE));	//# Passing cursor handle in place of icon
	iconTray.hWnd = hWnd;
	wcscpy_s(iconTray.szTip, L"Control left click (Ctrl + Space)");
	iconTray.uCallbackMessage = WM_TRAY;
	iconTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	iconTray.uID = IDC_TRAY;
	Shell_NotifyIcon(NIM_ADD, &iconTray);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			doRemoveHook(hWnd);
			Shell_NotifyIcon(NIM_DELETE, &iconTray);
			DestroyWindow(hWnd);
			break;
		case IDM_DIS_EN_ABLE:
			changeState(!checkEnabled());
			if (checkEnabled())
			{
				iconTray.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DISABLE));
				wcscpy_s(iconTray.szInfoTitle, L"Notifications");
				wcscpy_s(iconTray.szInfo, L"Disabled left click");
				Shell_NotifyIcon(NIM_MODIFY, &iconTray);
			}
			else
			{
				iconTray.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ENABLE));
				wcscpy_s(iconTray.szInfoTitle, L"Notifications");
				wcscpy_s(iconTray.szInfo, L"Enabled left click");
				Shell_NotifyIcon(NIM_MODIFY, &iconTray);
			}
			break;
		case IDM_CHECK:
			if (checkEnabled())
			{
				iconTray.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DISABLE));
				wcscpy_s(iconTray.szInfoTitle, L"Notifications");
				wcscpy_s(iconTray.szInfo, L"Disabled left click");
				Shell_NotifyIcon(NIM_MODIFY, &iconTray);
			}
			else
			{
				iconTray.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ENABLE));
				wcscpy_s(iconTray.szInfoTitle, L"Notifications");
				wcscpy_s(iconTray.szInfo, L"Enabled left click");
				Shell_NotifyIcon(NIM_MODIFY, &iconTray);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_TRAY:
	{
		if (WM_LBUTTONDBLCLK == lParam)							//# Show settings dialog
			PostMessage(hWnd, WM_COMMAND, IDM_DIS_EN_ABLE, 0);
		else if (WM_RBUTTONDOWN == lParam)						//# Show menu
		{
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, IDM_DIS_EN_ABLE, L"Enabled (Ctrl + Space)");
			AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
			AppendMenu(hMenu, MF_STRING, IDM_ABOUT, L"About");
			AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");
			SetMenuDefaultItem(hMenu, 0, TRUE);

			if (checkEnabled())
			{
				CheckMenuItem(hMenu, 0, MF_BYPOSITION | MF_CHECKED);
			}
			else
			{
				CheckMenuItem(hMenu, 0, MF_BYPOSITION | MF_UNCHECKED);
			}

			SetForegroundWindow(hWnd);
			POINT pt;
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

			DestroyMenu(hMenu);
			hMenu = NULL;

			PostMessage(hWnd, WM_NULL, 0, 0);
		}
	}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		doRemoveHook(hWnd);
		Shell_NotifyIcon(NIM_DELETE, &iconTray);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void doInstallHook(HWND hWnd)
{
	// gọi hàm HookDll theo kiểu Run-time
	// Định nghĩa prototype của hàm
	typedef VOID(*MYPROC)(HWND);

	HINSTANCE hinstLib;
	MYPROC ProcAddr;

	// load HookDll và lấy handle của HookDll module
	hinstLib = LoadLibrary(L"HookDll.dll");
	// Nếu load thành công, lấy địa chỉ của hàm _doInstallHook trong HookDll
	if (hinstLib != NULL) {
		ProcAddr = (MYPROC)GetProcAddress(hinstLib, "_doInstallHook");
		// Nếu lấy được địa chỉ hàm, gọi thực hiện hàm
		if (ProcAddr != NULL)
			ProcAddr(hWnd);
	}
}

void doRemoveHook(HWND hWnd)
{
	// gọi hàm HookDll theo kiểu Run-time
	// Định nghĩa prototype của hàm
	typedef VOID(*MYPROC)(HWND);

	HINSTANCE hinstLib;
	MYPROC ProcAddr;

	// load HookDll và lấy handle của HookDll module
	hinstLib = LoadLibrary(L"HookDll.dll");
	// Nếu load thành công, lấy địa chỉ của hàm _doRemoveHook trong HookDll
	if (hinstLib != NULL) {
		ProcAddr = (MYPROC)GetProcAddress(hinstLib, "_doRemoveHook");
		// Nếu lấy được địa chỉ hàm, gọi thực hiện hàm
		if (ProcAddr != NULL)
			ProcAddr(hWnd);
	}
}

bool checkEnabled()
{
	// gọi hàm HookDll theo kiểu Run-time
	// Định nghĩa prototype của hàm
	typedef bool(*MYPROC)();

	HINSTANCE hinstLib;
	MYPROC ProcAddr;

	// load HookDll và lấy handle của HookDll module
	hinstLib = LoadLibrary(L"HookDll.dll");
	// Nếu load thành công, lấy địa chỉ của hàm _checkEnabled trong HookDll
	if (hinstLib != NULL) {
		ProcAddr = (MYPROC)GetProcAddress(hinstLib, "_checkEnabled");
		// Nếu lấy được địa chỉ hàm, gọi thực hiện hàm
		if (ProcAddr != NULL)
			return ProcAddr();
	}
}

void changeState(bool state)
{
	// gọi hàm HookDll theo kiểu Run-time
	// Định nghĩa prototype của hàm
	typedef VOID(*MYPROC)(bool);

	HINSTANCE hinstLib;
	MYPROC ProcAddr;

	// load HookDll và lấy handle của HookDll module
	hinstLib = LoadLibrary(L"HookDll.dll");
	// Nếu load thành công, lấy địa chỉ của hàm _changeState trong HookDll
	if (hinstLib != NULL) {
		ProcAddr = (MYPROC)GetProcAddress(hinstLib, "_changeState");
		// Nếu lấy được địa chỉ hàm, gọi thực hiện hàm
		if (ProcAddr != NULL)
			ProcAddr(state);
	}
}