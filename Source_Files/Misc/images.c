/*

	images.c
	Thursday, July 20, 1995 3:29:30 PM- rdm created.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb. 5, 2000 (Loren Petrich):
	Better handling of case of no scenario-file resource fork

Aug 21, 2000 (Loren Petrich):
	Added object-oriented file handling
	
	LoadedResource handles are assumed to always be locked,
	and HLock() and HUnlock() have been suppressed for that reason.
*/

#include "macintosh_cseries.h"

#include <stdlib.h>

#include "interface.h"
#include "shell.h"
// #include "portable_files.h"
#include "images.h"
#include "screen.h" // for build_direct_color_table

extern short interface_bit_depth;
extern WindowPtr screen_window;

// Adding resource-file objects
static OpenedResourceFile ImagesResources;
static OpenedResourceFile ScenarioResources;
// static short images_file_handle= NONE;
// static short scenario_file_handle= NONE;

enum {
	_images_file_delta16= 1000,
	_images_file_delta32= 2000,
	_scenario_file_delta16= 10000,
	_scenario_file_delta32= 20000
};

/* -------------- local prototypes */
static void shutdown_images_handler(void);
// LP: looks in an opened resource file
static short determine_pict_resource_id(OpenedResourceFile& OFile,
	OSType pict_resource_type, short base_id,
	short delta16, short delta32);
static void draw_picture(LoadedResource& PictRsrc);
// static void draw_picture(PicHandle picture);

/* ------------- code */
void initialize_images_manager(
	void)
{
	FileSpecifier File;
	File.SetToApp();
	File.SetName(getcstr(temporary, strFILENAMES, filenameIMAGES),_typecode_images);
	if (!File.Exists()) alert_user(fatalError, strERRORS, badExtraFileLocations, fnfErr);
	
	if (!File.Open(ImagesResources))
		alert_user(fatalError, strERRORS, badExtraFileLocations, File.GetError());
	
	/*
	FSSpec file;
	OSErr error;

	error= get_file_spec(&file, strFILENAMES, filenameIMAGES, strPATHS);
	if (error==noErr)
	{
		boolean was_aliased, is_folder; 
	
		ResolveAliasFile(&file, TRUE, &is_folder, &was_aliased);
		images_file_handle= FSpOpenResFile(&file, fsRdPerm);
		error= ResError();
	}
	
	if (error!=noErr || images_file_handle==NONE)
	{
		alert_user(fatalError, strERRORS, badExtraFileLocations, error);
	}
	else
	{
		atexit(shutdown_images_handler);
	}
	*/
	
	atexit(shutdown_images_handler);
}

bool get_picture_resource_from_images(short base_resource, LoadedResource& PictRsrc)
// PicHandle get_picture_resource_from_images(
// 	short base_resource)
{
	// short old_resfile;
	// PicHandle picture;
	
	assert(ImagesResources.IsOpen());
	ImagesResources.Push();
	
	short RsrcID = determine_pict_resource_id(
		ImagesResources,
		'PICT', base_resource,
		_images_file_delta16, _images_file_delta32);
	bool Success = ImagesResources.Get('PICT',RsrcID,PictRsrc);
	
	// assert(images_file_handle != NONE);
	// old_resfile= CurResFile();
	// UseResFile(images_file_handle);
	// picture= (PicHandle) Get1Resource('PICT', 
	//	determine_pict_resource_id('PICT', base_resource,
	//	_images_file_delta16, _images_file_delta32));
//	if (picture) HNoPurge((Handle)picture);
	// UseResFile(old_resfile);
	ImagesResources.Pop();
	
	return Success;
	// return picture;
}

void set_scenario_images_file(FileSpecifier& File)
//	FileDesc *file)
{

	File.Open(ScenarioResources);
	/*
	static boolean installed= FALSE;
	boolean was_aliased, is_folder; 

	if(scenario_file_handle!=NONE)
	{
		CloseResFile(scenario_file_handle);
		scenario_file_handle= NONE;
	}
	
	ResolveAliasFile((FSSpec *) file, TRUE, &is_folder, &was_aliased);
	scenario_file_handle= FSpOpenResFile((FSSpec *) file, fsRdPerm);
	// LP change: no need to bomb out
	// assert(!ResError());
	*/
	
	return;
}

