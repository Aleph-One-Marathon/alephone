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

	MacOS-specific screen-drawing stuff
	Created by Loren Petrich, Dec. 23, 2000
	
	There is a parallel SDL file

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
	Included Steve Bytnar's OSX QDPort flushing code
*/

#include "my32bqd.h"

// Truly global globals for screen_drawing
extern GWorldPtr world_pixels;
extern GWorldPtr HUD_Buffer;

// Globals local to screen_drawing
static GWorldPtr old_graphics_port= NULL;
static GDHandle old_graphics_device= NULL;
static GrafPtr destination_graphics_port= NULL;


// Sets current font to this index of interface font;
// used in computer_interface.cpp
extern void UseInterfaceFont(short font_index);


// Code

void _set_port_to_screen_window(
	void)
{
	assert(!old_graphics_port && !old_graphics_device && !destination_graphics_port);
	GetGWorld(&old_graphics_port, &old_graphics_device);
	SetGWorld((GWorldPtr) GetScreenGrafPort(), NULL);
	destination_graphics_port= (GrafPtr) GetScreenGrafPort();
}

void _set_port_to_gworld(
	void)
{
	assert(!old_graphics_port && !old_graphics_device && !destination_graphics_port);
	GetGWorld(&old_graphics_port, &old_graphics_device);
	SetGWorld((GWorldPtr) world_pixels, NULL);
	destination_graphics_port= (GrafPtr) world_pixels;
}

void _restore_port(
	void)
{
	// LP: kludge for recognizing when one had been drawing into the HUD
//#if defined(USE_CARBON_ACCESSORS)
	if (destination_graphics_port == (GrafPtr)HUD_Buffer)
		SetOrigin(0,0);
/*
#else
	if (destination_graphics_port == (GrafPort *)HUD_Buffer)
		SetOrigin(0,0);
#endif
*/
	assert(old_graphics_port && old_graphics_device && destination_graphics_port);
	SetGWorld(old_graphics_port, old_graphics_device);
	old_graphics_port= NULL;
	old_graphics_device= NULL;
	destination_graphics_port= NULL;
}

/* If source==NULL, source= the shapes bounding rectangle */
void _draw_screen_shape(
	shape_descriptor shape_id, 
	screen_rectangle *destination, 
	screen_rectangle *source)
{
	PixMapHandle pixmap;
	RGBColor old_fore, old_back;
	Rect actual_source;

	/* Avoid unwanted coloring.. */
	GetForeColor(&old_fore);
	GetBackColor(&old_back);
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);
	
	/* Draw the panels... */
	pixmap= get_shape_pixmap(shape_id, false);
	if (!pixmap)
	{
		/* Restore the colors.. */
		RGBForeColor(&old_fore);
		RGBBackColor(&old_back);
		return;
	}
	
	if(!source)
	{
		actual_source= (*pixmap)->bounds;
/*
#ifdef DEBUG
{
	short dest_width, source_width;
	short dest_height, source_height;
	
	dest_width= destination->right-destination->left;
	source_width= (*pixmap)->bounds.right-(*pixmap)->bounds.left;
	dest_height= destination->bottom-destination->top;
	source_height= (*pixmap)->bounds.bottom-(*pixmap)->bounds.top;
	if(source_height != dest_height || source_width != dest_width)
	{
		dprintf("Changing size of %d Original: %d %d New: %d %d", shape_id,
			source_width, source_height, dest_width, dest_height);
	}
}
#endif
*/
	} else {
		actual_source= *((Rect *) source);
	}

	assert(destination_graphics_port);
//#if defined(USE_CARBON_ACCESSORS)
	CopyBits((BitMapPtr) *pixmap, GetPortBitMapForCopyBits(destination_graphics_port),
		&actual_source, (Rect *) destination, srcCopy, (RgnHandle) nil);
	/* flush part of the port */
	FlushGrafPortRect(destination_graphics_port, *(Rect*)destination);
/*
#else
	CopyBits((BitMapPtr) *pixmap, &destination_graphics_port->portBits,
		&actual_source, (Rect *) destination, srcCopy, (RgnHandle) nil);
#endif
*/

	/* Restore the colors.. */
	RGBForeColor(&old_fore);
	RGBBackColor(&old_back);
}

#define NUMBER_OF_PLAYER_COLORS 8
void _get_player_color(
	size_t color_index,
	RGBColor *color)
{
	assert(color_index>=0 && color_index<NUMBER_OF_PLAYER_COLORS);
	_get_interface_color(color_index+PLAYER_COLOR_BASE_INDEX, color);
}

