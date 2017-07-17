#pragma once
#ifndef CONFIGSTATE_H
#define CONFIGSTATE_H

#include "common.h"

class ConfigState
{
public:
	uint8_t dynamicState;
	uint8_t taskbar_appearance = TASKBARACCENT.ACCENT_ENABLE_BLURBEHIND;
	uint32_t color;
	bool dynamicws;
	bool dynamicstart;
	std::vector<std::wstring> IgnoredClassNames;
	std::vector<std::wstring> IgnoredExeNames;
	std::vector<std::wstring> IgnoredWindowTitles;

};

#endif // CONFIGSTATE_H