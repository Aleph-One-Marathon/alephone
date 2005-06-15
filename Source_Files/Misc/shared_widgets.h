
/*

	Copyright (C) 2005 and beyond by the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/

/*
 *  shared_widgets.h - Widgets for carbon and sdl dialogs
 */

// This is where high level widgets with shared implementations on SDL and Carbon can go.
// Currently we have none of those, so all this does is choose which widget header to include.

#ifndef SHARED_WIDGETS_H
#define SHARED_WIDGETS_H

#include "cseries.h"

#ifdef SDL
#include "sdl_widgets.h"
#else
#include "carbon_widgets.h"
#endif

#endif
