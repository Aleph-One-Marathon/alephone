/*
 *  shell_beos.cpp - Main game loop and input handling, BeOS specific stuff
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
	BPath path(&info.ref), dir;
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
