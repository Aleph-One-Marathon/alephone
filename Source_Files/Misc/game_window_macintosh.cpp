/*

	macintosh_game_window.c
	Tuesday, August 29, 1995 6:09:18 PM- rdm created.

	May 28, 2000 (Loren Petrich): Added support for buffering the Heads-Up Display
*/

#include "macintosh_cseries.h"
#include "my32bqd.h"

#include "map.h"
#include "shell.h"
#include "preferences.h"
#include "screen_drawing.h"
#include "interface.h"
#include "screen.h"
#include "portable_files.h"
#include "mysound.h" // for screen_definitions.h
#include "screen_definitions.h"
#include "images.h"

extern GrafPtr screen_window;
extern GWorldPtr world_pixels;

extern void update_everything(short time_elapsed);

extern void draw_panels(void);

/* ------------- code begins! */
void draw_panels(
	void)
{
	// LP addition: sets the HUD buffer to whatever is its appropriate state;
	// reasonable to do this here, since this routine gets the background picture
	// for the HUD.
	ResetHUDBuffer();

	struct screen_mode_data new_mode= graphics_preferences->screen_mode;
	PicHandle picture;
	Rect destination= {320, 0, 480, 640};
	Rect source= {0, 0, 160, 640};

	new_mode.acceleration= _no_acceleration;
	new_mode.size= _100_percent;
	new_mode.high_resolution= TRUE;
	change_screen_mode(&new_mode, FALSE);

	myLockPixels(world_pixels);

	picture= get_picture_resource_from_images(INTERFACE_PANEL_BASE);
	if(picture)
	{
		// LP addition: use HUD buffer if possible
		if (!_set_port_to_HUD())
			_set_port_to_gworld();
		SetOrigin(0, 320);

		HLock((Handle) picture);
		DrawPicture(picture, &destination);
		HUnlock((Handle) picture);

		update_everything(NONE);
		ReleaseResource((Handle) picture);
	
		SetOrigin(0, 0);
		_restore_port();
	} else {
		/* Either they don't have the picture, or they are out of memory.  Most likely no memory */
		alert_user(fatalError, strERRORS, outOfMemory, ResError());
	}

	/* Note that you don't get here if the picture failed.. */
	// LP addition: redirecting the HUD drawing to its own buffer
	// if that buffer is available
	if (!DrawBufferedHUD(source,destination))
	{
		GrafPtr old_port;
		RGBColor old_forecolor, old_backcolor;
		
		GetPort(&old_port);
		SetPort(screen_window);

		GetForeColor(&old_forecolor);
		GetBackColor(&old_backcolor);
		RGBForeColor(&rgb_black);
		RGBBackColor(&rgb_white);
		
		/* Slam it to the screen. */
		CopyBits((BitMapPtr)*world_pixels->portPixMap, &screen_window->portBits, //(BitMapPtr)*screen_pixmap,
			&source, &destination, srcCopy, (RgnHandle) NULL);
		RGBForeColor(&old_forecolor);
		RGBBackColor(&old_backcolor);
		SetPort(old_port);
	}
	myUnlockPixels(world_pixels);

	change_screen_mode(&graphics_preferences->screen_mode, FALSE);
}