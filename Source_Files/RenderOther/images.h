#ifndef __IMAGES_H
#define __IMAGES_H

/*
	images.h

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

	Thursday, July 20, 1995 3:30:51 PM- rdm created.
	

Aug 21, 2000 (Loren Petrich)
	Added CLUT-source selector

Aug 26, 2000 (Loren Petrich)
	Added object-oriented file handling

Jul 31, 2002 (Loren Petrich)
	Added text-resource access in analogy with others' image- and sound-resource access;
	this is for supporting the M2-Win95 file format
*/

// LP: CodeWarrior complains unless I give the full definition of these classes
#include "FileHandler.h"

#include <memory>

extern void initialize_images_manager(void);

extern bool images_picture_exists(int base_resource);
extern bool scenario_picture_exists(int base_resource);

// Select what resource file is to be the source of the color table;
// this is for the benefit of resource-file 
enum
{
	CLUTSource_Images,
	CLUTSource_Scenario
};
extern struct color_table *calculate_picture_clut(int CLUTSource, int pict_resource_number);
extern struct color_table *build_8bit_system_color_table(void);

extern void set_scenario_images_file(FileSpecifier& File);
extern void unset_scenario_images_file();
extern void set_shapes_images_file(FileSpecifier& File);
extern void set_external_resources_images_file(FileSpecifier& File);
extern void set_sounds_images_file(FileSpecifier& File);

extern void draw_full_screen_pict_resource_from_images(int pict_resource_number);
extern void draw_full_screen_pict_resource_from_scenario(int pict_resource_number);

extern void scroll_full_screen_pict_resource_from_scenario(int pict_resource_number, bool text_block);

// Places a MacOS resource handle into an appropriate wrapper object;
// a resource-fork emulator may put a pointer instead.
extern bool get_picture_resource_from_images(int base_resource, LoadedResource& PictRsrc);
extern bool get_sound_resource_from_images(int resource_number, LoadedResource& PictRsrc);
extern bool get_picture_resource_from_scenario(int base_resource, LoadedResource& PictRsrc);

extern bool get_sound_resource_from_scenario(int resource_number, LoadedResource& SoundRsrc);
extern bool get_text_resource_from_scenario(int resource_number, LoadedResource& TextRsrc);

// Convert MacOS PICT resource to SDL surface
extern std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> picture_to_surface(LoadedResource &rsrc);

// Rescale/tile surface
extern SDL_Surface *rescale_surface(SDL_Surface *s, int width, int height);
extern SDL_Surface *tile_surface(SDL_Surface *s, int width, int height);

#endif

