/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/

/*
 *  images_macintosh.cpp - Image management, MacOS implementation (included by images.cpp)
 */


// From screen.c
extern short interface_bit_depth;
extern WindowPtr screen_window;


/*
 *  Get system color table
 */

struct color_table *build_8bit_system_color_table(void)
{
	/*
	struct color_table *system_colors;
	CTabHandle clut= GetCTable(8);
	
	system_colors= new color_table;
	assert(system_colors);
	
	build_color_table(system_colors, clut);
	DisposeCTable(clut);
	*/
	
	LoadedResource Default_CLUT_Rsrc;
	Default_CLUT_Rsrc.GetSystemColorTable();
	assert(Default_CLUT_Rsrc.IsLoaded());
	
	color_table *system_colors = new color_table;
	build_color_table(system_colors, Default_CLUT_Rsrc);
	
	return system_colors;
}


/*
 *  Scroll image across screen
 */

// pixels/second
#define SCROLLING_SPEED (MACHINE_TICKS_PER_SECOND/20)

void scroll_full_screen_pict_resource_from_scenario(
	int pict_resource_number,
	bool text_block)
{
	LoadedResource PictRsrc;
	get_picture_resource_from_scenario(pict_resource_number, PictRsrc);
	
	if (PictRsrc.IsLoaded())
	{
		PicHandle picture = PicHandle(PictRsrc.GetHandle());
	
		short picture_width= RECTANGLE_WIDTH(&(*picture)->picFrame);
		short picture_height= RECTANGLE_HEIGHT(&(*picture)->picFrame);
		short screen_width= 640; //RECTANGLE_WIDTH(&screen_window->portRect);
		short screen_height= 480; //RECTANGLE_HEIGHT(&screen_window->portRect);
		bool scroll_horizontal= picture_width>screen_width;
		bool scroll_vertical= picture_height>screen_height;
		Rect picture_frame= (*picture)->picFrame;
		
		HNoPurge((Handle)picture);
		
		OffsetRect(&picture_frame, -picture_frame.left, -picture_frame.top);
		
		if (scroll_horizontal || scroll_vertical)
		{
			GWorldPtr pixels= (GWorldPtr) NULL;
			OSErr error= NewGWorld(&pixels, interface_bit_depth, &picture_frame,
				(CTabHandle) NULL, GetWorldDevice(), noNewDevice);

			/* Flush the events.. */
			FlushEvents(keyDownMask|keyUpMask|autoKeyMask|mDownMask|mUpMask, 0);
			
			if (error==noErr)
			{
				CGrafPtr old_port;
				GDHandle old_device;
				PixMapHandle pixmap;
				bool locked;
				
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
					bool done= false;
					bool aborted= false;
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
						if (scroll_horizontal && delta>picture_width-screen_width) delta= picture_width-screen_width, done= true;
						if (scroll_vertical && delta>picture_height-screen_height) delta= picture_height-screen_height, done= true;
						
						SetRect(&destination, 0, 0, 640, 480);
						
						SetRect(&source, 0, 0, screen_width, screen_height);
						OffsetRect(&source, scroll_horizontal ? delta : 0, scroll_vertical ? delta : 0);

						CopyBits((BitMapPtr)*pixels->portPixMap, &screen_window->portBits,
							&source, &destination, srcCopy, (RgnHandle) NULL);
						
						/* You can't do this, because it calls flushevents every time.. */
//						if(wait_for_click_or_keypress(0)!=NONE) done= true;

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
									aborted= true;
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


/*
 *  Draw picture resource centered on screen
 */

static void draw_picture(LoadedResource& PictRsrc)
{
	WindowPtr window= screen_window;
	
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
	}
}
