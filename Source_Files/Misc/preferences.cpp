/*

	preferences.c
	Tuesday, June 13, 1995 10:02:50 AM- rdm created.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 6, 2000 (Loren Petrich):
	Changed switch/case to if/then/elseif/else for typecodes

Feb 10, 2000 (Loren Petrich):
	Added code to do run/walk and swim/sink switching

Feb 11, 2000 (Loren Petrich):
	Made directory searches recursive;
	also increased MAXIMUM_FIND_FILES to 128

Feb 12, 2000 (Loren Petrich):
	Changed run/walk interchange default to on;
	swim/sink interchange default remains off.

Feb 25, 2000 (Loren Petrich):
	Set up dialog boxes for handling the chase-cam and crosshair parameters

Mar 19, 2000 (Loren Petrich):
	Added OpenGL support

Apr 27, 2000 (Loren Petrich):
	Added Josh Elsasser's "don't switch weapons" patch

May 2, 2000 (Loren Petrich):
	Eliminated "draw every other line" dialog-box option (already grayed out for PowerPC).

May 3, 2000 (Loren Petrich):
	Added Simon Brownlee's patch for handling the "hardware acceleration" button

Jul 7, 2000 (Loren Petrich): Added Ben Thompson's InputSprocket support

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 25, 2000 (Loren Petrich)
	Confined searches to the app's directory
*/

#include <string.h>
#include <stdlib.h>

#include "cseries.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>	// for getlogin()
#endif

#include "map.h"
#include "shell.h" /* For the screen_mode structure */
#include "interface.h"
#include "mysound.h"
#include "ISp_Support.h" /* BT: Added April 16, 2000 for Input Sprocket Support */

#include "preferences.h"
#include "wad.h"
#include "wad_prefs.h"
#include "game_errors.h"
#include "network.h" // for _ethernet, etc.
#include "find_files.h"
#include "game_wad.h" // for set_map_file
#include "screen.h"
#include "fades.h"
#include "extensions.h"

#include "tags.h"
#include "FileHandler.h"

#ifdef env68k
	#pragma segment dialogs
#endif

enum {
	ditlGRAPHICS= 4001,
	iCHOOSE_MONITOR=1,
	// LP change: finally eliminted this option
	// iDRAW_EVERY_OTHER_LINE,
	iHARDWARE_ACCELERATION,
	iNUMBER_OF_COLORS,
	iWINDOW_SIZE,
	iDETAIL,
	iBRIGHTNESS,
	iOPENGL_OPTIONS	// LP addition: OpenGL-options button
};

enum {
	ditlPLAYER= 4002,
	iDIFFICULTY_LEVEL= 1,
	iNAME,
	iCOLOR,
	iTEAM,
	// LP additions:
	iCHASE_CAM = 7,
	iCROSSHAIRS
};

enum {
	ditlSOUND= 4003,
	iSTEREO= 1,
	iACTIVE_PANNING,
	iHIGH_QUALITY,
	iAMBIENT_SOUND,
	iVOLUME,
	iCHANNELS,
	iMORE_SOUNDS
};

// LP additions: InputSprocket, run/walk and swim/sink reversers;
// renamed "iSET_KEYS" to "iSET_INPUTS".
// LP: added Josh Elsasser's don't-switch-weapons patch
// LP: added separate InputSprocket button
enum {
	ditlINPUT= 4004,
	iMOUSE_CONTROL= 1,
	iKEYBOARD_CONTROL,
	iINPUT_SPROCKET_CONTROL,
	iSET_KEYS,
	iINTERCHANGE_RUN_WALK,
	iINTERCHANGE_SWIM_SINK,
	iDONT_SWITCH_TO_NEW_WEAPON,
	iSET_INPUT_SPROCKET
};

enum {
	ditlENVIRONMENT= 4005,
	iMAP= 1,
	iPHYSICS,
	iSHAPES,
	iSOUNDS,
	iPATCHES_LIST
};

enum {
	strPREFERENCES_GROUPS= 139,
	graphicsGroup= 0,
	playerGroup,
	soundGroup,
	inputGroup,
	environmentGroup
};

struct graphics_preferences_data *graphics_preferences;
struct serial_number_data *serial_preferences;
struct network_preferences_data *network_preferences;
struct player_preferences_data *player_preferences;
struct input_preferences_data *input_preferences;
struct sound_manager_parameters *sound_preferences;
struct environment_preferences_data *environment_preferences;

/* ----------- private prototypes */
static void default_graphics_preferences(void *preferences);
static boolean validate_graphics_preferences(void *preferences);
static void default_serial_number_preferences(void *prefs);
static boolean validate_serial_number_preferences(void *prefs);
static void default_network_preferences(void *preferences);
static boolean validate_network_preferences(void *prefs);
static void default_player_preferences(void *prefs);
static boolean validate_player_preferences(void *prefs);
static void default_input_preferences(void *preferences);
static boolean validate_input_preferences(void *prefs);
static void *get_graphics_pref_data(void);
static void setup_graphics_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_graphics_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static boolean teardown_graphics_dialog(DialogPtr dialog, short first_item, void *prefs);
static void *get_player_pref_data(void);
static void setup_player_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_player_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static boolean teardown_player_dialog(DialogPtr dialog, short first_item, void *prefs);
static void *get_sound_pref_data(void);
static void setup_sound_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_sound_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static boolean teardown_sound_dialog(DialogPtr dialog, short first_item, void *prefs);
static void *get_input_pref_data(void);
static void setup_input_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_input_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static boolean teardown_input_dialog(DialogPtr dialog, short first_item, void *prefs);
static void set_popup_enabled_state(DialogPtr dialog, short item_number, short item_to_affect,
	boolean enabled);
static void default_environment_preferences(void *prefs);
static boolean validate_environment_preferences(void *prefs);
static void *get_environment_pref_data(void);
static void setup_environment_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_environment_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static boolean teardown_environment_dialog(DialogPtr dialog, short first_item, void *prefs);
static void fill_in_popup_with_filetype(DialogPtr dialog, short item, int type, unsigned long checksum);
// static void fill_in_popup_with_filetype(DialogPtr dialog, short item, OSType type, unsigned long checksum);
static MenuHandle get_popup_menu_handle(DialogPtr dialog, short item);
static boolean allocate_extensions_memory(void);
static void free_extensions_memory(void);
static void build_extensions_list(void);
static void search_from_directory(DirectorySpecifier& BaseDir);
// static void search_from_directory(FSSpec *file);
static unsigned long find_checksum_and_file_spec_from_dialog(DialogPtr dialog, 
	short item_hit, OSType type, FSSpec *file);
static void	rebuild_patchlist(DialogPtr dialog, short item, unsigned long parent_checksum,
	struct environment_preferences_data *preferences);

// LP: turned into a file-specifier method
// static unsigned long get_file_modification_date(FSSpec *file);

// LP: fake portable-files stuff
#ifdef mac
inline short memory_error() {return MemError();}
#else
inline short memory_error() {return 0;}
#endif