bool get_sound_resource_from_scenario(short resource_number, LoadedResource& SoundRsrc)
// SndListHandle get_sound_resource_from_scenario(
//	short resource_number)
{
	// SndListHandle sound_handle;
	// short old_resfile;

	if (!ScenarioResources.IsOpen()) return false;	
	ScenarioResources.Push();

	bool Success = ScenarioResources.Get('snd ', resource_number, SoundRsrc);
	
	// Trying to keep this HNoPurge, for whatever reason
	if (Success)
	{
		Handle SndHdl = SoundRsrc.GetHandle();
		if (SndHdl) HNoPurge(SndHdl);
	}
	// LP: more graceful degradation
	// if (scenario_file_handle == NONE) return NULL;
	// assert(scenario_file_handle != NONE);
	// old_resfile= CurResFile();
	// UseResFile(scenario_file_handle);
	// sound_handle= (SndListHandle) Get1Resource('snd ', resource_number);
	// if (sound_handle) HNoPurge((Handle)sound_handle);
	// UseResFile(old_resfile);
	
	ScenarioResources.Pop();
	
	// return sound_handle;
	return Success;
}

bool get_picture_resource_from_scenario(short base_resource, LoadedResource& PictRsrc)
// PicHandle get_picture_resource_from_scenario(
// 	short base_resource)
{
	// short old_resfile;
	PicHandle picture;

	if (!ScenarioResources.IsOpen()) return false;
	ScenarioResources.Push();
	
	short RsrcID = determine_pict_resource_id(
		ScenarioResources,
		'PICT', base_resource,
		_images_file_delta16, _images_file_delta32);
	bool WasSuccessful = ScenarioResources.Get('PICT',RsrcID,PictRsrc);
	
	// Trying to keep this HNoPurge, for whatever reason
	if (WasSuccessful)
	{
		Handle PictHdl = PictRsrc.GetHandle();
		if (PictHdl) HNoPurge(PictHdl);
	}
	// LP: more graceful degradation
	// if (scenario_file_handle == NONE) return NULL;
	// assert(scenario_file_handle != NONE);
	// old_resfile= CurResFile();
	// UseResFile(scenario_file_handle);
	// picture= (PicHandle) Get1Resource('PICT', 
	//	determine_pict_resource_id('PICT', base_resource,
	//		_scenario_file_delta16, _scenario_file_delta32));
	// if (picture) HNoPurge((Handle)picture);
	// UseResFile(old_resfile);
	
	return WasSuccessful;
	// return picture;
}

struct color_table *calculate_picture_clut(
	int CLUTSource,
	short pict_resource_number)
{
	// Handle resource;
	struct color_table *picture_table= NULL;
	
	// Select the source
	OpenedResourceFile *OFilePtr = NULL;
	switch(CLUTSource)
	{
	case CLUTSource_Images:
		OFilePtr = &ImagesResources;
		break;
		
	case CLUTSource_Scenario:
		OFilePtr = &ScenarioResources;
		break;
	
	default:
		vassert(false,csprintf(temporary,"Invalid resource-file selector: %d",CLUTSource));
	}
	
	LoadedResource CLUT_Rsrc;
	if (OFilePtr->Get('clut',pict_resource_number,CLUT_Rsrc))
	// resource= GetResource('clut', pict_resource_number);
	// if (resource)
	{
		Handle resource = CLUT_Rsrc.GetHandle();
		
		HNoPurge(resource);
		
		/* Allocate the space.. */
		picture_table= new color_table;
		assert(picture_table);

		if(interface_bit_depth==8)
		{
			build_color_table(picture_table, (CTabHandle)resource);
		} else {
			build_direct_color_table(picture_table, interface_bit_depth);
		}
		
		// Automatic from LoadedResource going out of scope
		// ReleaseResource(resource);
	}
	
	return picture_table;
}

struct color_table *build_8bit_system_color_table(
	void)
{
	struct color_table *system_colors;
	CTabHandle clut= GetCTable(8);
	
	system_colors= new color_table;
	assert(system_colors);
	
	build_color_table(system_colors, clut);
	DisposeCTable(clut);
	
	return system_colors;
}

boolean images_picture_exists(
	short base_resource)
{
	LoadedResource PictRsrc;
	return get_picture_resource_from_images(base_resource,PictRsrc);
	
	/*
	PicHandle picture= get_picture_resource_from_images(base_resource);
	boolean exists= FALSE;
	
	if(picture)
	{
		exists= TRUE;
		ReleaseResource((Handle) picture);
	}
	
	return exists;
	*/
}

boolean scenario_picture_exists(
	short base_resource)
{
	LoadedResource PictRsrc;
	return get_picture_resource_from_scenario(base_resource,PictRsrc);
	
	/*
	PicHandle picture= get_picture_resource_from_scenario(base_resource);
	boolean exists= FALSE;
	
	if(picture)
	{
		exists= TRUE;
		ReleaseResource((Handle) picture);
	}

	return exists;
	*/
}

