#ifndef __GAME_WAD_H
#define __GAME_WAD_H

/*
	GAME_WAD.H
	Sunday, July 3, 1994 10:45:56 PM

Jan 30, 2000 (Loren Petrich)
	Changed "new" to "_new" to make data structures more C++-friendly
*/

#include "portable_files.h"

/* -------------------- Private or map editor functions */
void allocate_map_for_counts(short polygon_count, short side_count,
	short endpoint_count, short line_count, long terminal_data_length);
void load_points(saved_map_pt *points, short count);
void load_lines(saved_line *lines, short count);
void load_sides(saved_side *sides, short count, short version);
void load_polygons(saved_poly *polys, short count, short version);
void load_lights(struct static_light_data *lights, short count, short version);
void load_annotations(saved_annotation *annotations, short count);
void load_objects(saved_object *map_objects, short count);
void load_media(struct media_data *media, short count);
void load_map_info(saved_map_data *map_info);
void load_ambient_sound_images(struct ambient_sound_image_data *data, short count);
void load_random_sound_images(struct random_sound_image_data *data, short count);
void load_terminal_data(byte *data, long length);

/* Used _ONLY_ by game_wad.c internally and precalculate.c. */
boolean process_map_wad(struct wad_data *wad, boolean restoring_game, short version);

/* Final three calls, must be in this order! */
void recalculate_redundant_map(void);
void scan_and_add_platforms(struct static_platform_data *platform_static_data, short count);
void complete_loading_level(short *map_indexes, short map_index_count, 
	struct static_platform_data *platform_data, short platform_data_count,
	struct platform_data *actual_platform_data, short actual_platform_data_count, short version);

boolean save_game_file(FileDesc *file);

/* -------------- New functions */
void pause_game(void);
void resume_game(void);
void get_current_saved_game_name(unsigned char *file_name);

boolean match_checksum_with_map(short vRefNum, long dirID, unsigned long checksum, 
	FileDesc *file);
void set_map_file(FileDesc *file);

/* --------- from PREPROCESS_MAP_MAC.C */
void get_default_map_spec(FileDesc *_new);
void get_default_physics_spec(FileDesc *_new);

void add_finishing_touches_to_save_file(FileDesc *file);

#endif