/* ---------------- code */
void initialize_preferences(
	void)
{
	OSErr err;

	if(!w_open_preferences_file(getcstr(temporary, strFILENAMES, filenamePREFERENCES),
		_typecode_preferences))
	// if(!w_open_preferences_file(getpstr(ptemporary, strFILENAMES, filenamePREFERENCES),
	// 	PREFERENCES_TYPE))
	{
		/* Major memory error.. */
		alert_user(fatalError, strERRORS, outOfMemory, memory_error());
	}

	if(error_pending())
	{
		short type;
		
		err= get_game_error(&type);
		dprintf("Er: %d type: %d", err, type);
		set_game_error(systemError, noErr);
	}
	
	/* If we didn't open, we initialized.. */
	graphics_preferences= (struct graphics_preferences_data *)get_graphics_pref_data();
	player_preferences= (struct player_preferences_data *)get_player_pref_data();
	input_preferences= (struct input_preferences_data *)get_input_pref_data();
	sound_preferences= (struct sound_manager_parameters *)get_sound_pref_data();
	serial_preferences= (struct serial_number_data *)w_get_data_from_preferences(
		prefSERIAL_TAG,sizeof(struct serial_number_data),
		default_serial_number_preferences,
		validate_serial_number_preferences);
	network_preferences= (struct network_preferences_data *)w_get_data_from_preferences(
		prefNETWORK_TAG, sizeof(struct network_preferences_data),
		default_network_preferences,
		validate_network_preferences);
	environment_preferences= (struct environment_preferences_data *)get_environment_pref_data();
}

struct preferences_dialog_data prefs_data[]={
	{ strPREFERENCES_GROUPS, graphicsGroup, ditlGRAPHICS, get_graphics_pref_data, setup_graphics_dialog, hit_graphics_item, teardown_graphics_dialog },
	{ strPREFERENCES_GROUPS, playerGroup, ditlPLAYER, get_player_pref_data, setup_player_dialog, hit_player_item, teardown_player_dialog },
	{ strPREFERENCES_GROUPS, soundGroup, ditlSOUND, get_sound_pref_data, setup_sound_dialog, hit_sound_item, teardown_sound_dialog },
	{ strPREFERENCES_GROUPS, inputGroup, ditlINPUT, get_input_pref_data, setup_input_dialog, hit_input_item, teardown_input_dialog },
#ifndef DEMO
	{ strPREFERENCES_GROUPS, environmentGroup, ditlENVIRONMENT, get_environment_pref_data, setup_environment_dialog, hit_environment_item, teardown_environment_dialog }
#endif
};
#define NUMBER_OF_PREFS_PANELS (sizeof(prefs_data)/sizeof(struct preferences_dialog_data))

void handle_preferences(
	void)
{
	/* Save the existing preferences, in case we have to reload them. */
	write_preferences();

	if(set_preferences(prefs_data, NUMBER_OF_PREFS_PANELS, initialize_preferences))
	{
		/* Save the new ones. */
		write_preferences();
		set_sound_manager_parameters(sound_preferences);
		load_environment_from_preferences();
	}
	
	return;
}

void write_preferences(
	void)
{
	OSErr err;
	w_write_preferences_file();

	if(error_pending())
	{
		short type;
		
		err= get_game_error(&type);
		dprintf("Er: %d type: %d", err, type);
		set_game_error(systemError, noErr);
	}
}

/* ------------- private prototypes */
static void default_graphics_preferences(
	void *prefs)
{
	struct graphics_preferences_data *preferences=(struct graphics_preferences_data *)prefs;

#ifdef mac
	preferences->device_spec.slot= NONE;
	preferences->device_spec.flags= deviceIsColor;
#endif
	preferences->device_spec.bit_depth= 8;
	preferences->device_spec.width= 640;
	preferences->device_spec.height= 480;

	preferences->screen_mode.gamma_level= DEFAULT_GAMMA_LEVEL;
	if (hardware_acceleration_code(&preferences->device_spec) == _valkyrie_acceleration)
	{
		preferences->screen_mode.size= _100_percent;
		preferences->screen_mode.bit_depth = 16;
		preferences->screen_mode.high_resolution = FALSE;
		preferences->screen_mode.acceleration = _valkyrie_acceleration;
	}
	else if (system_information->machine_is_68k)
	{
		preferences->screen_mode.size= _100_percent;
		preferences->screen_mode.high_resolution= FALSE;
		preferences->screen_mode.acceleration = _no_acceleration;
		preferences->screen_mode.bit_depth = 8;
	}
	else // we got a good machine
	{
		preferences->screen_mode.size= _100_percent;
		preferences->screen_mode.high_resolution= TRUE;
		preferences->screen_mode.acceleration = _no_acceleration;
		preferences->screen_mode.bit_depth = 8;
	}
	
	preferences->screen_mode.draw_every_other_line= FALSE;
	
#ifdef HAVE_OPENGL
	OGL_SetDefaults(preferences->OGL_Configure);
#endif
}

static boolean validate_graphics_preferences(
	void *prefs)
{
	struct graphics_preferences_data *preferences=(struct graphics_preferences_data *)prefs;
	boolean changed= FALSE;

	if(preferences->screen_mode.gamma_level<0 || preferences->screen_mode.gamma_level>=NUMBER_OF_GAMMA_LEVELS)
	{
		preferences->screen_mode.gamma_level= DEFAULT_GAMMA_LEVEL;
		changed= TRUE;
	}

	if (preferences->screen_mode.acceleration==_valkyrie_acceleration)
	{
		if (hardware_acceleration_code(&preferences->device_spec) != _valkyrie_acceleration)
		{
			preferences->screen_mode.size= _100_percent;
			preferences->screen_mode.bit_depth = 8;
			preferences->screen_mode.high_resolution = FALSE;
			preferences->screen_mode.acceleration = _no_acceleration;
			changed= TRUE;
		} else {
			if(preferences->screen_mode.high_resolution)
			{
				preferences->screen_mode.high_resolution= FALSE;
				changed= TRUE;
			}
			
			if(preferences->screen_mode.bit_depth != 16)
			{
				preferences->screen_mode.bit_depth= 16;
				changed= TRUE;
			}
			
			if(preferences->screen_mode.draw_every_other_line)
			{
				preferences->screen_mode.draw_every_other_line= FALSE;
				changed= TRUE;
			}
		}
	}

	if (preferences->screen_mode.bit_depth==32 && !machine_supports_32bit(&preferences->device_spec)) 
	{
		preferences->screen_mode.bit_depth= 16;
		changed= TRUE;
	}

	/* Don't change out of 16 bit if we are in valkyrie mode. */	
	if (preferences->screen_mode.acceleration!=_valkyrie_acceleration
		&& preferences->screen_mode.bit_depth==16 && !machine_supports_16bit(&preferences->device_spec)) 
	{
		preferences->screen_mode.bit_depth= 8;
		changed= TRUE;
	}

	return changed;
}

static void default_serial_number_preferences(
	void *prefs)
{
	memset(prefs, 0, sizeof(struct serial_number_data));
}

