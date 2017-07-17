#pragma once
#ifndef TASKBAR_H
#define TASKBAR_H
#include <windows.h>
#include <map>
#include <ShlObj.h>
#include "configstate.h"
#include "common.h"


struct ACCENTPOLICY
{
	int nAccentState;
	int nFlags;
	int nColor;
	int nAnimationId;
};

struct WINCOMPATTRDATA
{
	int nAttribute; // the composition attribute index
	void* pData; // ACCENTPOLICY passed
	unsigned long ulDataSize; // size of ACCENTPOLICY passed
};

typedef BOOL(WINAPI*pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
static pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetWindowCompositionAttribute");



struct TASKBARPROPERTIES
{
	HMONITOR hmon; // Current monitor for Taskbar
	TASKBARSTATE state;
};


class TaskbarManager
{
public:
	void update();
private:
//	HWND taskbar;
//	HWND secondtaskbar;
	TaskbarManager(ConfigState &configState);
	ConfigState* config;
	bool CALLBACK TaskbarManager::EnumWindowsProcess(HWND hWnd, LPARAM lParam);
	void SetWindowBlur(HWND hWnd, int appearance = 0);
	bool isBlacklisted(HWND hWnd);
	void RefreshHandles();
	IVirtualDesktopManager *desktop_manager;
	std::map<HWND, TASKBARPROPERTIES> taskbars; // Create a map for all taskbars
	uint32_t counter = 0;

};

inline TaskbarManager::TaskbarManager(ConfigState &configState)
{
	//Virtual Desktop stuff
	::CoInitialize(NULL);
	HRESULT desktop_success = ::CoCreateInstance(__uuidof(VirtualDesktopManager), NULL, CLSCTX_INPROC_SERVER, IID_IVirtualDesktopManager, (void **)&desktop_manager);
	if (!desktop_success) { OutputDebugStringW(L"Initialization of VirtualDesktopManager failed"); }
}


bool CALLBACK TaskbarManager::EnumWindowsProcess(HWND hWnd, LPARAM lParam)
{
	HMONITOR _monitor;

	if (config->dynamicws)
	{
		WINDOWPLACEMENT result = {};
		GetWindowPlacement(hWnd, &result);
		if (result.showCmd == SW_MAXIMIZE) {
			BOOL on_current_desktop;
			desktop_manager->IsWindowOnCurrentVirtualDesktop(hWnd, &on_current_desktop);
			if (IsWindowVisible(hWnd) && on_current_desktop)
			{
				if (!isBlacklisted(hWnd))
				{
					_monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
					for (auto &taskbar : taskbars)
					{
						if (taskbar.second.hmon == _monitor &&
							taskbar.second.state != StartMenuOpen)
						{
							taskbar.second.state = WindowMaximised;
						}
					}
				}
			}
		}
	}
	return true;
}


bool TaskbarManager::isBlacklisted(HWND hWnd)
{
	// Get respective attributes
	TCHAR className[MAX_PATH];
	TCHAR exeName_path[MAX_PATH];
	TCHAR windowTitle[MAX_PATH];
	GetClassName(hWnd, className, _countof(className));
	GetWindowText(hWnd, windowTitle, _countof(windowTitle));

	DWORD ProcessId;
	GetWindowThreadProcessId(hWnd, &ProcessId);
	HANDLE processhandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, ProcessId);
	GetModuleFileNameEx(processhandle, NULL, exeName_path, _countof(exeName_path));

	std::wstring exeName = PathFindFileNameW(exeName_path);
	std::wstring w_WindowTitle = windowTitle;

	// Check if the different vars are in their respective vectors
	for (auto & value : config->IgnoredClassNames)
	{
		if (className == value.c_str()) { return true; }
	}
	for (auto & value : config->IgnoredExeNames)
	{
		if (exeName == value) { return true; }
	}
	for (auto & value : config->IgnoredWindowTitles)
	{
		if (w_WindowTitle.find(value) != std::wstring::npos)
		{
			return true;
		}
	}
	return false;
}

