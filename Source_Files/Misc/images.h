#ifndef __IMAGES_H
#define __IMAGES_H

/*

	images.h
	Thursday, July 20, 1995 3:30:51 PM- rdm created.
	

Aug 21, 2000 (Loren Petrich)
	Added file-object support

*/

void initialize_images_manager(void);

boolean images_picture_exists(short base_resource);
boolean scenario_picture_exists(short base_resource);

struct color_table *calculate_picture_clut(short pict_resource_number);
struct color_table *build_8bit_system_color_table(void);

void set_scenario_images_file(FileSpecifier& File);
// void set_scenario_images_file(FileDesc *file);

void draw_full_screen_pict_resource_from_images(short pict_resource_number);
void draw_full_screen_pict_resource_from_scenario(short pict_resource_number);

void scroll_full_screen_pict_resource_from_scenario(short pict_resource_number, boolean text_block);

// Places a MacOS resource handle into an appropriate wrapper object;
// a resource-fork emulator may put a pointer instead.
bool get_picture_resource_from_images(short base_resource, LoadedResource& PictRsrc);
bool get_picture_resource_from_scenario(short base_resource, LoadedResource& PictRsrc);

bool get_sound_resource_from_scenario(short resource_number, LoadedResource& SndRsrc);


/*
#ifdef mac
PicHandle get_picture_resource_from_images(short base_resource);
PicHandle get_picture_resource_from_scenario(short base_resource);

SndListHandle get_sound_resource_from_scenario(short resource_number);
#endif
*/

#endif

