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

#include "FileHandler.h"

void initialize_images_manager(void);

boolean images_picture_exists(short base_resource);
boolean scenario_picture_exists(short base_resource);

// Modified to select what resource file is to be the source of the color table;
// this is for the benefit of resource-file 
enum
{
	CLUTSource_Images,
	CLUTSource_Scenario
};
struct color_table *calculate_picture_clut(int CLUTSource, short pict_resource_number);
// struct color_table *calculate_picture_clut(short pict_resource_number);
struct color_table *build_8bit_system_color_table(void);

void set_scenario_images_file(FileSpecifier& File);

void draw_full_screen_pict_resource_from_images(short pict_resource_number);
void draw_full_screen_pict_resource_from_scenario(short pict_resource_number);

void scroll_full_screen_pict_resource_from_scenario(short pict_resource_number, boolean text_block);

#ifdef mac

// Places a MacOS resource handle into an appropriate wrapper object;
// a resource-fork emulator may put a pointer instead.
bool get_picture_resource_from_images(short base_resource, LoadedResource& PictRsrc);
bool get_picture_resource_from_scenario(short base_resource, LoadedResource& PictRsrc);

bool get_sound_resource_from_scenario(short resource_number, LoadedResource& SndRsrc);

#elif defined(SDL)

extern void *get_picture_resource_from_images(short base_resource, uint32 &size);
extern void *get_picture_resource_from_scenario(short base_resource, uint32 &size);

extern void *get_sound_resource_from_scenario(short resource_number, uint32 &size);

#endif

#endif