static boolean validate_serial_number_preferences(
	void *prefs)
{
	(void) (prefs);
	return FALSE;
}

/* -------------- network preferences */
static boolean ethernet_active(void);

static void default_network_preferences(
	void *prefs)
{
	struct network_preferences_data *preferences=(struct network_preferences_data *)prefs;

	preferences->type= _ethernet;

	preferences->allow_microphone = TRUE;
	preferences->game_is_untimed = FALSE;
	preferences->difficulty_level = 2;
	preferences->game_options =	_multiplayer_game | _ammo_replenishes | _weapons_replenish
		| _specials_replenish |	_monsters_replenish | _burn_items_on_death | _suicide_is_penalized 
		| _force_unique_teams | _live_network_stats;
	preferences->time_limit = 10 * TICKS_PER_SECOND * 60;
	preferences->kill_limit = 10;
	preferences->entry_point= 0;
	preferences->game_type= _game_of_kill_monsters;
	
	return;
}

static boolean validate_network_preferences(
	void *preferences)
{
	struct network_preferences_data *prefs=(struct network_preferences_data *)preferences;
	boolean changed= FALSE;

	if(prefs->type<0||prefs->type>_ethernet)
	{
		if(ethernet_active())
		{
			prefs->type= _ethernet;
		} else {
			prefs->type= _localtalk;
		}
		changed= TRUE;
	}
	
	if(prefs->game_is_untimed != TRUE && prefs->game_is_untimed != FALSE)
	{
		prefs->game_is_untimed= FALSE;
		changed= TRUE;
	}

	if(prefs->allow_microphone != TRUE && prefs->allow_microphone != FALSE)
	{
		prefs->allow_microphone= TRUE;
		changed= TRUE;
	}

	if(prefs->game_type<0 || prefs->game_type >= NUMBER_OF_GAME_TYPES)
	{
		prefs->game_type= _game_of_kill_monsters;
		changed= TRUE;
	}
	
	return changed;
}

/* ------------- player preferences */
static void get_name_from_system(unsigned char *name);

static void default_player_preferences(
	void *preferences)
{
	struct player_preferences_data *prefs=(struct player_preferences_data *)preferences;

	obj_clear(*prefs);

#ifdef mac
	GetDateTime(&prefs->last_time_ran);
#endif
	prefs->difficulty_level= 2;
	get_name_from_system(prefs->name);
	
	// LP additions for new fields:
	
	prefs->ChaseCam.Behind = 1536;
	prefs->ChaseCam.Upward = 0;
	prefs->ChaseCam.Rightward = 0;
	prefs->ChaseCam.Flags = 0;
	
	prefs->Crosshairs.Thickness = 2;
	prefs->Crosshairs.FromCenter = 8;
	prefs->Crosshairs.Length = 16;
	prefs->Crosshairs.Color = rgb_white;
	
	return;
}

static boolean validate_player_preferences(
	void *prefs)
{
	(void) (prefs);
	return FALSE;
}

#define strUSER_NAME -16096

static void get_name_from_system(
	unsigned char *name)
{
#if defined(mac)
	StringHandle name_handle;
	char old_state;

	name_handle= GetString(strUSER_NAME);
	assert(name_handle);
	
	old_state= HGetState((Handle)name_handle);
	HLock((Handle)name_handle);
	
	pstrcpy(name, *name_handle);
	HSetState((Handle)name_handle, old_state);
#elif defined(__unix__) || defined(__BEOS__)
	char *login = getlogin();
	strcpy((char *)name, login ? login : "Bob User");
#else
#error get_name_from_system() needs to be implemented for this platform
#endif
}

/* ------------ input preferences */
static void default_input_preferences(
	void *prefs)
{
	struct input_preferences_data *preferences=(struct input_preferences_data *)prefs;

	preferences->input_device= _keyboard_or_game_pad;
	set_default_keys(preferences->keycodes, _standard_keyboard_setup);
	
	// LP addition: set up defaults for modifiers:
	// interchange run and walk, but don't interchange swim and sink.
	preferences->modifiers = _inputmod_interchange_run_walk;
}

static boolean validate_input_preferences(
	void *prefs)
{
	(void) (prefs);
	return FALSE;
}

static boolean ethernet_active(
	void)
{
#ifdef mac
	short  refnum;
	OSErr  error;

	error= OpenDriver("\p.ENET", &refnum);
	
	return error==noErr ? TRUE : FALSE;
#else
	return TRUE;
#endif
}

/* ------------- dialog functions */

/* --------- graphics */
static void *get_graphics_pref_data(
	void)
{
	return w_get_data_from_preferences(
		prefGRAPHICS_TAG, sizeof(struct graphics_preferences_data),
		default_graphics_preferences,
		validate_graphics_preferences);
}

enum {
	_hundreds_colors_menu_item= 1,
	_thousands_colors_menu_item,
	_millions_colors_menu_item,
	_billions_colors_menu_item
};

static void setup_graphics_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
#ifdef mac
	struct graphics_preferences_data *preferences= (struct graphics_preferences_data *) prefs;
	short value, active;

	if(machine_supports_32bit(&preferences->device_spec))
	{
		active= TRUE;
	} else {
		if(preferences->screen_mode.bit_depth==32) preferences->screen_mode.bit_depth= 16;
		active= FALSE;
	}
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _millions_colors_menu_item, active);

	if(machine_supports_16bit(&preferences->device_spec))
	{
		active= TRUE;
	} else {
		if(preferences->screen_mode.bit_depth==16) preferences->screen_mode.bit_depth= 8;
		active= FALSE;
	}
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _thousands_colors_menu_item, active);
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _billions_colors_menu_item, FALSE);

	/* Force the stuff for the valkyrie board.. */
	if(preferences->screen_mode.acceleration==_valkyrie_acceleration)
	{
		preferences->screen_mode.high_resolution= FALSE;
		preferences->screen_mode.bit_depth= 16;
		preferences->screen_mode.draw_every_other_line= FALSE;
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), 
			CONTROL_INACTIVE, _thousands_colors_menu_item);
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDETAIL, first_item), 
			CONTROL_INACTIVE, NONE);

		/* You can't choose a monitor with hardware acceleration enabled.. */
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iCHOOSE_MONITOR, first_item), 
			CONTROL_INACTIVE, NONE);
	} else {
		/* Make sure it is enabled. */
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), 
			CONTROL_ACTIVE, NONE);
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDETAIL, first_item), 
			CONTROL_ACTIVE, NONE);
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iCHOOSE_MONITOR, first_item), 
			CONTROL_ACTIVE, NONE);
	}
	
	switch(preferences->screen_mode.bit_depth)
	{
		case 32: value= _millions_colors_menu_item; break;
		case 16: value= _thousands_colors_menu_item; break;
		case 8:	 value= _hundreds_colors_menu_item; break;
		default: value= _hundreds_colors_menu_item;
			// LP change:
			assert(false);
			// halt();
	}
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), NONE, value);

