/*

	preferences_macintosh.c
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

Oct 14, 2000 (Loren Petrich):
	Added music-volume stuff

Sept 9, 2001 (Loren Petrich):
	Modified the checkbox code to be much more reasonable.
	Also added Ian Rickard's smart-dialog processing.
	And added a routine for setting preferences to single files
	if only single files of some type are present (idiot-proofing)
*/

#ifdef env68k
	#pragma segment dialogs
#endif

enum {
	ditlGRAPHICS= 4001,
	iCHOOSE_MONITOR=1,
	// LP change: finally eliminated this option
	// iDRAW_EVERY_OTHER_LINE,
	iHARDWARE_ACCELERATION,
	iNUMBER_OF_COLORS,
	iWINDOW_SIZE,
	iDETAIL,
	iBRIGHTNESS,
	iOPENGL_OPTIONS, // LP addition: OpenGL-options button
	iFILL_SCREEN // LP addition: fill-the-screen button
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

// LP: added music and Ian Rickard's relative-to-system flag
enum {
	ditlSOUND= 4003,
	iSTEREO= 1,
	iACTIVE_PANNING,
	iAMBIENT_SOUND,
	iHIGH_QUALITY,
	iMORE_SOUNDS,
	iRELATIVE_VOLUME,
	iVOLUME,
	iCHANNELS,
	iMUSIC
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
	iSET_INPUT_SPROCKET,
	iINTERCHANGE_RUN_WALK,
	iINTERCHANGE_SWIM_SINK,
	iDONT_SWITCH_TO_NEW_WEAPON,
	iUSE_INTERFACE_BUTTON_SOUNDS
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


/* ----------- private prototypes */
static void setup_graphics_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_graphics_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_graphics_dialog(DialogPtr dialog, short first_item, void *prefs);
static void setup_player_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_player_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_player_dialog(DialogPtr dialog, short first_item, void *prefs);
static void setup_sound_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_sound_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_sound_dialog(DialogPtr dialog, short first_item, void *prefs);
static void setup_input_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_input_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_input_dialog(DialogPtr dialog, short first_item, void *prefs);
static void set_popup_enabled_state(DialogPtr dialog, short item_number, short item_to_affect,
	bool enabled);
static void setup_environment_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_environment_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_environment_dialog(DialogPtr dialog, short first_item, void *prefs);
static void fill_in_popup_with_filetype(DialogPtr dialog, short item, int type, unsigned long checksum);
static MenuHandle get_popup_menu_handle(DialogPtr dialog, short item);
static bool allocate_extensions_memory(void);
static void free_extensions_memory(void);
static void build_extensions_list(void);
static void search_from_directory(DirectorySpecifier& BaseDir);
static unsigned long find_checksum_and_file_spec_from_dialog(DialogPtr dialog, 
	short item_hit, uint32 type, FSSpec *file);
static void SetToLoneFile(uint32 Type, FSSpec& File, unsigned long& Checksum);
static void	rebuild_patchlist(DialogPtr dialog, short item, unsigned long parent_checksum,
	struct environment_preferences_data *preferences);

/* ---------------- code */

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
	
	// Preserve the old sound preferences, so that music volume can be previewed,
	// while being able to revert to the original values
	sound_manager_parameters OriginalSoundParameters;
	obj_copy(OriginalSoundParameters,*sound_preferences);

	if(set_preferences(prefs_data, NUMBER_OF_PREFS_PANELS, initialize_preferences))
	{
		/* Save the new ones. */
		write_preferences();
		set_sound_manager_parameters(sound_preferences);
		load_environment_from_preferences();
	}
	else
		set_sound_manager_parameters(&OriginalSoundParameters);
	
	return;
}


/*
 *  Get user name
 */

#define strUSER_NAME -16096

static void get_name_from_system(
	unsigned char *name)
{
	StringHandle name_handle;
	char old_state;

	name_handle= GetString(strUSER_NAME);
	assert(name_handle);
	
	old_state= HGetState((Handle)name_handle);
	HLock((Handle)name_handle);
	
	pstrcpy(name, *name_handle);
	HSetState((Handle)name_handle, old_state);
}


/*
 *  Ethernet available?
 */

static bool ethernet_active(
	void)
{
	short  refnum;
	OSErr  error;

	error= OpenDriver("\p.ENET", &refnum);
	
	return error==noErr ? true : false;
}


/* ------------- dialog functions */

/* --------- graphics */
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
	struct graphics_preferences_data *preferences= (struct graphics_preferences_data *) prefs;
	short value, active;
	
