// HookDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <Windowsx.h>

#define IDM_CHECK 114
#define EXPORT  __declspec(dllexport)

HHOOK hKeyboardHook = NULL;
HHOOK hMouseHook = NULL;
HINSTANCE hinstLib;
HWND hWndProgram;

bool flag = false;

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) // không xử lý message 
		return CallNextHookEx(hMouseHook, nCode, wParam, lParam);

	if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP)
	{
		if (flag) return -1;
	}

	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) // không xử lý message 
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);

	if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_SPACE) & 0x8000))
	{
		//MessageBeep(MB_ICONASTERISK);
		flag = !flag;
		PostMessage(hWndProgram, WM_COMMAND, IDM_CHECK, 0);
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

EXPORT void _doInstallHook(HWND hWnd)
{
	hWndProgram = hWnd;
	if (hKeyboardHook != NULL) return;
	if (hMouseHook != NULL) return;

	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardHookProc, hinstLib, 0);
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)MouseHookProc, hinstLib, 0);
}

EXPORT void _doRemoveHook(HWND hWnd)
{
	if (hKeyboardHook != NULL) UnhookWindowsHookEx(hKeyboardHook);
	if (hMouseHook != NULL) UnhookWindowsHookEx(hMouseHook);

	hMouseHook = NULL;
	hKeyboardHook = NULL;
}

EXPORT bool _checkEnabled()
{
	return flag;
}

EXPORT void _changeState(bool state)
{
	flag = state;
}