#ifdef env68k
	if(preferences->screen_mode.bit_depth != 8 || preferences->screen_mode.high_resolution)
	{
		preferences->screen_mode.draw_every_other_line= FALSE;
		active= CONTROL_INACTIVE;
	} else {
		active= CONTROL_ACTIVE;
	}
#else
	preferences->screen_mode.draw_every_other_line= FALSE;
	active= CONTROL_INACTIVE;
#endif
/*
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDRAW_EVERY_OTHER_LINE, first_item), 
		active, preferences->screen_mode.draw_every_other_line);
*/
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iWINDOW_SIZE, first_item), NONE, 
		preferences->screen_mode.size+1);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDETAIL, first_item), NONE, 
		!preferences->screen_mode.high_resolution+1); 
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iBRIGHTNESS, first_item), NONE, 
		preferences->screen_mode.gamma_level+1);
	
	// LP change: adding OpenGL support
	// Simon Brownlee patch: did correct handling of "active"
	active = OGL_IsPresent() ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	if (active!=CONTROL_ACTIVE) preferences->screen_mode.acceleration = _no_acceleration;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iHARDWARE_ACCELERATION, first_item), 
		active, (preferences->screen_mode.acceleration == _opengl_acceleration));
	/*
	active = (hardware_acceleration_code(&preferences->device_spec) == _valkyrie_acceleration) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iHARDWARE_ACCELERATION, first_item), 
		active, (preferences->screen_mode.acceleration == _valkyrie_acceleration));
	*/
		
	return;
#endif
}

static void hit_graphics_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
#ifdef mac
	struct graphics_preferences_data *preferences= (struct graphics_preferences_data *) prefs;
	ControlHandle control;
	short item_type;
	Rect bounds;
	boolean resetup= TRUE;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iCHOOSE_MONITOR:
			display_device_dialog(&preferences->device_spec);
			/* We resetup because the new device might not support millions, etc.. */
			break;
		/*
		case iDRAW_EVERY_OTHER_LINE:
			preferences->screen_mode.draw_every_other_line= !preferences->screen_mode.draw_every_other_line;
			break;
		*/	
		case iHARDWARE_ACCELERATION:
			// LP change: added OpenGL support here
			if(preferences->screen_mode.acceleration == _opengl_acceleration)
			{
				preferences->screen_mode.acceleration= _no_acceleration;
			} else {
				preferences->screen_mode.acceleration= _opengl_acceleration;
			}
			/*
			preferences->screen_mode.draw_every_other_line= !preferences->screen_mode.draw_every_other_line;
			if(preferences->screen_mode.acceleration == _valkyrie_acceleration)
			{
				preferences->screen_mode.acceleration= _no_acceleration;
			} else {
				preferences->screen_mode.acceleration= _valkyrie_acceleration;
			}
			*/
			break;
			
		case iNUMBER_OF_COLORS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			switch(GetControlValue(control))
			{
				case _hundreds_colors_menu_item: preferences->screen_mode.bit_depth= 8; break;
				case _thousands_colors_menu_item: preferences->screen_mode.bit_depth= 16; break;
				case _millions_colors_menu_item: preferences->screen_mode.bit_depth= 32; break;
				case _billions_colors_menu_item:
				default: preferences->screen_mode.bit_depth= 8;
					// LP change:
					assert(false);
					// halt();
			}
			break;

		case iWINDOW_SIZE:		
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->screen_mode.size= GetControlValue(control)-1;
			break;
			
		case iDETAIL:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->screen_mode.high_resolution= !(GetControlValue(control)-1);
			break;
		
		case iBRIGHTNESS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->screen_mode.gamma_level= GetControlValue(control)-1;
			break;
		
		case iOPENGL_OPTIONS:
			OGL_ConfigureDialog(preferences->OGL_Configure);
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	if(resetup)
	{
		setup_graphics_dialog(dialog, first_item, prefs);
	}
#endif
}
	
static boolean teardown_graphics_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void) (dialog, first_item, prefs);
	return TRUE;
}

/* --------- player */
static void *get_player_pref_data(
	void)
{
	return w_get_data_from_preferences(
		prefPLAYER_TAG,sizeof(struct player_preferences_data),
		default_player_preferences,
		validate_player_preferences);
}

static void setup_player_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
#ifdef mac
	struct player_preferences_data *preferences= (struct player_preferences_data *)prefs;
	Handle item;
	short item_type;
	Rect bounds;

	/* Setup the difficulty level */
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDIFFICULTY_LEVEL, first_item), NONE, 
		preferences->difficulty_level+1);

	/* Setup the name. */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), &item_type, &item, &bounds);
	SetDialogItemText(item, preferences->name);
	SelectDialogItemText(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), 0, SHORT_MAX);

	/* Setup the color */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iCOLOR, first_item), &item_type, &item, &bounds);
	SetControlValue((ControlHandle) item, preferences->color+1);

	/* Setup the team */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iTEAM, first_item), &item_type, &item, &bounds);
	SetControlValue((ControlHandle) item, preferences->team+1);
#endif
}

static void hit_player_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
#ifdef mac
	ControlHandle control;
	short item_type;
	Rect bounds;
	struct player_preferences_data *preferences= (struct player_preferences_data *)prefs;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iDIFFICULTY_LEVEL:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->difficulty_level= GetControlValue(control)-1;
			break;

		case iCOLOR:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->color= GetControlValue(control)-1;
			break;

		case iTEAM:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->team= GetControlValue(control)-1;
			break;
		
		// LP additions:
		case iCHASE_CAM:
			Configure_ChaseCam(preferences->ChaseCam);
			break;
			
		case iCROSSHAIRS:
			Configure_Crosshairs(preferences->Crosshairs);
			break;
	}
#endif
}
	
static boolean teardown_player_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
#ifdef mac
	Handle control;
	short item_type;
	Rect bounds;
	struct player_preferences_data *preferences= (struct player_preferences_data *)prefs;
	Str255 buffer;
	
	/* Get the player name */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), &item_type, &control, &bounds);
	GetDialogItemText(control, buffer);
	if(buffer[0]>PREFERENCES_NAME_LENGTH) buffer[0]= PREFERENCES_NAME_LENGTH;
	memcpy(preferences->name, buffer, buffer[0]+1);
#endif

	return TRUE;
}

/* --------- sound */
static void *get_sound_pref_data(
	void)
{
	return w_get_data_from_preferences(
		prefSOUND_TAG,sizeof(struct sound_manager_parameters),
		default_sound_manager_parameters,
		NULL);
}

