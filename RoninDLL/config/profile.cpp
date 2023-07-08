#include "config/profile.h"
#include "dedicated/dedicated.h"
#include <string>

std::string GetRoninPrefix()
{
	return RONIN_FOLDER_PREFIX;
}

void InitialiseRoninPrefix()
{
	char* clachar = strstr(GetCommandLineA(), "-profile=");
	if (clachar)
	{
		std::string cla = std::string(clachar);
		if (strncmp(cla.substr(9, 1).c_str(), "\"", 1))
		{
			int space = cla.find(" ");
			std::string dirname = cla.substr(9, space - 9);
			spdlog::info("Found profile in command line arguments: " + dirname);
			RONIN_FOLDER_PREFIX = dirname;
		}
		else
		{
			std::string quote = "\"";
			int quote1 = cla.find(quote);
			int quote2 = (cla.substr(quote1 + 1)).find(quote);
			std::string dirname = cla.substr(quote1 + 1, quote2);
			spdlog::info("Found profile in command line arguments: " + dirname);
			RONIN_FOLDER_PREFIX = dirname;
		}
	}
	else
	{
		spdlog::info("Profile was not found in command line arguments. Using default: R2Ronin");
		RONIN_FOLDER_PREFIX = "R2Ronin";
	}

	// set the console title to show the current profile
	// dont do this on dedi as title contains useful information on dedi and setting title breaks it as well
	if (!IsDedicatedServer())
		SetConsoleTitleA((std::string("RoninLauncher | ") + RONIN_FOLDER_PREFIX).c_str());
}