void draw_full_screen_pict_resource_from_images(
	short pict_resource_number)
{
	LoadedResource PictRsrc;
	get_picture_resource_from_images(pict_resource_number,PictRsrc);
	draw_picture(PictRsrc);
	/*
	PicHandle picture;

	picture= get_picture_resource_from_images(pict_resource_number);
	draw_picture(picture);
	*/
	
	return;
}

void draw_full_screen_pict_resource_from_scenario(
	short pict_resource_number)
{
	LoadedResource PictRsrc;
	get_picture_resource_from_scenario(pict_resource_number,PictRsrc);
	draw_picture(PictRsrc);
	/*
	LoadedResource PictRsrc;
	PicHandle picture;
	
	picture= get_picture_resource_from_scenario(pict_resource_number);
	draw_picture(picture);
	*/
	
	return;
}

// pixels/second
#define SCROLLING_SPEED (MACHINE_TICKS_PER_SECOND/20)

void scroll_full_screen_pict_resource_from_scenario(
	short pict_resource_number,
	boolean text_block)
{
	LoadedResource PictRsrc;
	get_picture_resource_from_scenario(pict_resource_number, PictRsrc);
	// PicHandle picture= get_picture_resource_from_scenario(pict_resource_number);
	
	// if (picture)
	if (PictRsrc.IsLoaded())
	{
		PicHandle picture = PicHandle(PictRsrc.GetHandle());
	
		short picture_width= RECTANGLE_WIDTH(&(*picture)->picFrame);
		short picture_height= RECTANGLE_HEIGHT(&(*picture)->picFrame);
		short screen_width= 640; //RECTANGLE_WIDTH(&screen_window->portRect);
		short screen_height= 480; //RECTANGLE_HEIGHT(&screen_window->portRect);
		boolean scroll_horizontal= picture_width>screen_width;
		boolean scroll_vertical= picture_height>screen_height;
		Rect picture_frame= (*picture)->picFrame;
		
		HNoPurge((Handle)picture);
		
		OffsetRect(&picture_frame, -picture_frame.left, -picture_frame.top);
		
		if (scroll_horizontal || scroll_vertical)
		{
			GWorldPtr pixels= (GWorldPtr) NULL;
			OSErr error= NewGWorld(&pixels, interface_bit_depth, &picture_frame,
				(CTabHandle) NULL, world_device, noNewDevice);

			/* Flush the events.. */
			FlushEvents(keyDownMask|keyUpMask|autoKeyMask|mDownMask|mUpMask, 0);
			
			if (error==noErr)
			{
				CGrafPtr old_port;
				GDHandle old_device;
				PixMapHandle pixmap;
				boolean locked;
				
				GetGWorld(&old_port, &old_device);
				SetGWorld(pixels, (GDHandle) NULL);

				pixmap= GetGWorldPixMap(pixels);
				assert(pixmap);
				locked= LockPixels(pixmap);
				assert(locked);

				// HLock((Handle) picture);
				DrawPicture(picture, &picture_frame);
				// HUnlock((Handle) picture);
				SetGWorld(old_port, old_device);
				
				{
					long start_tick= TickCount();
					boolean done= FALSE;
					boolean aborted= FALSE;
					GrafPtr old_port;
					RGBColor old_forecolor, old_backcolor;
					EventRecord event;

					GetPort(&old_port);
					SetPort(screen_window);
			
					GetForeColor(&old_forecolor);
					GetBackColor(&old_backcolor);
					RGBForeColor(&rgb_black);
					RGBBackColor(&rgb_white);

					do
					{
						Rect source, destination;
						short delta;
						
						delta= (TickCount()-start_tick)/(text_block ? (2*SCROLLING_SPEED) : SCROLLING_SPEED);
						if (scroll_horizontal && delta>picture_width-screen_width) delta= picture_width-screen_width, done= TRUE;
						if (scroll_vertical && delta>picture_height-screen_height) delta= picture_height-screen_height, done= TRUE;
						
						SetRect(&destination, 0, 0, 640, 480);
						
						SetRect(&source, 0, 0, screen_width, screen_height);
						OffsetRect(&source, scroll_horizontal ? delta : 0, scroll_vertical ? delta : 0);

						CopyBits((BitMapPtr)*pixels->portPixMap, &screen_window->portBits,
							&source, &destination, srcCopy, (RgnHandle) NULL);
						
						/* You can't do this, because it calls flushevents every time.. */
//						if(wait_for_click_or_keypress(0)!=NONE) done= TRUE;

						/* Give system time.. */
						global_idle_proc();

						/* Check for events to abort.. */
						if(GetOSEvent(keyDownMask|autoKeyMask|mDownMask, &event))
						{
							switch(event.what)
							{
								case nullEvent:
									break;
									
								case mouseDown:
								case keyDown:
								case autoKey:
									aborted= TRUE;
									break;
								
								default:
									// LP change:
									assert(false);
									// halt();
									break;
							}
						}
					}
					while (!done && !aborted);

					RGBForeColor(&old_forecolor);
					RGBBackColor(&old_backcolor);
					SetPort(old_port);
				}

				// UnlockPixels(pixmap);				
				DisposeGWorld(pixels);
			}
		}
		
		// ReleaseResource((Handle)picture);
	}
					
	return;
}

