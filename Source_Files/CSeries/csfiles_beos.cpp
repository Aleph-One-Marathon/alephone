/*
 *  csfiles_beos.cpp - Some BeOS-specific functions that didn't fit elsewhere
 *                     (used by shell_sdl.cpp)
 *
 *  Written in 2000 by Christian Bauer
 */

#include <AppKit.h>
#include <StorageKit.h>
#include <string>


/*
 *  Find application and preferences directories
 */

string get_application_directory(void)
{
	app_info info;
	be_app->GetAppInfo(&info);
	BEntry entry(&info.ref);
	BPath path(&entry), dir;
	path.GetParent(&dir);
	return dir.Path();
}

string get_preferences_directory(void)
{
	BPath prefs_dir;
	find_directory(B_USER_SETTINGS_DIRECTORY, &prefs_dir, true);
	prefs_dir.Append("Aleph One");
	return prefs_dir.Path();
}