void TaskbarManager::update()
{
	if (this->counter >= 10)
	{
		for (auto & taskbar: taskbars)
		{
			taskbar.second.state = Normal;
		}
		if (config->dynamicws) {
			this->counter = 0;
			EnumWindows(reinterpret_cast<WNDENUMPROC>(this->EnumWindowsProcess), NULL);
		}
		if (config->dynamicstart)
		{
			HWND foreground;
			TCHAR ForehWndClass[MAX_PATH];
			TCHAR ForehWndName[MAX_PATH];

			foreground = GetForegroundWindow();
			GetWindowText(foreground, ForehWndName, _countof(ForehWndName));
			GetClassName(foreground, ForehWndClass, _countof(ForehWndClass));

			if (!_tcscmp(ForehWndClass, _T("Windows.UI.Core.CoreWindow")) &&
				(!_tcscmp(ForehWndName, _T("Search")) || !_tcscmp(ForehWndName, _T("Cortana"))))
			{
				// Detect monitor Start Menu is open on
				HMONITOR _monitor;
				_monitor = MonitorFromWindow(foreground, MONITOR_DEFAULTTOPRIMARY);
				for (auto &taskbar : taskbars)
				{
					if (taskbar.second.hmon == _monitor)
					{
						taskbar.second.state = StartMenuOpen;
					}
					else {
						taskbar.second.state = Normal;
					}
				}
			}
		}
	}

	for (auto const &taskbar : taskbars)
	{
		if (taskbar.second.state == WindowMaximised) {
			SetWindowBlur(taskbar.first, config->dynamicState);
			// A window is maximised; let's make sure that we blur the window.
		}
		else if (taskbar.second.state == Normal) {
			SetWindowBlur(taskbar.first);  // Taskbar should be normal, call using normal transparency settings
		}

	}
	counter++;
}

void TaskbarManager::SetWindowBlur(HWND hWnd, int appearance = 0) // `appearance` can be 0, which means 'follow config->taskbar_appearance'
{
	if (SetWindowCompositionAttribute)
	{
		ACCENTPOLICY policy;

		if (appearance) // Custom taskbar appearance is set
		{
			if (config->dynamicState == TASKBARACCENT.ACCENT_ENABLE_TINTED)
			{ // dynamic-ws is set to tint
				if (appearance == TASKBARACCENT.ACCENT_ENABLE_TINTED) { policy = { TASKBARACCENT.ACCENT_ENABLE_TRANSPARENTGRADIENT, 2, config->color, 0 }; } // Window is maximised
				else { policy = { TASKBARACCENT.ACCENT_ENABLE_TRANSPARENTGRADIENT, 2, 0x00000000, 0 }; } // Desktop is shown (this shouldn't ever be called tho, just in case)
			}
			else { policy = { appearance, 2, config->color, 0 }; }
		}
		else { // Use the defaults
			if (config->dynamicState == TASKBARACCENT.ACCENT_ENABLE_TINTED) { policy = { TASKBARACCENT.ACCENT_ENABLE_TRANSPARENTGRADIENT, 2, 0x00000000, 0 }; } // dynamic-ws is tint and desktop is shown
			else if (config->taskbar_appearance == TASKBARACCENT.ACCENT_NORMAL_GRADIENT) { policy = { TASKBARACCENT.ACCENT_ENABLE_TRANSPARENTGRADIENT, 2, (int)0xd9000000, 0 }; } // normal gradient color
			else { policy = { config->taskbar_appearance, 2, config->color, 0 }; }
		}

		WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) }; // WCA_ACCENT_POLICY=19
		SetWindowCompositionAttribute(hWnd, &data);
	}

}


void TaskbarManager::RefreshHandles()
{
	HWND _taskbar;
	HWND _secondtaskbar;
	TASKBARPROPERTIES _properties;

	taskbars.clear();
	_taskbar = FindWindowW(L"Shell_TrayWnd", NULL);

	_properties.hmon = MonitorFromWindow(_taskbar, MONITOR_DEFAULTTOPRIMARY);
	_properties.state = Normal;
	taskbars.insert(std::make_pair(_taskbar, _properties));
	while (_secondtaskbar = FindWindowEx(0, _secondtaskbar, L"Shell_SecondaryTrayWnd", NULL))
	{
		_properties.hmon = MonitorFromWindow(_secondtaskbar, MONITOR_DEFAULTTOPRIMARY);
		_properties.state = Normal;
		taskbars.insert(std::make_pair(_secondtaskbar, _properties));
	}
}




#endif // TASKBAR_H
