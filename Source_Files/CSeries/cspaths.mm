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

#import <Cocoa/Cocoa.h>
#include "cstypes.h"
#include "cspaths.h"

static std::string _add_app_name(std::string parent)
{
#ifdef PREFER_APP_NAME_TO_BUNDLE_ID
	return parent + "/" + get_application_name();
#else
	return parent + "/" + "AlephOne";
#endif
}

static std::string _add_app_id(std::string parent)
{
#ifdef PREFER_APP_NAME_TO_BUNDLE_ID
	return parent + "/" + get_application_name();
#else
	return parent + "/" + get_application_identifier();
#endif
}

static std::string _get_local_data_path()
{
	static std::string local_data_dir = "";
	if (local_data_dir.empty())
	{
		NSArray *arr = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
		NSString *supportPath = [arr objectAtIndex:0];
		if (supportPath != nil)
			local_data_dir = _add_app_name([supportPath UTF8String]);
	}
	return local_data_dir;
}

static std::string _get_default_data_path()
{
	static std::string default_dir = "";
	if (default_dir.empty())
	{
		char parentdir[MAXPATHLEN];
		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
		if (CFURLGetFileSystemRepresentation(url2, true, (UInt8 *)parentdir, MAXPATHLEN)) {
			default_dir = parentdir;
		}
		CFRelease(url);
		CFRelease(url2);
	}
	return default_dir;
}

static std::string _get_library_path()
{
	static std::string library_dir = "";
	if (library_dir.empty())
	{
		NSArray *arr = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
		NSString *libraryPath = [arr objectAtIndex:0];
		if (libraryPath != nil)
			library_dir = [libraryPath UTF8String];
	}
	return library_dir;
}

#ifdef MAC_APP_STORE
static std::string _get_pictures_path()
{
	static std::string pictures_dir = "";
	if (pictures_dir.empty())
	{
		NSArray *arr = NSSearchPathForDirectoriesInDomains(NSPicturesDirectory, NSUserDomainMask, YES);
		NSString *picturesPath = [arr objectAtIndex:0];
		if (picturesPath != nil)
			pictures_dir = [picturesPath UTF8String];
	}
	return pictures_dir;
}
#endif

std::string get_data_path(CSPathType type)
{
	std::string path = "";
	
	switch (type) {
		case kPathLocalData:
			path = _get_local_data_path();
			break;
		case kPathDefaultData:
			path = _get_default_data_path();
			break;
		case kPathLegacyData:
			// not applicable
			break;
		case kPathBundleData:
			path = std::string([[[NSBundle mainBundle] resourcePath] UTF8String]) + "/DataFiles";
			break;
		case kPathLogs:
			path = _get_library_path() + "/Logs";
			break;
		case kPathPreferences:
			path = _add_app_id(_get_library_path() + "/Preferences");
			break;
		case kPathLegacyPreferences:
			path = _get_local_data_path();
			break;
		case kPathScreenshots:
#ifdef MAC_APP_STORE
			path = _add_app_name(_get_pictures_path()) + " Screenshots";
#else
			path = _get_local_data_path() + "/Screenshots";
#endif
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
	static std::string name = "";
	if (name.empty())
	{
		NSDictionary *bundleInfo = [[NSBundle mainBundle] localizedInfoDictionary];
		NSString *appName = [bundleInfo objectForKey:(NSString *)kCFBundleNameKey];
		name = [appName UTF8String];
	}
	return name;
}

std::string get_application_identifier()
{
	static std::string ident = "";
	if (ident.empty())
	{
		NSDictionary *bundleInfo = [[NSBundle mainBundle] infoDictionary];
		NSString *bundleID = [bundleInfo objectForKey:(NSString *)kCFBundleIdentifierKey];
		ident = [bundleID UTF8String];
	}
	return ident;
}
