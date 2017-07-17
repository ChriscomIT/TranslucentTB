#pragma once
#ifndef COMMON_H
#define COMMON_H

struct TASKBARACCENT
{
	// these values correspond to actual paramters passed to the SetWindowsCompositionAttribute calls
	const int ACCENT_ENABLE_GRADIENT = 1; // Makes the taskbar a solid color specified by nColor. This mode doesn't care about the alpha channel.
	const int ACCENT_ENABLE_TRANSPARENTGRADIENT = 2; // Makes the taskbar a tinted transparent overlay. nColor is the tint color, sending nothing results in it interpreted as 0x00000000 (totally transparent, blends in with desktop)
	const int ACCENT_ENABLE_BLURBEHIND = 3; // Makes the taskbar a tinted blurry overlay. nColor is same as above.
											// these values are fake and used internally only
	const int ACCENT_ENABLE_TINTED = 5; // This is not a real state. We will handle it later.
	const int ACCENT_NORMAL_GRADIENT = 6; // Another fake value, handles normal state.
} TASKBARACCENT;

enum TASKBARSTATE
{
	Normal, // Proceed as normal. If no dynamic options are set, act as it says in opt.taskbar_appearance
	WindowMaximised, // There is a window which is maximised on the monitor this HWND is in. Display as blurred.
	StartMenuOpen // The Start Menu is open on the monitor this HWND is in. Display as it would be without TranslucentTB active.
};

#endif // COMMON_H
