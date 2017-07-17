#pragma once
#ifndef CONFIG_H
#define CONFIG_H


enum SAVECONFIGSTATES { DoNotSave, SaveTransparency, SaveAll } shouldsaveconfig;  // Create an enum to store all config states
																				  // DoNotSave        | Fairly self-explanatory
																				  // SaveTransparency | Save opt.taskbar_appearance
																				  // SaveAll          | Save all options

class ConfigHelper
{
public:

	std::wstring cfgPath() const { return configfile; }
	void setCfgPath(std::wstring path) { configfile = path; }
private:
	// holds whether the user passed a --config parameter on the command line
	bool explicitconfig;
	// config file path (defaults to ./config.cfg)
	std::wstring configfile = L"./config.cfg";
};


#endif // CONFIG_H