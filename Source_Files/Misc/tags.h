#ifndef __TAGS_H
#define __TAGS_H

/*
	TAGS.H
	Sunday, July 3, 1994 5:33:15 PM

	This is a list of all of the tags used by code that uses the wad file format. 
	One tag, KEY_TAG, has special meaning, and KEY_TAG_SIZE must be set to the 
	size of an index entry.  Each wad can only have one index entry.  You can get the
	index entry from a wad, or from all of the wads in the file easily.
	
	Marathon uses the KEY_TAG as the name of the level.

Feb 2, 2000 (Loren Petrich):
	Changed application creator to 26.A "Aleph One"
	Changed soundfile type to 'snd∞' to be Marathon-Infinity compatible

Feb 3, 2000 (Loren Petrich):
	Changed shapes-file type to 'shp∞' to be Marathon-Infinity compatible

Feb 4, 2000 (Loren Petrich):
	Changed most of the other 2's to ∞'s to be Marathon-Infinity compatible,
	except for the map file type.

Feb 6, 2000 (Loren Petrich):
	Added loading of typecodes from the resource fork

Aug 21, 2000 (Loren Petrich):
	Added a preferences filetype

Aug 22, 2000 (Loren Petrich):
	Added an images filetype
*/

#define MAXIMUM_LEVEL_NAME_SIZE 64

/* OSTypes.. */
// LP change: moved values to filetypes_macintosh.c
enum {
	_typecode_creator,
	_typecode_scenario,
	_typecode_savegame,
	_typecode_film,
	_typecode_physics,
	_typecode_shapes,
	_typecode_sounds,
	_typecode_patch,
	_typecode_images,
	_typecode_preferences,
	NUMBER_OF_TYPECODES
};

// LP addition: typecode handling
// Initializer: loads from resource fork
void initialize_typecodes();
// Accessor
OSType get_typecode(int which);

// These are no longer constants, which will cause trouble for switch/case constructions
// These have been eliminated in favor of using the above enum of abstracted filetypes
// as much as possible
/*
#define APPLICATION_CREATOR (get_typecode(_typecode_creator))
#define SCENARIO_FILE_TYPE (get_typecode(_typecode_scenario))
#define SAVE_GAME_TYPE (get_typecode(_typecode_savegame))
#define FILM_FILE_TYPE (get_typecode(_typecode_film))
#define PHYSICS_FILE_TYPE (get_typecode(_typecode_physics))
#define SHAPES_FILE_TYPE (get_typecode(_typecode_shapes))
#define SOUNDS_FILE_TYPE (get_typecode(_typecode_sounds))
#define PATCH_FILE_TYPE (get_typecode(_typecode_patch))
#define IMAGES_FILE_TYPE (get_typecode(_typcode_images))
#define PREFERENCES_FILE_TYPE (get_typecode(_typecode_prefs))
*/

/* Other tags-  */
#define POINT_TAG 'PNTS'
#define LINE_TAG 'LINS'
#define SIDE_TAG 'SIDS'
#define POLYGON_TAG 'POLY'
#define LIGHTSOURCE_TAG 'LITE'
#define ANNOTATION_TAG 'NOTE'
#define OBJECT_TAG 'OBJS'
#define GUARDPATH_TAG 'påth'
#define MAP_INFO_TAG 'Minf'
#define ITEM_PLACEMENT_STRUCTURE_TAG 'plac'
#define DOOR_EXTRA_DATA_TAG 'door'
#define PLATFORM_STATIC_DATA_TAG 'plat'
#define ENDPOINT_DATA_TAG 'EPNT'
#define MEDIA_TAG 'medi'
#define AMBIENT_SOUND_TAG 'ambi'
#define RANDOM_SOUND_TAG 'bonk'
#define TERMINAL_DATA_TAG 'term'

/* Save/Load game tags. */
#define PLAYER_STRUCTURE_TAG 'plyr'
#define DYNAMIC_STRUCTURE_TAG 'dwol'
#define OBJECT_STRUCTURE_TAG 'mobj'
#define DOOR_STRUCTURE_TAG 'door'
#define MAP_INDEXES_TAG 'iidx'
#define AUTOMAP_LINES 'alin'
#define AUTOMAP_POLYGONS 'apol'
#define MONSTERS_STRUCTURE_TAG 'mOns'
#define EFFECTS_STRUCTURE_TAG 'fx  '
#define PROJECTILES_STRUCTURE_TAG 'bang'
#define PLATFORM_STRUCTURE_TAG 'PLAT'
#define WEAPON_STATE_TAG 'weap'
#define TERMINAL_STATE_TAG 'cint'

/* Physix model tags */
#define MONSTER_PHYSICS_TAG 'MNpx'
#define EFFECTS_PHYSICS_TAG 'FXpx'
#define PROJECTILE_PHYSICS_TAG 'PRpx'
#define PHYSICS_PHYSICS_TAG 'PXpx'
#define WEAPONS_PHYSICS_TAG 'WPpx'

/* Preferences Tags.. */
#define prefGRAPHICS_TAG 'graf'
#define prefSERIAL_TAG 'serl'
#define prefNETWORK_TAG 'netw'
#define prefPLAYER_TAG 'plyr'
#define prefINPUT_TAG 'inpu'
#define prefSOUND_TAG 'snd '
#define prefENVIRONMENT_TAG 'envr'

#endif
