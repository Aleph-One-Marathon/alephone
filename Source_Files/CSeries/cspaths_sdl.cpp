/*
 
	Copyright (C) 2017 and beyond by Jeremiah Morris
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
 
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
 
	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
 
 */

#include "cstypes.h"
#include "cspaths.h"
#include "csstrings.h"
#include "alephversion.h"
#ifdef HAVE_CONFIG_H
#include "confpaths.h"
#endif

#if defined(__APPLE__) && defined(__MACH__)

char get_path_list_separator()
{
	return ':';
}

// other functions handled in cspaths.mm

#elif defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <wchar.h>

char get_path_list_separator()
{
	return ';';
}

static std::string _get_local_data_path()
{
	static std::string local_dir = "";
	if (local_dir.empty())
	{
		wchar_t file_name[MAX_PATH];
		SHGetFolderPathW(NULL,
						CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
						NULL,
						0,
						file_name);
		local_dir = wide_to_utf8(file_name) + "\\AlephOne";
	}
	return local_dir;
}

static std::string _get_default_data_path()
{
	static std::string default_dir = "";
	if (default_dir.empty())
	{
		wchar_t file_name[MAX_PATH];
		const DWORD r = GetModuleFileNameW(NULL, file_name, MAX_PATH); // can truncate
		if (r == 0 || r == MAX_PATH)
			return "";
		*wcsrchr(file_name, L'\\') = L'\0'; // cut off basename, leaving parent dir
		default_dir = wide_to_utf8(file_name);
	}
	return default_dir;
}

static std::string _get_prefs_path()
{
	static std::string prefs_dir = "";
	if (prefs_dir.empty())
	{
		wchar_t file_name[MAX_PATH];
		SHGetFolderPathW(NULL,
						CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
						NULL,
						0,
						file_name);
		prefs_dir = wide_to_utf8(file_name) + "\\AlephOne";
	}
	return prefs_dir;
}

static std::string _get_legacy_login_name()
{
	static std::string login_name = "";
	if (login_name.empty())
	{
		char login[17];
		DWORD len = 17;
		
		bool hasName = (GetUserNameA(login, &len) == TRUE);
		if (!hasName || strpbrk(login, "\\/:*?\"<>|") != NULL)
			strcpy(login, "Bob User");
		login_name = login;
	}
	return login_name;
}

std::string get_data_path(CSPathType type)
{
	std::string path = "";
	
	switch (type) {
		case kPathLocalData:
		case kPathLogs:
			path = _get_local_data_path();
			break;
		case kPathDefaultData:
			path = _get_default_data_path();
			break;
		case kPathBundleData:
			// not applicable
			break;
		case kPathPreferences:
			path = _get_prefs_path();
			break;
		case kPathLegacyData:
		case kPathLegacyPreferences:
			path = _get_default_data_path() + "\\Prefs\\" + _get_legacy_login_name();
			break;
		case kPathScreenshots:
			path = _get_local_data_path() + "\\Screenshots";
			break;
		case kPathSavedGames:
			path = _get_local_data_path() + "\\Saved Games";
			break;
		case kPathQuickSaves:
			path = _get_local_data_path() + "\\Quick Saves";
			break;
		case kPathImageCache:
			path = _get_local_data_path() + "\\Image Cache";
			break;
		case kPathRecordings:
			path = _get_local_data_path() + "\\Recordings";
			break;
	}
	return path;
}

std::string get_application_name()
{
	return std::string(A1_DISPLAY_NAME);
}

std::string get_application_identifier()
{
	return std::string("org.bungie.source.AlephOne");
}

#elif defined(__ANDROID__)

// Android: use SDL's app-specific storage. Internal storage (private, always writable) holds
// prefs/saves/logs/etc.; external app storage holds the scenario data (Map/Shapes/Sounds/Images)
// so it can be dropped in via adb or a file manager: /sdcard/Android/data/<app>/files
#include <SDL2/SDL.h>

char get_path_list_separator()
{
	return ':';
}

static std::string _android_internal_path()
{
	const char* p = SDL_AndroidGetInternalStoragePath();
	return p ? std::string(p) : std::string(".");
}

static std::string _android_external_path()
{
	if (SDL_AndroidGetExternalStorageState() != 0)
	{
		const char* p = SDL_AndroidGetExternalStoragePath();
		if (p) return std::string(p);
	}
	return _android_internal_path();
}

std::string get_data_path(CSPathType type)
{
	switch (type) {
		case kPathPreferences:
			return _android_internal_path();
		// Logs/local data live on EXTERNAL app storage so they're reachable when the
		// Quest is browsed as a USB disk: /sdcard/Android/data/org.alephone/files
		// (internal storage is app-private and only reachable via `adb run-as`).
		case kPathLocalData:
		case kPathLogs:
			return _android_external_path();
		case kPathDefaultData:
			return _android_external_path();   // <- scenario data goes here
		case kPathLegacyData:
		case kPathBundleData:
		case kPathLegacyPreferences:
			return "";
		case kPathScreenshots:
			return _android_external_path() + "/Screenshots";
		case kPathSavedGames:
			return _android_external_path() + "/Saved Games";
		case kPathQuickSaves:
			return _android_external_path() + "/Quick Saves";
		case kPathImageCache:
			return _android_external_path() + "/Image Cache";
		case kPathRecordings:
			return _android_external_path() + "/Recordings";
	}
	return "";
}

std::string get_application_name()
{
	return std::string(A1_DISPLAY_NAME);
}

std::string get_application_identifier()
{
	return std::string("org.bungie.source.AlephOne");
}

#else

// Linux and compatible

char get_path_list_separator()
{
	return ':';
}

static std::string _get_local_data_path()
{
	static std::string local_dir = "";
	if (local_dir.empty())
	{
		const char *home = getenv("HOME");
		if (home)
			local_dir = std::string(home) + "/.alephone";
	}
	return local_dir;
}

std::string get_data_path(CSPathType type)
{
	std::string path = "";
	
	switch (type) {
		case kPathLocalData:
		case kPathLogs:
		case kPathPreferences:
			path = _get_local_data_path();
			break;
		case kPathDefaultData:
#ifdef PKGDATADIR
			path = PKGDATADIR;
#endif
			break;
		case kPathLegacyData:
		case kPathBundleData:
		case kPathLegacyPreferences:
			// not applicable
			break;
		case kPathScreenshots:
			path = _get_local_data_path() + "/Screenshots";
			break;
		case kPathSavedGames:
			path = _get_local_data_path() + "/Saved Games";
			break;
		case kPathQuickSaves:
			path = _get_local_data_path() + "/Quick Saves";
			break;
		case kPathImageCache:
			path = _get_local_data_path() + "/Image Cache";
			break;
		case kPathRecordings:
			path = _get_local_data_path() + "/Recordings";
			break;
	}
	return path;
}

std::string get_application_name()
{
	return std::string(A1_DISPLAY_NAME);
}

std::string get_application_identifier()
{
	return std::string("org.bungie.source.AlephOne");
}


#endif

