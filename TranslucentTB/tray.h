#pragma once
#ifndef TRAY_H
#define TRAY_H
#include "common.h"
#include "configstate.h"


#define WM_NOTIFY_TB 3141


class TrayMenu
{
public:
	void initTray(HWND parent, ConfigState* &config);
	void RefreshMenu();
private:
	unsigned int WM_TASKBARCREATED;
	unsigned int NEW_TTB_INSTANCE;
	HMENU popup;
	HMENU menu;
	NOTIFYICONDATA Tray;
	HWND tray_hwnd;
	ConfigState* config;
	LRESULT CALLBACK TBPROCWND(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

void TrayMenu::initTray(HWND parent, ConfigState* &config)
{
	this->config = config;
	Tray.cbSize = sizeof(Tray);
	Tray.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON));
	Tray.hWnd = parent;
	wcscpy_s(Tray.szTip, L"TranslucentTB");
	Tray.uCallbackMessage = WM_NOTIFY_TB;
	Tray.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	Tray.uID = 101;
	Shell_NotifyIcon(NIM_ADD, &Tray);
	Shell_NotifyIcon(NIM_SETVERSION, &Tray);
	RefreshMenu();

}

void TrayMenu::RefreshMenu()
{
	if (config->dynamicws)
	{
		CheckMenuRadioItem(popup, IDM_BLUR, IDM_DYNAMICWS, IDM_DYNAMICWS, MF_BYCOMMAND);
	}
	else if (config->taskbar_appearance == TASKBARACCENT.ACCENT_ENABLE_BLURBEHIND)
	{
		CheckMenuRadioItem(popup, IDM_BLUR, IDM_DYNAMICWS, IDM_BLUR, MF_BYCOMMAND);
	}
	else if (config->taskbar_appearance == TASKBARACCENT.ACCENT_ENABLE_TRANSPARENTGRADIENT)
	{
		CheckMenuRadioItem(popup, IDM_BLUR, IDM_DYNAMICWS, IDM_CLEAR, MF_BYCOMMAND);
	}
	else if (config->taskbar_appearance == TASKBARACCENT.ACCENT_NORMAL_GRADIENT)
	{
		CheckMenuRadioItem(popup, IDM_BLUR, IDM_DYNAMICWS, IDM_NORMAL, MF_BYCOMMAND);
	}

	if (config->dynamicstart)
	{
		CheckMenuItem(popup, IDM_DYNAMICSTART, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		CheckMenuItem(popup, IDM_DYNAMICSTART, MF_BYCOMMAND | MF_UNCHECKED);
	}

	if (RegGetValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", L"TranslucentTB", RRF_RT_REG_SZ, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		CheckMenuItem(popup, IDM_AUTOSTART, MF_BYCOMMAND | MF_CHECKED);
	}
}

LRESULT CALLBACK TrayMenu::TBPROCWND(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_NOTIFY_TB:
		if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hWnd);
			UINT tray = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY, pt.x, pt.y, 0, hWnd, NULL);
			switch (tray)
			{
			case IDM_BLUR:
				config->dynamicws = false;
				config->taskbar_appearance = TASKBARACCENT.ACCENT_ENABLE_BLURBEHIND;
				if (shouldsaveconfig == DoNotSave &&
					shouldsaveconfig != SaveAll)
					shouldsaveconfig = SaveTransparency;
				RefreshMenu();
				break;
			case IDM_CLEAR:
				config->dynamicws = false;
				config->taskbar_appearance = TASKBARACCENT.ACCENT_ENABLE_TRANSPARENTGRADIENT;
				if (shouldsaveconfig == DoNotSave &&
					shouldsaveconfig != SaveAll)
					shouldsaveconfig = SaveTransparency;
				RefreshMenu();
				break;
			case IDM_NORMAL:
				config->dynamicws = false;
				config->taskbar_appearance = TASKBARACCENT.ACCENT_NORMAL_GRADIENT;
				RefreshMenu();
				// TODO: shouldsaveconfig implementation
				break;
			case IDM_DYNAMICWS:
				config->taskbar_appearance = TASKBARACCENT.ACCENT_ENABLE_TRANSPARENTGRADIENT;
				config->dynamicws = true;
				EnumWindows(&EnumWindowsProcess, NULL);
				// TODO: shouldsaveconfig implementation
				RefreshMenu();
				break;
			case IDM_DYNAMICSTART:
				config->dynamicstart = !config->dynamicstart;
				// TODO: shouldsaveconfig implementation
				RefreshMenu();
				break;
			case IDM_AUTOSTART:
				if (RegGetValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", L"TranslucentTB", RRF_RT_REG_SZ, NULL, NULL, NULL) == ERROR_SUCCESS)
				{
					HKEY hkey = NULL;
					RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
					RegDeleteValue(hkey, L"TranslucentTB");
				}
				else {
					add_to_startup();
				}
				RefreshMenu();
				break;
			case IDM_EXIT:
				run = false;
				break;
			}
		}
	}
	if (message == WM_TASKBARCREATED) // Unfortunately, WM_TASKBARCREATED is not a constant, so I can't include it in the switch.
	{
		RefreshHandles();
		initTray(tray_hwnd, config);
	}
	else if (message == NEW_TTB_INSTANCE) {
		shouldsaveconfig = DoNotSave;
		run = false;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


#endif // TRAY_H