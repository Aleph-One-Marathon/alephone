/* cspaths.h

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

	Provides OS-specific directory paths and application info
*/

#ifndef _CSERIES_PATHS_
#define _CSERIES_PATHS_

#include "cstypes.h"
#include <string>

typedef enum {
	kPathLocalData,
	kPathDefaultData,
	kPathLegacyData,
	kPathBundleData,
	kPathLogs,
	kPathPreferences,
	kPathLegacyPreferences,
	kPathScreenshots,
	kPathSavedGames,
	kPathQuickSaves,
	kPathImageCache,
	kPathRecordings
} CSPathType;

std::string get_data_path(CSPathType type);
char get_path_list_separator();

std::string get_application_name();
std::string get_application_identifier();

#endif
