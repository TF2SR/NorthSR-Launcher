#include "pch.h"
#include "loader.h"
#include "include/MinHook.h"
#include <string>
#include <system_error>
#include <sstream>
#include <fstream>
#include <filesystem>

void LibraryLoadError(DWORD dwMessageId, const wchar_t* libName, const wchar_t* location)
{
	char text[4096];
	std::string message = std::system_category().message(dwMessageId);
	sprintf_s(text, "Failed to load the %ls at \"%ls\" (%lu):\n\n%hs", libName, location, dwMessageId, message.c_str());
	if (dwMessageId == 126 && std::filesystem::exists(location))
	{
		sprintf_s(
			text,
			"%s\n\nThe file at the specified location DOES exist, so this error indicates that one of its *dependencies* failed to be "
			"found.",
			text);
	}
	MessageBoxA(GetForegroundWindow(), text, "Ronin Wsock32 Proxy Error", 0);
}

bool ShouldLoadRonin()
{
	bool loadRonin = strstr(GetCommandLineA(), "-ronin");

	if (loadRonin)
		return loadRonin;

	auto runRoninFile = std::ifstream("run_ronin.txt");
	if (runRoninFile)
	{
		std::stringstream runRoninFileBuffer;
		runRoninFileBuffer << runRoninFile.rdbuf();
		runRoninFile.close();
		if (!runRoninFileBuffer.str()._Starts_with("0"))
			loadRonin = true;
	}
	return loadRonin;
}

bool LoadRonin()
{
	FARPROC Hook_Init = nullptr;
	{
		swprintf_s(buffer1, L"%s\\Ronin.dll", exePath);
		auto hHookModule = LoadLibraryExW(buffer1, 0, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (hHookModule)
			Hook_Init = GetProcAddress(hHookModule, "InitialiseRonin");
		if (!hHookModule || Hook_Init == nullptr)
		{
			LibraryLoadError(GetLastError(), L"Ronin.dll", buffer1);
			return false;
		}
	}

	((bool (*)())Hook_Init)();
	return true;
}

typedef int (*LauncherMainType)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LauncherMainType LauncherMainOriginal;

int LauncherMainHook(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (ShouldLoadRonin())
		LoadRonin();
	return LauncherMainOriginal(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

bool ProvisionRonin()
{
	if (!ShouldLoadRonin())
		return true;

	if (MH_Initialize() != MH_OK)
	{
		MessageBoxA(
			GetForegroundWindow(), "MH_Initialize failed\nThe game cannot continue and has to exit.", "Ronin Wsock32 Proxy Error", 0);
		return false;
	}

	auto launcherHandle = GetModuleHandleA("launcher.dll");
	if (!launcherHandle)
	{
		MessageBoxA(
			GetForegroundWindow(),
			"Launcher isn't loaded yet.\nThe game cannot continue and has to exit.",
			"Ronin Wsock32 Proxy Error",
			0);
		return false;
	}

	LPVOID pTarget = GetProcAddress(launcherHandle, "LauncherMain");
	if (MH_CreateHook(pTarget, &LauncherMainHook, reinterpret_cast<LPVOID*>(&LauncherMainOriginal)) != MH_OK ||
		MH_EnableHook(pTarget) != MH_OK)
		MessageBoxA(GetForegroundWindow(), "Hook creation failed for function LauncherMain.", "Ronin Wsock32 Proxy Error", 0);

	return true;
}
