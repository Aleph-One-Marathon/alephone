#ifndef __IMAGES_H
#define __IMAGES_H

/*

	images.h
	Thursday, July 20, 1995 3:30:51 PM- rdm created.
	

Aug 21, 2000 (Loren Petrich)
	Added CLUT-source selector

Aug 26, 2000 (Loren Petrich)
	Added object-oriented file handling
*/

// LP: CodeWarrior complains unless I give the full definition of these classes
#include "FileHandler.h"
// class FileSpecifier;
// class LoadedResource;

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

extern void draw_full_screen_pict_resource_from_images(int pict_resource_number);
extern void draw_full_screen_pict_resource_from_scenario(int pict_resource_number);

extern void scroll_full_screen_pict_resource_from_scenario(int pict_resource_number, bool text_block);

// Places a MacOS resource handle into an appropriate wrapper object;
// a resource-fork emulator may put a pointer instead.
extern bool get_picture_resource_from_images(int base_resource, LoadedResource& PictRsrc);
extern bool get_picture_resource_from_scenario(int base_resource, LoadedResource& PictRsrc);

extern bool get_sound_resource_from_scenario(int resource_number, LoadedResource& SndRsrc);

#ifdef SDL
// Convert MacOS PICT resource to SDL surface
extern SDL_Surface *picture_to_surface(LoadedResource &rsrc);

// Rescale/tile surface
extern SDL_Surface *rescale_surface(SDL_Surface *s, int width, int height);
extern SDL_Surface *tile_surface(SDL_Surface *s, int width, int height);
#endif

#endif