	// IR-inspired: force to some reasonable value:
	if (preferences->screen_mode.bit_depth >= 32)
		preferences->screen_mode.bit_depth = 32;
	else if (preferences->screen_mode.bit_depth >= 16)
		preferences->screen_mode.bit_depth = 16;
	else
		preferences->screen_mode.bit_depth = 8;
	
	if(machine_supports_32bit(&preferences->device_spec))
	{
		active= true;
	} else {
		if(preferences->screen_mode.bit_depth==32) preferences->screen_mode.bit_depth= 16;
		active= false;
	}
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _millions_colors_menu_item, active);

	if(machine_supports_16bit(&preferences->device_spec))
	{
		active= true;
	} else {
		if(preferences->screen_mode.bit_depth==16) preferences->screen_mode.bit_depth= 8;
		active= false;
	}
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _thousands_colors_menu_item, active);
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _billions_colors_menu_item, false);

	/* Make sure it is enabled. */
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), 
		CONTROL_ACTIVE, NONE);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDETAIL, first_item), 
		CONTROL_ACTIVE, NONE);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iCHOOSE_MONITOR, first_item), 
		CONTROL_ACTIVE, NONE);
	
	switch(preferences->screen_mode.bit_depth)
	{
		case 32: value= _millions_colors_menu_item; break;
		case 16: value= _thousands_colors_menu_item; break;
		case 8:	 value= _hundreds_colors_menu_item; break;
		default: value= _hundreds_colors_menu_item;
	}
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), NONE, value);

	// No more special 68K support either
	preferences->screen_mode.draw_every_other_line= false;
	active= CONTROL_INACTIVE;
	
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
	
	// LP: added full-screen option; will not activate it until it's in good working order
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iFILL_SCREEN, first_item), 
		CONTROL_ACTIVE, (preferences->screen_mode.fullscreen));
		
	return;
}

static void hit_graphics_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
	struct graphics_preferences_data *preferences= (struct graphics_preferences_data *) prefs;
	ControlHandle control;
	short item_type;
	Rect bounds;
	bool resetup= true;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iCHOOSE_MONITOR:
			display_device_dialog(&preferences->device_spec);
			/* We resetup because the new device might not support millions, etc.. */
			break;
		
		case iHARDWARE_ACCELERATION:
			// LP change: added OpenGL support here
			if(preferences->screen_mode.acceleration == _opengl_acceleration)
			{
				preferences->screen_mode.acceleration= _no_acceleration;
			} else {
				preferences->screen_mode.acceleration= _opengl_acceleration;
				// Ian-Rickard-inspired change
				// OpenGL mode is only reasonable with more than 8-bit
				preferences->screen_mode.bit_depth = MAX(preferences->screen_mode.bit_depth, 16);
			}
			break;
			
		case iNUMBER_OF_COLORS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			switch(GetControlValue(control))
			{
				case _hundreds_colors_menu_item:
					// IR-inspired change; no OpenGL mode in 8-bit color depth
					preferences->screen_mode.bit_depth= 8;
					preferences->screen_mode.acceleration= _no_acceleration;
					break;
				case _thousands_colors_menu_item: preferences->screen_mode.bit_depth= 16; break;
				case _millions_colors_menu_item: preferences->screen_mode.bit_depth= 32; break;
				case _billions_colors_menu_item:
				default: preferences->screen_mode.bit_depth= 8;
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
		
		case iFILL_SCREEN:
			// LP change: added full-screen support here
			preferences->screen_mode.fullscreen = !preferences->screen_mode.fullscreen;
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
}
	
static bool teardown_graphics_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void) (dialog, first_item, prefs);
	return true;
}

/* --------- player */
static void setup_player_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
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
	SelectDialogItemText(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), 0, SHRT_MAX);

	/* Setup the color */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iCOLOR, first_item), &item_type, &item, &bounds);
	SetControlValue((ControlHandle) item, preferences->color+1);

	/* Setup the team */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iTEAM, first_item), &item_type, &item, &bounds);
	SetControlValue((ControlHandle) item, preferences->team+1);
}

static void hit_player_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
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
}
	
static bool teardown_player_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
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

	return true;
}