void _get_interface_color(
	size_t color_index, 
	RGBColor *color)
{	
	// LP change: from internal color array
	assert(color_index>=0 && static_cast<int>(color_index)<NumInterfaceColors);
	
	rgb_color &Color = InterfaceColors[color_index];
	color->red = Color.red;
	color->green = Color.green;
	color->blue = Color.blue;
}

void _scroll_window(
	short dy, 
	short rectangle_id, 
	short background_color_index)
{
	Rect *destination= _get_interface_rect(rectangle_id);
	RgnHandle updateRgn;
	RGBColor old_color, new_color;

	GetBackColor(&old_color);
	_get_interface_color(background_color_index, &new_color);
	RGBBackColor(&new_color);

	updateRgn= NewRgn();
	ScrollRect(destination, 0, dy, updateRgn);
	DisposeRgn(updateRgn);

	RGBBackColor(&old_color);
}

void _fill_screen_rectangle(
	screen_rectangle *rectangle, 
	short color_index)
{
	RGBColor old_color, new_color;

	GetForeColor(&old_color);
	_get_interface_color(color_index, &new_color);
	RGBForeColor(&new_color);
	PaintRect((Rect *) rectangle);
	RGBForeColor(&old_color);
}

void _draw_screen_shape_at_x_y(
	shape_descriptor shape,
	short x,
	short y)
{
	PixMapHandle pixmap;
	RGBColor old_fore, old_back;
	Rect destination;

	/* Avoid unwanted coloring.. */
	GetForeColor(&old_fore);
	GetBackColor(&old_back);
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);
	
	/* Draw the panels... */
	pixmap= get_shape_pixmap(shape, false);
	if (!pixmap)
	{
		/* Restore the colors.. */
		RGBForeColor(&old_fore);
		RGBBackColor(&old_back);
		return;
	}
	
	/* Offset to zero base, and add in x, y */
	destination= (*pixmap)->bounds;
	OffsetRect(&destination, x-destination.left, y-destination.top);
	
	/* Slam the puppy...  */
	assert(destination_graphics_port);
//#if defined(USE_CARBON_ACCESSORS)
	CopyBits((BitMapPtr) *pixmap, GetPortBitMapForCopyBits(destination_graphics_port),
		&(*pixmap)->bounds, &destination, srcCopy, (RgnHandle) nil);
	/* flush part of the port */
	FlushGrafPortRect(destination_graphics_port, destination);
/*
#else
	CopyBits((BitMapPtr) *pixmap, &destination_graphics_port->portBits, //&screen_window->portBits,
		&(*pixmap)->bounds, &destination, srcCopy, (RgnHandle) nil);
#endif
*/

	/* Restore the colors.. */
	RGBForeColor(&old_fore);
	RGBBackColor(&old_back);
}

#ifdef UNUSED
void _draw_screen_shape_centered(
	shape_descriptor shape,
	screen_rectangle *rectangle,
	short flags)
{
	PixMapHandle pixmap;
	RGBColor old_fore, old_back;
	Rect destination, source;
	short left_offset, top_offset;
	
	/* Avoid unwanted coloring.. */
	GetForeColor(&old_fore);
	GetBackColor(&old_back);
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);
	
	/* Draw the panels... */
	pixmap= get_shape_pixmap(shape, false);
	if (!pixmap)
	{
		/* Restore the colors.. */
		RGBForeColor(&old_fore);
		RGBBackColor(&old_back);
		return;
	}
	
	/* Offset to zero base, and add in x, y */
	destination= source= (*pixmap)->bounds;
	
	if(flags & _center_horizontal)
	{
		left_offset= (RECTANGLE_WIDTH(rectangle)-RECTANGLE_WIDTH(&source))/2;
	} else {
		left_offset= 0;
	}
	
	if(flags & _center_vertical)
	{
		top_offset= (RECTANGLE_HEIGHT(rectangle)-RECTANGLE_HEIGHT(&source))/2;
	} else if (flags & _bottom_justified) {
		top_offset= RECTANGLE_HEIGHT(rectangle)-RECTANGLE_HEIGHT(&destination);
	} else {
		top_offset= 0;
	}
	
	OffsetRect(&destination, rectangle->left+left_offset, 
		rectangle->top+top_offset);
	
	/* Slam the puppy...  */
	assert(destination_graphics_port);
	CopyBits((BitMapPtr) *pixmap, &destination_graphics_port->portBits, // &screen_window->portBits,
		&source, &destination, srcCopy, (RgnHandle) nil);

	/* Restore the colors.. */
	RGBForeColor(&old_fore);
	RGBBackColor(&old_back);
}
#endif

