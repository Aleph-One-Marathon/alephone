#ifndef __EXTENSIONS_H
#define __EXTENSIONS_H

/*
	extensions.h

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Tuesday, October 31, 1995 11:42:19 AM- rdm created.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

#include "cstypes.h"

class FileSpecifier;

#define BUNGIE_PHYSICS_DATA_VERSION 0
#define PHYSICS_DATA_VERSION 1

/* ------------- prototypes */

/* Set the physics file to read from.. */
void set_physics_file(FileSpecifier& File);

void set_to_default_physics_file(void);

/* Proceses the entire physics file.. */
void import_definition_structures(void);

void *get_network_physics_buffer(int32 *physics_length);
void process_network_physics_model(void *data);

#endif