static void setup_sound_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
#ifdef mac
	struct sound_manager_parameters *preferences= (struct sound_manager_parameters *) prefs;
	short active;
	word available_flags;

	available_flags= available_sound_manager_flags(preferences->flags);

	/* First setup the popups */
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iVOLUME, first_item), NONE, 
		preferences->volume+1);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iCHANNELS, first_item), NONE, 
		preferences->channel_count);

	active= (available_flags & _stereo_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iSTEREO, first_item), active, 
		(preferences->flags & _stereo_flag) ? TRUE : FALSE);

	/* Don't do dynamic tracking if you aren't in stereo. */
	if(!(preferences->flags & _stereo_flag))
	{
		preferences->flags &= ~_dynamic_tracking_flag;
	}

	active= (available_flags & _dynamic_tracking_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iACTIVE_PANNING, first_item), active, 
		(preferences->flags & _dynamic_tracking_flag) ? TRUE : FALSE);

	active= (available_flags & _16bit_sound_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iHIGH_QUALITY, first_item), active, 
		(preferences->flags & _16bit_sound_flag) ? TRUE : FALSE);

	active= (available_flags & _ambient_sound_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iAMBIENT_SOUND, first_item), active, 
		(preferences->flags & _ambient_sound_flag) ? TRUE : FALSE);

	active= (available_flags & _more_sounds_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iMORE_SOUNDS, first_item), active, 
		(preferences->flags & _more_sounds_flag) ? TRUE : FALSE);

	return;
#endif
}

static void hit_sound_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
#ifdef mac
	ControlHandle control;
	short item_type;
	Rect bounds;
	struct sound_manager_parameters *preferences= (struct sound_manager_parameters *)prefs;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iSTEREO:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->flags |= _stereo_flag;
			} else {
				preferences->flags &= ~_stereo_flag;
			}
			break;

		case iACTIVE_PANNING:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->flags |= _dynamic_tracking_flag;
			} else {
				preferences->flags &= ~_dynamic_tracking_flag;
			}
			break;
			
		case iHIGH_QUALITY:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->flags |= _16bit_sound_flag;
			} else {
				preferences->flags &= ~_16bit_sound_flag;
			}
			break;

		case iAMBIENT_SOUND:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->flags |= _ambient_sound_flag;
			} else {
				preferences->flags &= ~_ambient_sound_flag;
			}
			break;

		case iMORE_SOUNDS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->flags |= _more_sounds_flag;
			} else {
				preferences->flags &= ~_more_sounds_flag;
			}
			break;
			
		case iVOLUME:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->volume= GetControlValue(control)-1;
			test_sound_volume(preferences->volume, _snd_adjust_volume);
			break;

		case iCHANNELS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->channel_count= GetControlValue(control);	
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	setup_sound_dialog(dialog, first_item, prefs);

	return;
#endif
}
	
static boolean teardown_sound_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void) (dialog, first_item, prefs);
	return TRUE;
}

/* --------- input */

static void *get_input_pref_data(
	void)
{
	return w_get_data_from_preferences(
		prefINPUT_TAG,sizeof(struct input_preferences_data),
		default_input_preferences,
		validate_input_preferences);
}

static void setup_input_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
#ifdef mac
	struct input_preferences_data *preferences= (struct input_preferences_data *)prefs;
	short which;
	
	// LP change: implemented Ben Thompson ISp support
	which = (preferences->input_device == _mouse_yaw_pitch) ? iMOUSE_CONTROL :
		(preferences->input_device == _keyboard_or_game_pad) ?iKEYBOARD_CONTROL: iINPUT_SPROCKET_CONTROL;
	modify_radio_button_family(dialog, LOCAL_TO_GLOBAL_DITL(iMOUSE_CONTROL, first_item), 
		LOCAL_TO_GLOBAL_DITL(iINPUT_SPROCKET_CONTROL, first_item), 
		LOCAL_TO_GLOBAL_DITL(which, first_item));
	//	which = (preferences->input_device == _mouse_yaw_pitch) ? iMOUSE_CONTROL : iKEYBOARD_CONTROL;
	//		modify_radio_button_family(dialog, LOCAL_TO_GLOBAL_DITL(iMOUSE_CONTROL, first_item), 
	//		LOCAL_TO_GLOBAL_DITL(iKEYBOARD_CONTROL, first_item), 
	//		LOCAL_TO_GLOBAL_DITL(which, first_item));
	
	// LP addition: handle the input modifiers
	short active;
		
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iINTERCHANGE_RUN_WALK, first_item), active, 
		(preferences->modifiers & _inputmod_interchange_run_walk) ? TRUE : FALSE);
	
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iINTERCHANGE_SWIM_SINK, first_item), active, 
		(preferences->modifiers & _inputmod_interchange_swim_sink) ? TRUE : FALSE);
	
	// LP addition: Josh Elsasser's dont-switch-weapons patch
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDONT_SWITCH_TO_NEW_WEAPON, first_item), active, 
		(preferences->modifiers & _inputmod_dont_switch_to_new_weapon) ? TRUE : FALSE);
#endif
}

static void hit_input_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
#ifdef mac
	ControlHandle control;
	short item_type;
	Rect bounds;
	struct input_preferences_data *preferences= (struct input_preferences_data *)prefs;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		// LP change: added ISp-control button action
		case iMOUSE_CONTROL:
		case iKEYBOARD_CONTROL:
		case iINPUT_SPROCKET_CONTROL:
					modify_radio_button_family(dialog, LOCAL_TO_GLOBAL_DITL(iMOUSE_CONTROL, first_item), 
				LOCAL_TO_GLOBAL_DITL(iINPUT_SPROCKET_CONTROL, first_item), item_hit);
			//	modify_radio_button_family(dialog, LOCAL_TO_GLOBAL_DITL(iMOUSE_CONTROL, first_item), 
			//		LOCAL_TO_GLOBAL_DITL(iKEYBOARD_CONTROL, first_item), item_hit);
			// LP change: added Ben Thompson's ISp support
			preferences->input_device= GLOBAL_TO_LOCAL_DITL(item_hit, first_item)==iMOUSE_CONTROL ?
				_mouse_yaw_pitch : (GLOBAL_TO_LOCAL_DITL(item_hit, first_item)==iINPUT_SPROCKET_CONTROL ? 
				_input_sprocket_only : _keyboard_or_game_pad);
		break;
		
		// Added run/walk and swim/sink interchange; renamed iSET_KEYS (LP: renamed it back)
		case iINTERCHANGE_RUN_WALK:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->modifiers |= _inputmod_interchange_run_walk;
			} else {
				preferences->modifiers &= ~_inputmod_interchange_run_walk;
			}
			break;
			
		case iINTERCHANGE_SWIM_SINK:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->modifiers |= _inputmod_interchange_swim_sink;
			} else {
				preferences->modifiers &= ~_inputmod_interchange_swim_sink;
			}
			break;
			
		// LP addition: Josh Elsasser's dont-switch-weapons patch
		case iDONT_SWITCH_TO_NEW_WEAPON:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			if(!GetControlValue(control))
			{
				preferences->modifiers |= _inputmod_dont_switch_to_new_weapon;
			} else {
				preferences->modifiers &= ~_inputmod_dont_switch_to_new_weapon;
			}
			break;
			
		case iSET_KEYS:
			{
				short key_codes[NUMBER_OF_KEYS];
				
				objlist_copy(key_codes, preferences->keycodes, NUMBER_OF_KEYS);
				if(configure_key_setup(key_codes))
				{
					objlist_copy(preferences->keycodes, key_codes, NUMBER_OF_KEYS);
					set_keys(key_codes);
				}
			}
			break;

		// LP change: modification of Ben Thompson's change
		case iSET_INPUT_SPROCKET:
			ConfigureMarathonISpControls();
			break;
			
		default:
			break;
	}
	
	setup_input_dialog(dialog, first_item, prefs);