short _text_width(
	const char *buffer, 
	short font_id)
{
	TextSpec old_font;
	short width;

	assert(font_id>=0 && font_id<NUMBER_OF_INTERFACE_FONTS);
	
	GetFont(&old_font);
	InterfaceFonts[font_id].Use();
	// SetFont(&interface_fonts.fonts[font_id]);
	width= TextWidth(buffer, 0, strlen(buffer));
	SetFont(&old_font);

	return width;
}

/* This expects a cstring. Draws to the current port*/
void _draw_screen_text(
	const char *text,
	screen_rectangle *destination,
	short flags,
	short font_id,
	short text_color)
{
	TextSpec old_font;
	short x, y;
	RGBColor old_color, new_color;
	char text_to_draw[256];

	assert(font_id>=0 && font_id<NUMBER_OF_INTERFACE_FONTS);
	
	GetFont(&old_font);
	InterfaceFonts[font_id].Use();
	// SetFont(&interface_fonts.fonts[font_id]);

	GetForeColor(&old_color);
	_get_interface_color(text_color, &new_color);
	RGBForeColor(&new_color);

	/* Copy the text to draw.. */
	strcpy(text_to_draw, text);

	/* Check for wrapping, and if it occurs, be recursive... */
	if(flags & _wrap_text)
	{
/*еее WHAT IS THE INTERNATIONALIZED WAY OF DETERMINING NON-PRINTING CHARACTERS? IE SPACES? */
		short last_non_printing_character;
		short text_width;
		unsigned short count= 0;

		text_width= 0;
		last_non_printing_character= 0;
		count= 0;
		while(count<strlen(text_to_draw) && text_width<RECTANGLE_WIDTH(destination))
		{
			text_width+= CharWidth(text_to_draw[count]);
			if(text_to_draw[count]==' ') last_non_printing_character= count;
			count++;
		}
		
		if(count!=strlen(text_to_draw))
		{
			char remaining_text_to_draw[256];
			screen_rectangle new_destination;
			
			/* If we ever have to wrap text, we can't also center vertically.  Sorry */
			flags &= ~_center_vertical;
			flags |= _top_justified;
			
			/* Pass the rest of it back in, recursively, on the next line.. */
			BlockMove(&text_to_draw[last_non_printing_character+1], remaining_text_to_draw,
				strlen(&text_to_draw[last_non_printing_character+1])+1);
	
			new_destination= *destination;
			new_destination.top+= InterfaceFonts[font_id].LineSpacing;
			// new_destination.top+= interface_fonts.line_spacing[font_id];
			_draw_screen_text(remaining_text_to_draw, &new_destination, flags, font_id, text_color);
	
			/* now truncate our text to draw...*/
			text_to_draw[last_non_printing_character]= 0;
		}
	}

	/* Handle the horizontal stuff. */
	if(flags & _center_horizontal || flags & _right_justified)
	{
		short text_width;
		
		text_width= TextWidth(text_to_draw, 0, strlen(text_to_draw));
		
		if(text_width>RECTANGLE_WIDTH(destination))
		{
			short length;
			short trunc_code;
	
			/* Truncate the puppy.. */	
			x= destination->left;
			length= strlen(text);
			trunc_code= TruncText(RECTANGLE_WIDTH(destination), text_to_draw, &length, truncEnd);
			text_to_draw[length]= 0;

			/* Now recenter it.. */
			text_width= TextWidth(text_to_draw, 0, strlen(text_to_draw));
			if (flags & _center_horizontal)
				x= destination->left+(((destination->right-destination->left)-text_width)>>1);
			else
				x= destination->right - text_width;
		} else {
			if (flags & _center_horizontal)
				x= destination->left+(((destination->right-destination->left)-text_width)>>1);
			else
				x= destination->right - text_width;
		}
	} else {
		x= destination->left;

/* New addition.  Hopefully this is okay.. */
		if(TextWidth(text_to_draw, 0, strlen(text_to_draw))>RECTANGLE_WIDTH(destination))
		{
			short length;
			short trunc_code;
	
			/* Truncate the puppy.. */	
			length= strlen(text);
			trunc_code= TruncText(RECTANGLE_WIDTH(destination), text_to_draw, &length, truncEnd);
			text_to_draw[length]= 0;
		} 
	}
	
	if(flags & _center_vertical)
	{
		short text_height;
		short offset;

		text_height= InterfaceFonts[font_id].Height;
		// text_height= interface_fonts.heights[font_id];
		if(text_height>RECTANGLE_HEIGHT(destination))
		{
			/* too tall, we punt. */
			y= destination->bottom;
		} else {
			y= destination->bottom;
			offset= RECTANGLE_HEIGHT(destination)-text_height;
			y-= ((offset>>1) + (offset&1) + 1);
		}
	} else if (flags & _top_justified) {
		short text_height;

		text_height= InterfaceFonts[font_id].Height;
		// text_height= interface_fonts.heights[font_id];
		if(text_height>RECTANGLE_HEIGHT(destination))
		{
			/* too tall, we punt. */
			y= destination->bottom;
		} else {
			y= destination->top+text_height;
		}
	} else {
		y= destination->bottom;
	}

	/* Now draw it. */
	MoveTo(x, y);
	DrawText(text_to_draw, 0, strlen(text_to_draw));
	
	/* Restore.. */
	RGBForeColor(&old_color);
	SetFont(&old_font);
}

