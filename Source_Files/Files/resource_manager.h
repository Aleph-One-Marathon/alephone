/*

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

*/

/*
 *  resource_manager.h - MacOS resource handling for non-Mac platforms
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "cstypes.h"
#include <stdio.h>
#include <vector>
#include <SDL.h>

#ifndef NO_STD_NAMESPACE
using std::vector;
#endif

class FileSpecifier;
class LoadedResource;

extern void initialize_resources(void);

extern SDL_RWops *open_res_file(FileSpecifier &file);
extern void close_res_file(SDL_RWops *file);
extern SDL_RWops *open_res_file_from_rwops(SDL_RWops *file);
extern SDL_RWops *cur_res_file(void);
extern void use_res_file(SDL_RWops *file);

extern size_t count_1_resources(uint32 type);
extern size_t count_resources(uint32 type);

extern void get_1_resource_id_list(uint32 type, vector<int> &ids);
extern void get_resource_id_list(uint32 type, vector<int> &ids);

extern bool get_1_resource(uint32 type, int id, LoadedResource &rsrc);
extern bool get_resource(uint32 type, int id, LoadedResource &rsrc);

extern bool get_1_ind_resource(uint32 type, int index, LoadedResource &rsrc);
extern bool get_ind_resource(uint32 type, int index, LoadedResource &rsrc);

extern bool has_1_resource(uint32 type, int id);
extern bool has_resource(uint32 type, int id);

#endif