#endif
}
	
static boolean teardown_input_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void)(dialog, first_item, prefs);
	return TRUE;
}

/* ------------------ environment preferences */
// LP change: increased
#define MAXIMUM_FIND_FILES (128)
// #define MAXIMUM_FIND_FILES (32)

struct file_description {
	// OSType file_type;
	// This is now the _typecode_stuff specified in tags.h (abstract file typing)
	int file_type;
	unsigned long checksum;
	unsigned long parent_checksum;
};

static FileSpecifier *accessory_files= NULL;
// static FSSpec *accessory_files= NULL;
static struct file_description *file_descriptions= NULL;
static short accessory_file_count= 0;
static boolean physics_valid= TRUE;

static void default_environment_preferences(
	void *preferences)
{
	struct environment_preferences_data *prefs= (struct environment_preferences_data *)preferences;

	obj_set(*prefs, NONE);

	FileSpecifier DefaultFile;
	
	get_default_map_spec(DefaultFile);
	prefs->map_checksum= read_wad_file_checksum(DefaultFile);
#ifdef mac
	obj_copy(prefs->map_file, DefaultFile.GetSpec());
#else
	DefaultFile.GetName(prefs->map_file);
#endif
	
	get_default_physics_spec(DefaultFile);
	prefs->physics_checksum= read_wad_file_checksum(DefaultFile);
#ifdef mac
	obj_copy(prefs->physics_file, DefaultFile.GetSpec());
#else
	DefaultFile.GetName(prefs->physics_file);
#endif
	
	get_default_shapes_spec(DefaultFile);
	
	prefs->shapes_mod_date = DefaultFile.GetDate();
	// prefs->shapes_mod_date= get_file_modification_date(&DefaultFile.Spec);
#ifdef mac
	obj_copy(prefs->shapes_file, DefaultFile.GetSpec());
#else
	DefaultFile.GetName(prefs->shapes_file);
#endif

	get_default_sounds_spec(DefaultFile);
	
	prefs->sounds_mod_date = DefaultFile.GetDate();
	// prefs->sounds_mod_date= get_file_modification_date(&DefaultFile.Spec);
#ifdef mac
	obj_copy(prefs->sounds_file, DefaultFile.GetSpec());
#else
	DefaultFile.GetName(prefs->sounds_file);
#endif
}

static boolean validate_environment_preferences(
	void *prefs)
{
	(void) (prefs);
	return FALSE;
}

static void *get_environment_pref_data(
	void)
{
	return w_get_data_from_preferences(
		prefENVIRONMENT_TAG,sizeof(struct environment_preferences_data), 
		default_environment_preferences,
		validate_environment_preferences);
}

static void setup_environment_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
#ifdef mac
	struct environment_preferences_data *preferences= (struct environment_preferences_data *)prefs;

	if(allocate_extensions_memory())
	{
		SetCursor(*GetCursor(watchCursor));
		
		build_extensions_list();

		/* Fill in the extensions.. */
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iMAP, first_item),
			_typecode_scenario, preferences->map_checksum);
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iPHYSICS, first_item),
			_typecode_physics, preferences->physics_checksum);
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iSHAPES, first_item),
			_typecode_shapes, preferences->shapes_mod_date);
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iSOUNDS, first_item),
			_typecode_sounds, preferences->sounds_mod_date);

		SetCursor(&qd.arrow);
	} else {
		// LP change:
		assert(false);
		// halt();
	}
	
	return;
#endif
}

static void hit_environment_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
#ifdef mac
	struct environment_preferences_data *preferences= (struct environment_preferences_data *)prefs;

	(void)(dialog);
	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iMAP:
			preferences->map_checksum= find_checksum_and_file_spec_from_dialog(dialog, item_hit, 
				_typecode_scenario,	&preferences->map_file);
			break;
			
		case iPHYSICS:
			preferences->physics_checksum= find_checksum_and_file_spec_from_dialog(dialog, item_hit, 
				_typecode_physics,	&preferences->physics_file);
			break;
			
		case iSHAPES:
			preferences->shapes_mod_date= find_checksum_and_file_spec_from_dialog(dialog, item_hit,
				_typecode_shapes, &preferences->shapes_file);
			break;

		case iSOUNDS:
			preferences->sounds_mod_date= find_checksum_and_file_spec_from_dialog(dialog, item_hit,
				_typecode_sounds, &preferences->sounds_file);
			break;
			
		case iPATCHES_LIST:
			break;
			
		default:
			break;
	}

	return;
#endif
}

/* Load the environment.. */
void load_environment_from_preferences(
	void)
{
	FileSpecifier File;
	OSErr error;
	struct environment_preferences_data *prefs= environment_preferences;

#ifdef mac

	File.SetSpec(prefs->map_file);
	/*
	error= FSMakeFSSpec(prefs->map_file.vRefNum, prefs->map_file.parID, prefs->map_file.name,
		(FSSpec *) &file);
	if(!error)
	*/
	if (File.Exists())
	{
		set_map_file(File);
		// set_map_file(&file);
	} else {
		/* Try to find the checksum */
		// if(find_wad_file_that_has_checksum(&file,
		if(find_wad_file_that_has_checksum(File,
			_typecode_scenario, strPATHS, prefs->map_checksum))
		{
			set_map_file(File);
			// set_map_file(&file);
		} else {
			set_to_default_map();
		}
	}

	File.SetSpec(prefs->physics_file);
	/*
	error= FSMakeFSSpec(prefs->physics_file.vRefNum, prefs->physics_file.parID, prefs->physics_file.name,
		(FSSpec *) &file);
	if(!error)
	*/
	if (File.Exists())
	{
		set_physics_file(File);
		// set_physics_file(&file);
		import_definition_structures();
	} else {
		// if(find_wad_file_that_has_checksum(&file,
		if(find_wad_file_that_has_checksum(File,
			_typecode_physics, strPATHS, prefs->physics_checksum))
		{
			set_physics_file(File);
			// set_physics_file(&file);
			import_definition_structures();
		} else {
			/* Didn't find it.  Don't change them.. */
		}
	}
	
	File.SetSpec(prefs->shapes_file);
	/*
	error= FSMakeFSSpec(prefs->shapes_file.vRefNum, prefs->shapes_file.parID, prefs->shapes_file.name,
		(FSSpec *) &file);
	if(!error)
	*/
	if (File.Exists())
	{
		open_shapes_file(File);
		// open_shapes_file((FSSpec *) &file);
	} else {
		// if(find_file_with_modification_date(&file,
		if(find_file_with_modification_date(File,
			_typecode_shapes, strPATHS, prefs->shapes_mod_date))
		{
			open_shapes_file(File);
			// open_shapes_file((FSSpec *) &file);
		} else {
			/* What should I do? */
		}
	}

	File.SetSpec(prefs->sounds_file);
	/*
	error= FSMakeFSSpec(prefs->sounds_file.vRefNum, prefs->sounds_file.parID, prefs->sounds_file.name,
		(FSSpec *) &file);
	if(!error)
	*/
	if (File.Exists())
	{
		open_sound_file(File);
		// open_sound_file((FSSpec *) &file);
	} else {
		// if(find_file_with_modification_date(&file,
		if(find_file_with_modification_date(File,
			_typecode_sounds, strPATHS, prefs->sounds_mod_date))
		{
			open_sound_file(File);
			// open_sound_file((FSSpec *) &file);
		} else {
			/* What should I do? */
		}
	}
	
	return;
#else

	File.SetName(prefs->map_file, FileSpecifier::C_Map);
	if (File.Exists())
		set_map_file(File);
	else {
		/* Try to find the checksum */
		if (find_wad_file_that_has_checksum(File, SCENARIO_FILE_TYPE, strPATHS, prefs->map_checksum))
			set_map_file(File);
		else
			set_to_default_map();
	}

	File.SetName(prefs->physics_file, FileSpecifier::C_Phys);
	if (File.Exists()) {
		set_physics_file(File);
		import_definition_structures();
	} else {
		if (find_wad_file_that_has_checksum(File, PHYSICS_FILE_TYPE, strPATHS, prefs->physics_checksum)) {
			set_physics_file(File);
			import_definition_structures();
		} else {
			/* Didn't find it.  Don't change them.. */
		}
	}
	
	File.SetName(prefs->shapes_file, FileSpecifier::C_Shape);
	if (File.Exists())
		open_shapes_file(File);
	else {
#if 0	//!!
		if (find_file_with_modification_date(File, SHAPES_FILE_TYPE, strPATHS, prefs->shapes_mod_date))
			open_shapes_file(File);
		else {
			/* What should I do? */
		}
#endif
	}

	File.SetName(prefs->sounds_file, FileSpecifier::C_Sound);
	if (File.Exists())
		open_sound_file(File);
	else {
#if 0	//!!
		if (find_file_with_modification_date(File, SOUNDS_FILE_TYPE, strPATHS, prefs->sounds_mod_date))
			open_sound_file(File);
		else {
			/* What should I do? */
		}
#endif
	}
#endif
}