void _erase_screen(
	short color_index)
{
//#if defined(USE_CARBON_ACCESSORS)
	Rect rect;
	GetPortBounds(GetScreenGrafPort(), &rect);
	_fill_rect((screen_rectangle *)&rect, color_index);
/*
#else
	_fill_rect((screen_rectangle *) &GetScreenGrafPort()->portRect, color_index);
#endif
*/
//#if defined(TARGET_API_MAC_CARBON)
	CGrafPtr curPort;
	GetPort(&curPort);
	if (QDIsPortBuffered(curPort))
	{
		RgnHandle rgn = NewRgn();
		RectRgn(rgn, &rect);
		QDFlushPortBuffer(curPort, rgn);
		DisposeRgn(rgn);
		
		// Try and wait for the screen to redraw
		RunCurrentEventLoop(kEventDurationSecond/4);
	}
//#endif
}

short _get_font_line_height(
	short font_index)
{
	assert(font_index>=0 && font_index<NUMBER_OF_INTERFACE_FONTS);
	return InterfaceFonts[font_index].LineSpacing;
	// return interface_fonts.line_spacing[font_index];
}

void _fill_rect(
	screen_rectangle *rectangle, 
	short color_index)
{
	RGBColor old_color, new_color;

	_get_interface_color(color_index, &new_color);
	
	GetForeColor(&old_color);
	RGBForeColor(&new_color);
	PaintRect((Rect *) rectangle);
	RGBForeColor(&old_color);
}

void _frame_rect(
	screen_rectangle *rectangle,
	short color_index)
{
	RGBColor old_color, new_color;

	_get_interface_color(color_index, &new_color);
	
	GetForeColor(&old_color);
	RGBForeColor(&new_color);
	FrameRect((Rect *) rectangle);
	RGBForeColor(&old_color);
}

static TextSpec NullSpec = {0, 0, 0};

TextSpec *_get_font_spec(
	short font_index)
{
	// return &(interface_fonts.fonts[font_index]);
	return &NullSpec;
}

// Sets current font to this index of interface font;
// used in computer_interface.cpp
void UseInterfaceFont(short font_index)
{
	assert(font_index>=0 && font_index<NUMBER_OF_INTERFACE_FONTS);
	
	InterfaceFonts[font_index].Use();
}

/* ------- Private prototypes */
static void load_interface_rectangles(
	void) 
{
	// All this is now in XML hands (a whole lot of unnecessary crap deleted)
}

static Rect *_get_interface_rect(
	short index)
{
	return (Rect *) get_interface_rectangle(index);
}

#if 0
static short _get_font_height(
	TextSpec *font)
{
	FontInfo info;
	TextSpec old_font;
	
	GetFont(&old_font);
	SetFont(font);
	GetFontInfo(&info);
	SetFont(&old_font);
	
	return info.ascent+info.leading;
}

static short _get_font_line_spacing(
	TextSpec *font)
{
	FontInfo info;
	TextSpec old_font;
	
	GetFont(&old_font);
	SetFont(font);
	GetFontInfo(&info);
	SetFont(&old_font);
	
	return info.ascent+info.descent+info.leading;
}
#endif

static void	load_screen_interface_colors(
	void)
{
	// All this is now in XML hands
}


// LP addition: set the graphics port to the Heads-Up-Display buffer
void _set_port_to_HUD()
{
	// Do lazy allocation:
	Rect HUD_Bounds = {320, 0, 480, 640};
	if (!HUD_Buffer)
	{
		short error= myNewGWorld(&HUD_Buffer, 0, &HUD_Bounds, (CTabHandle) NULL, (GDHandle) NULL, 0);
		if (error!=noErr) alert_user(fatalError, strERRORS, outOfMemory, error);
	}
	assert(HUD_Buffer);
	
	assert(!old_graphics_port && !old_graphics_device && !destination_graphics_port);
	GetGWorld(&old_graphics_port, &old_graphics_device);
	SetGWorld(HUD_Buffer, NULL);
	destination_graphics_port= (GrafPtr)HUD_Buffer;
	SetOrigin(0, 320);
}