/* ------------------ private code */
static short determine_pict_resource_id(
	OpenedResourceFile& OFile,
	OSType pict_resource_type,
	short base_id,
	short delta16,
	short delta32)
{
	short actual_id= base_id;
	boolean done= FALSE;
	short bit_depth= interface_bit_depth;

	while(!done)
	{
		Handle resource;
		short next_bit_depth;
	
		actual_id= base_id;
		switch(bit_depth)
		{
			case 8:	
				next_bit_depth= 0; 
				break;
				
			case 16: 
				next_bit_depth= 8;
				actual_id += delta16; 
				break;
				
			case 32: 
				next_bit_depth= 16;
				actual_id += delta32;	
				break;
				
			default: 
				// LP change:
				assert(false);
				// halt();
				break;
		}
		
		if (OFile.Check(pict_resource_type, actual_id)) done = TRUE;
		/*
		SetResLoad(FALSE);
		resource= GetResource(pict_resource_type, actual_id);
		if(resource)
		{
			ReleaseResource(resource);
			done= TRUE;
		}
		SetResLoad(TRUE);
		*/
		
		if(!done)
		{
			if(next_bit_depth)
			{
				bit_depth= next_bit_depth;
			} else {
				/* Didn't find it. Return the 8 bit version and bail.. */
				done= TRUE;
			}
		}
	}
	return actual_id;
}

static void shutdown_images_handler(
	void)
{
	ScenarioResources.Close();
	ImagesResources.Close();
	
	/*
	CloseResFile(images_file_handle);
	if(scenario_file_handle!=NONE)
	{
		CloseResFile(scenario_file_handle);
	}
	*/
	
	return;
}

static void draw_picture(LoadedResource& PictRsrc)
//	PicHandle picture)
{
	WindowPtr window= screen_window;
	
	// if (picture)
	if (PictRsrc.IsLoaded())
	{
		// Don't detach the picture handle here
		PicHandle picture = PicHandle(PictRsrc.GetHandle());
	
		Rect bounds;
		GrafPtr old_port;
		
		bounds= (*picture)->picFrame;
		AdjustRect(&window->portRect, &bounds, &bounds, centerRect);
		OffsetRect(&bounds, bounds.left<0 ? -bounds.left : 0, bounds.top<0 ? -bounds.top : 0);
//		OffsetRect(&bounds, -2*bounds.left, -2*bounds.top);
//		OffsetRect(&bounds, (RECTANGLE_WIDTH(&window->portRect)-RECTANGLE_WIDTH(&bounds))/2 + window->portRect.left,
//			(RECTANGLE_HEIGHT(&window->portRect)-RECTANGLE_HEIGHT(&bounds))/2 + window->portRect.top);

		GetPort(&old_port);
		SetPort(window);

		{
			RgnHandle new_clip_region= NewRgn();
			
			if (new_clip_region)
			{
				RgnHandle old_clip_region= window->clipRgn;

				SetRectRgn(new_clip_region, 0, 0, 640, 480);
				SectRgn(new_clip_region, old_clip_region, new_clip_region);
				window->clipRgn= new_clip_region;

				// HLock((Handle) picture);
				DrawPicture(picture, &bounds);
				// HUnlock((Handle) picture);
				
				window->clipRgn= old_clip_region;
				
				DisposeRgn(new_clip_region);
			}
		}
		
		ValidRect(&window->portRect);
		SetPort(old_port);
		
		// Don't unload the picture here
		// if (!(HGetState((Handle)picture) & 0x40)) ReleaseResource((Handle) picture);
	}

	return;
}