static boolean teardown_environment_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
#ifdef mac
	struct environment_preferences_data *preferences= (struct environment_preferences_data *)prefs;

	(void) (dialog, first_item);
	/* Proceses the entire physics file.. */
	free_extensions_memory();
#endif
	
	return TRUE;
}

/*
static unsigned long get_file_modification_date(
	FSSpec *file)
{
#ifdef mac
	CInfoPBRec pb;
	OSErr error;
	unsigned long modification_date= 0l;
	
	memset(&pb, 0, sizeof(pb));
	pb.hFileInfo.ioVRefNum= file->vRefNum;
	pb.hFileInfo.ioNamePtr= file->name;
	pb.hFileInfo.ioDirID= file->parID;
	pb.hFileInfo.ioFDirIndex= 0;
	error= PBGetCatInfoSync(&pb);
	if(!error)
	{
		modification_date= pb.hFileInfo.ioFlMdDat;
	}

	return modification_date;
#else
	return 0;
#endif
}
*/

/* ---------------- miscellaneous */
#ifdef mac
static void set_popup_enabled_state(
	DialogPtr dialog,
	short item_number,
	short item_to_affect,
	boolean enabled)
{
	MenuHandle menu;
	struct PopupPrivateData **privateHndl;
	ControlHandle control;
	short item_type;
	Rect bounds;
	
	/* Get the menu handle */
	GetDialogItem(dialog, item_number, &item_type, (Handle *) &control, &bounds);
	assert(item_type&ctrlItem);

	/* I don't know how to assert that it is a popup control... <sigh> */
	privateHndl= (PopupPrivateData **) ((*control)->contrlData);
	assert(privateHndl);
	
	menu= (*privateHndl)->mHandle;
	assert(menu);
	
	if(enabled)
	{
		EnableItem(menu, item_to_affect);
	} else {
		DisableItem(menu, item_to_affect);
	}
}

static boolean allocate_extensions_memory(
	void)
{
	boolean success;

	assert(!accessory_files);
	assert(!file_descriptions);

	accessory_file_count= 0;
	// A lot prettier than the original mallocs...
	accessory_files= new FileSpecifier[MAXIMUM_FIND_FILES];
	file_descriptions= new file_description[MAXIMUM_FIND_FILES];
	if(file_descriptions && accessory_files)
	{
		success= TRUE;
	} else {
		if(file_descriptions) delete []file_descriptions;
		if(accessory_files) delete []accessory_files;
		accessory_files= NULL;
		file_descriptions= NULL;
		success= FALSE;
	}
	
	return success;
}

static void free_extensions_memory(
	void)
{
	assert(accessory_files);
	assert(file_descriptions);

	delete []file_descriptions;
	delete []accessory_files;
	accessory_files= NULL;
	file_descriptions= NULL;
	accessory_file_count= 0;

	return;
}

static Boolean file_is_extension_and_add_callback(
	FileSpecifier& File,
	// FSSpec *file,
	void *data)
{
	unsigned long checksum;
	CInfoPBRec *pb= (CInfoPBRec *) data;
	
	assert(accessory_files);
	assert(file_descriptions);

	if(accessory_file_count<MAXIMUM_FIND_FILES)
	{
		// LP change, since the filetypes are no longer constants
		int Filetype = File.GetType();
		// OSType Filetype = pb->hFileInfo.ioFlFndrInfo.fdType;
		if (Filetype == _typecode_scenario || Filetype == _typecode_physics)
		{
			checksum= read_wad_file_checksum(File);
			// checksum= read_wad_file_checksum((FileDesc *) file);
			if(checksum != NONE) /* error. */
			{
				accessory_files[accessory_file_count]= File;
				// accessory_files[accessory_file_count]= *file;
				file_descriptions[accessory_file_count].file_type= Filetype;
				// file_descriptions[accessory_file_count].file_type= pb->hFileInfo.ioFlFndrInfo.fdType;
				file_descriptions[accessory_file_count++].checksum= checksum;
			}
		}
		else if (Filetype == _typecode_patch)
		{
			checksum= read_wad_file_checksum(File);
			// checksum= read_wad_file_checksum((FileDesc *) file);
			if(checksum != NONE) /* error. */
			{
				unsigned long parent_checksum;
				
				parent_checksum= read_wad_file_parent_checksum(File);
				// parent_checksum= read_wad_file_parent_checksum((FileDesc *) file);
				accessory_files[accessory_file_count]= File;
				// accessory_files[accessory_file_count]= *file;
				file_descriptions[accessory_file_count].file_type= Filetype;
				// file_descriptions[accessory_file_count].file_type= pb->hFileInfo.ioFlFndrInfo.fdType;
				file_descriptions[accessory_file_count++].checksum= checksum;
				file_descriptions[accessory_file_count++].parent_checksum= parent_checksum;
			}
		}
		else if (Filetype == _typecode_shapes || Filetype == _typecode_sounds)
		{		
			accessory_files[accessory_file_count]= File;
			// accessory_files[accessory_file_count]= *file;
			file_descriptions[accessory_file_count].file_type= Filetype;
			// file_descriptions[accessory_file_count].file_type= pb->hFileInfo.ioFlFndrInfo.fdType;
			file_descriptions[accessory_file_count++].checksum= pb->hFileInfo.ioFlMdDat;
		}
	}
	
	return FALSE;
}