/* --------- sound */
static void setup_sound_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	struct sound_manager_parameters *preferences= (struct sound_manager_parameters *) prefs;
	short active;
	uint16 available_flags;

	available_flags= available_sound_manager_flags(preferences->flags);

	/* First setup the popups */
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iVOLUME, first_item), NONE, 
		preferences->volume+1);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iCHANNELS, first_item), NONE, 
		preferences->channel_count);
	// LP addition:
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iMUSIC, first_item), NONE, 
		preferences->music+1);

	active= (available_flags & _stereo_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iSTEREO, first_item), active, 
		(preferences->flags & _stereo_flag) ? true : false);

	/* Don't do dynamic tracking if you aren't in stereo. */
	if(!(preferences->flags & _stereo_flag))
	{
		preferences->flags &= ~_dynamic_tracking_flag;
	}

	active= (available_flags & _dynamic_tracking_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iACTIVE_PANNING, first_item), active, 
		(preferences->flags & _dynamic_tracking_flag) ? true : false);

	active= (available_flags & _16bit_sound_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iHIGH_QUALITY, first_item), active, 
		(preferences->flags & _16bit_sound_flag) ? true : false);

	active= (available_flags & _ambient_sound_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iAMBIENT_SOUND, first_item), active, 
		(preferences->flags & _ambient_sound_flag) ? true : false);

	active= (available_flags & _more_sounds_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iMORE_SOUNDS, first_item), active, 
		(preferences->flags & _more_sounds_flag) ? true : false);

	active= (available_flags & _relative_volume_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iRELATIVE_VOLUME, first_item), active, 
		(preferences->flags & _relative_volume_flag) ? true : false);

	return;
}

static void hit_sound_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
	ControlHandle control;
	short item_type;
	Rect bounds;
	struct sound_manager_parameters *preferences= (struct sound_manager_parameters *)prefs;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iSTEREO:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_stereo_flag,!GetControlValue(control));
			break;

		case iACTIVE_PANNING:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_dynamic_tracking_flag,!GetControlValue(control));
			break;
			
		case iHIGH_QUALITY:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_16bit_sound_flag,!GetControlValue(control));
			break;

		case iAMBIENT_SOUND:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_ambient_sound_flag,!GetControlValue(control));
			break;

		case iMORE_SOUNDS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_more_sounds_flag,!GetControlValue(control));
			break;

		case iRELATIVE_VOLUME:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_relative_volume_flag,!GetControlValue(control));
			// Need to hear relative/absolute distinction
			test_sound_volume(preferences->volume, Sound_AdjustVolume());
			// For checking out the music volume
			set_sound_manager_parameters(sound_preferences);
			break;
			
		case iVOLUME:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->volume= GetControlValue(control)-1;
			test_sound_volume(preferences->volume, Sound_AdjustVolume());
			// For checking out the music volume
			set_sound_manager_parameters(sound_preferences);
			break;

		case iCHANNELS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->channel_count= GetControlValue(control);	
			break;
			
		// LP addition:
		case iMUSIC:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->music= GetControlValue(control)-1;
			// For checking out the music volume
			set_sound_manager_parameters(sound_preferences);
			music_idle_proc();
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	setup_sound_dialog(dialog, first_item, prefs);

	return;
}
	
static bool teardown_sound_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void) (dialog, first_item, prefs);
	return true;
}

/* --------- input */

static void setup_input_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	struct input_preferences_data *preferences= (struct input_preferences_data *)prefs;
	short which;
	
	// LP change: implemented Ben Thompson ISp support
	which = (preferences->input_device == _mouse_yaw_pitch) ? iMOUSE_CONTROL :
		(preferences->input_device == _keyboard_or_game_pad) ?iKEYBOARD_CONTROL: iINPUT_SPROCKET_CONTROL;
	modify_radio_button_family(dialog, LOCAL_TO_GLOBAL_DITL(iMOUSE_CONTROL, first_item), 
		LOCAL_TO_GLOBAL_DITL(iINPUT_SPROCKET_CONTROL, first_item), 
		LOCAL_TO_GLOBAL_DITL(which, first_item));
	
	// LP addition: handle the input modifiers
	short active;
		
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iINTERCHANGE_RUN_WALK, first_item), active, 
		(preferences->modifiers & _inputmod_interchange_run_walk) ? true : false);
	
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iINTERCHANGE_SWIM_SINK, first_item), active, 
		(preferences->modifiers & _inputmod_interchange_swim_sink) ? true : false);
	
	// LP addition: Josh Elsasser's dont-switch-weapons patch
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDONT_SWITCH_TO_NEW_WEAPON, first_item), active, 
		(preferences->modifiers & _inputmod_dont_switch_to_new_weapon) ? true : false);
	
	// LP addition: whether interface-button sounds are on (Ian Rickard)
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iUSE_INTERFACE_BUTTON_SOUNDS, first_item), active, 
		(preferences->modifiers & _inputmod_use_button_sounds) ? true : false);
}

