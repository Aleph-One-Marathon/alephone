/*
	preprocess_map_shared.cpp

	Copyright (C) 2003 and beyond by Woody Zenfell, III
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

Feb. 3, 2003 (Woody Zenfell):
        Created.  Support for saving in netgames.
*/

#include "interface.h"


// The single-player quick save now works without presenting a dialog, and
// uses (cruder but) similar overwrite logic, so we just use that now.
bool
save_game_full_auto(bool inOverwriteRecent) {
        return save_game();
}