static void build_extensions_list(
	void)
{
	DirectorySpecifier BaseDir;
	BaseDir.SetToAppParent();
	search_from_directory(BaseDir);
	
	/*
	FSSpec my_spec;
	short path_count, ii;

	get_my_fsspec(&my_spec);
	search_from_directory(&my_spec);
	*/
	
	// LP: for now, will only care about looking in the Marathon app's directory
	#if 0
	/* Add the paths.. */
	path_count= countstr(strPATHS);
	for(ii= 0; ii<path_count; ++ii)
	{
		OSErr err;
		FSSpec file;
	
		getpstr(ptemporary, strPATHS, ii);
		
		/* Hmm... check FSMakeFSSpec... */
		/* Relative pathname.. */
		err= FSMakeFSSpec(my_spec.vRefNum, my_spec.parID, ptemporary, &file);
		
		if(!err) 
		{
			long parID;
			
			err= get_directories_parID(&file, &parID);
			if(!err)
			{
				file.parID= parID;
				search_from_directory(&file);
			} else {
				dprintf("Error: %d", err);
			}
		}
	}
	#endif

	return;
}

static void search_from_directory(DirectorySpecifier& BaseDir)
//	FSSpec *file)
{
	FileFinder pb;
	// struct find_file_pb pb;
	OSErr error;

	pb.Clear();	
	// memset(&pb, 0, sizeof(struct find_file_pb));
	pb.version= 0;
	// LP change: always recurse
	pb.flags= _ff_recurse | _ff_callback_with_catinfo;
#if 0
#ifdef FINAL
	pb.flags= _ff_recurse | _ff_callback_with_catinfo;
#else
	pb.flags= _ff_callback_with_catinfo;
#endif
#endif
	pb.search_type= _callback_only;
	pb.BaseDir = BaseDir;
	// pb.vRefNum= file->vRefNum;
	// pb.directory_id= file->parID;
	pb.Type= WILDCARD_TYPE;
	// pb.type_to_find= WILDCARD_TYPE;
	pb.buffer= NULL;
	pb.max= MAXIMUM_FIND_FILES;
	pb.callback= file_is_extension_and_add_callback;
	pb.user_data= NULL;
	pb.count= 0;

	vassert(pb.Find(), csprintf(temporary, "Error: %d", pb.GetError()));
	// error= find_files(&pb);
	// vassert(!error, csprintf(temporary, "Error: %d", error));

	return;
}

/* Note that we are going to assume that things don't change while they are in this */
/*  dialog- ie no one is copying files to their machine, etc. */
// Now intended to use the _typecode_stuff in tags.h (abstract filetypes)
static void fill_in_popup_with_filetype(
	DialogPtr dialog, 
	short item,
	int type,
	unsigned long checksum)
{
	MenuHandle menu;
	short index, value= NONE;
	ControlHandle control;
	short item_type, count;
	Rect bounds;

	/* Get the menu */
	menu= get_popup_menu_handle(dialog, item);
	
	/* Remove whatever it had */
	while(CountMItems(menu)) DeleteMenuItem(menu, 1);

	assert(file_descriptions);
	for(index= 0; index<accessory_file_count; ++index)
	{
		if(file_descriptions[index].file_type==type)
		{
			AppendMenu(menu, "\p ");
			SetMenuItemText(menu, CountMItems(menu), accessory_files[index].GetSpec().name);

			if(file_descriptions[index].checksum==checksum)
			{
				value= CountMItems(menu);
			}
		}
	} 

	/* Set the max value */
	GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);
	count= CountMItems(menu);

	if(count==0)
	{
		// LP change, since the filetypes are no longer constants
		if (type == _typecode_physics)
		{
			set_to_default_physics_file();
			AppendMenu(menu, getpstr(ptemporary, strPROMPTS, _default_prompt));
			value= 1;
			physics_valid= FALSE;
			count++;
		}
	} 
	
	SetControlMaximum(control, count);
	
	if(value != NONE)
	{
		SetControlValue(control, value);
	} else {
		/* Select the default one, somehow.. */
		SetControlValue(control, 1);
	}

	return;
}

static unsigned long find_checksum_and_file_spec_from_dialog(
	DialogPtr dialog, 
	short item_hit, 
	OSType type,
	FSSpec *file)
{
	short index;
	ControlHandle control;
	short item_type, value;
	Rect bounds;
	unsigned long checksum;
	
	/* Get the dialog item hit */
	GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
	value= GetControlValue(control);
	
	for(index= 0; index<accessory_file_count; ++index)
	{
		if(file_descriptions[index].file_type==type)
		{
			if(!--value)
			{
				/* This is it */
				*file= accessory_files[index].GetSpec();
				checksum= file_descriptions[index].checksum;
			}
		}
	}

	return checksum;
}

static MenuHandle get_popup_menu_handle(
	DialogPtr dialog,
	short item)
{
	struct PopupPrivateData **privateHndl;
	MenuHandle menu;
	short item_type;
	ControlHandle control;
	Rect bounds;

	/* Add the maps.. */
	GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);

	/* I don't know how to assert that it is a popup control... <sigh> */
	privateHndl= (PopupPrivateData **) ((*control)->contrlData);
	assert(privateHndl);

	menu= (*privateHndl)->mHandle;
	assert(menu);

	return menu;
}
#endif

#if 0
static boolean control_strip_installed(
	void)
{
	boolean installed= FALSE;
	long control_strip_version;
	OSErr error;
	
	error= Gestalt(gestaltControlStripVersion, &control_strip_version);
	if(!error)
	{
		installed= TRUE;
	}
	
	return installed;
}

static void hide_control_strip(
	void)
{
	assert(control_strip_installed());
	if(SBIsControlStripVisible())
	{
		SBShowHideControlStrip(FALSE);
	}
}

static void show_control_strip(
	void)
{
	assert(control_strip_installed());
	if(!SBIsControlStripVisible())
	{
		SBShowHideControlStrip(TRUE);
	}
}
#endif


// LP addition: get these from the preferences data
ChaseCamData& GetChaseCamData() {return player_preferences->ChaseCam;}
CrosshairData& GetCrosshairData() {return player_preferences->Crosshairs;}
OGL_ConfigureData& Get_OGL_ConfigureData() {return graphics_preferences->OGL_Configure;}

// LP addition: modification of Josh Elsasser's dont-switch-weapons patch
// so as to access preferences stuff here
bool dont_switch_to_new_weapon() {
	return TEST_FLAG(input_preferences->modifiers,_inputmod_dont_switch_to_new_weapon);
}