static void hit_input_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
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
			// LP change: added Ben Thompson's ISp support
			preferences->input_device= GLOBAL_TO_LOCAL_DITL(item_hit, first_item)==iMOUSE_CONTROL ?
				_mouse_yaw_pitch : (GLOBAL_TO_LOCAL_DITL(item_hit, first_item)==iINPUT_SPROCKET_CONTROL ? 
				_input_sprocket_only : _keyboard_or_game_pad);
		break;
		
		// Added run/walk and swim/sink interchange; renamed iSET_KEYS (LP: renamed it back)
		case iINTERCHANGE_RUN_WALK:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_interchange_run_walk,!GetControlValue(control));
			break;
			
		case iINTERCHANGE_SWIM_SINK:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_interchange_swim_sink,!GetControlValue(control));
			break;
			
		// LP addition: Josh Elsasser's dont-switch-weapons patch
		case iDONT_SWITCH_TO_NEW_WEAPON:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_dont_switch_to_new_weapon,!GetControlValue(control));
			break;
			
		// LP addition: whether interface-button sounds are on (Ian Rickard)
		case iUSE_INTERFACE_BUTTON_SOUNDS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_use_button_sounds,!GetControlValue(control));
			if(!GetControlValue(control))
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
}
	
static bool teardown_input_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void)(dialog, first_item, prefs);
	return true;
}

/* ------------------ environment preferences */
// LP change: increased
#define MAXIMUM_FIND_FILES (128)
// #define MAXIMUM_FIND_FILES (32)

struct file_description {
	// This is now the _typecode_stuff specified in tags.h (abstract file typing)
	int file_type;
	unsigned long checksum;
	unsigned long parent_checksum;
};

static FileSpecifier *accessory_files= NULL;
static struct file_description *file_descriptions= NULL;
static short accessory_file_count= 0;
static bool physics_valid= true;

static void setup_environment_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
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
}

static void hit_environment_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
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
}

static bool teardown_environment_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	// Clean up after dialogs; lone files may not be selected properly by them
	struct environment_preferences_data *preferences= (struct environment_preferences_data *)prefs;
	SetToLoneFile(_typecode_scenario, preferences->map_file, preferences->map_checksum);
	SetToLoneFile(_typecode_physics, preferences->physics_file, preferences->physics_checksum);
	SetToLoneFile(_typecode_shapes, preferences->shapes_file, preferences->shapes_mod_date);
	SetToLoneFile(_typecode_sounds, preferences->sounds_file, preferences->sounds_mod_date);

	(void) (dialog, first_item);
	/* Proceses the entire physics file.. */
	free_extensions_memory();
	
	return true;
}

/*
static unsigned long get_file_modification_date(
	FSSpec *file)
{
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
}
*/

/* ---------------- miscellaneous */
static void set_popup_enabled_state(
	DialogPtr dialog,
	short item_number,
	short item_to_affect,
	bool enabled)
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

static bool allocate_extensions_memory(
	void)
{
	bool success;

	assert(!accessory_files);
	assert(!file_descriptions);

	accessory_file_count= 0;
	// A lot prettier than the original mallocs...
	accessory_files= new FileSpecifier[MAXIMUM_FIND_FILES];
	file_descriptions= new file_description[MAXIMUM_FIND_FILES];
	if(file_descriptions && accessory_files)
	{
		success= true;
	} else {
		if(file_descriptions) delete []file_descriptions;
		if(accessory_files) delete []accessory_files;
		accessory_files= NULL;
		file_descriptions= NULL;
		success= false;
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

static bool file_is_extension_and_add_callback(
	FileSpecifier& File,
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
	
	return false;
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
			physics_valid= false;
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
	uint32 type,
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

static void SetToLoneFile(uint32 Type, FSSpec& File, unsigned long& Checksum)
{
	int LoneFileIndex = NONE;	// Null value (-1)
	
	for (int index=0; index<accessory_file_count; index++)
	{
		if (file_descriptions[index].file_type == Type)
		{
			if (LoneFileIndex != NONE)
				// No need to continue if duplicated
				return;
			else
				// Don't quit here; check for another file with this type
				LoneFileIndex = index;
		}
	}
	
	// Do the setting
	if (LoneFileIndex != NONE)
	{
		File = accessory_files[LoneFileIndex].GetSpec();
		Checksum = file_descriptions[LoneFileIndex].checksum;
	}
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

#if 0
static bool control_strip_installed(
	void)
{
	bool installed= false;
	long control_strip_version;
	OSErr error;
	
	error= Gestalt(gestaltControlStripVersion, &control_strip_version);
	if(!error)
	{
		installed= true;
	}
	
	return installed;
}

static void hide_control_strip(
	void)
{
	assert(control_strip_installed());
	if(SBIsControlStripVisible())
	{
		SBShowHideControlStrip(false);
	}
}

static void show_control_strip(
	void)
{
	assert(control_strip_installed());
	if(!SBIsControlStripVisible())
	{
		SBShowHideControlStrip(true);
	}
}
#endif
