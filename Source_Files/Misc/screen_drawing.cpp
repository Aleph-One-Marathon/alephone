/*
	SCREEN_DRAWING.C
	Monday, August 15, 1994 1:55:21 PM
 
    Wednesday, August 24, 1994 12:50:20 AM (ajr)
	  added _right_justified for _draw_screen_text
	Thursday, June 22, 1995 8:45:41 AM- note that we no longer hold your hand and set the port
		for you.  We have a grafptr and a restore ptr call.\

Apr 30, 2000 (Loren Petrich):
	Added XML-parser support (actually, some days earlier, but had modified it
	so as to have "interface" be defined in "game_window".

Jul 2, 2000 (Loren Petrich):
	The HUD is now always buffered; it is lazily allocated

Oct 19, 2000 (Loren Petrich):
	Added graceful degradation if get_shape_pixmap() returns NULL; CB had already done that
	with the SDL version.
*/

#pragma segment drawing

#include "macintosh_cseries.h"

#include "map.h"
#include "interface.h"
#include "shell.h"
#include "screen_drawing.h"
#include "fades.h"
#include "screen.h"
#include "my32bqd.h"

// LP addition: color parser
#include "ColorParser.h"

#include <string.h>

#define clutSCREEN_COLORS 130
#define finfFONTS 128

extern TextSpec *_get_font_spec(short font_index);

struct interface_font_info 
{
	TextSpec fonts[NUMBER_OF_INTERFACE_FONTS];
	short heights[NUMBER_OF_INTERFACE_FONTS];
	short line_spacing[NUMBER_OF_INTERFACE_FONTS];
};

extern GWorldPtr world_pixels;

// LP addition: the Heads-Up-Display buffer
extern GWorldPtr HUD_Buffer;

/* --------- Globals. */
// LP change: hardcoding this quantity since we know how many we need
static screen_rectangle interface_rectangles[NUMBER_OF_INTERFACE_RECTANGLES];
// static screen_rectangle *interface_rectangles;
// LP change: now hardcoded and XML-changeable
// static CTabHandle screen_colors;
static struct interface_font_info interface_fonts;
static GWorldPtr old_graphics_port= NULL;
static GDHandle old_graphics_device= NULL;
static GrafPtr destination_graphics_port= NULL;

// LP change: hardcoding the interface and player colors,
// so as to banish the 'clut' resources
const int NumInterfaceColors = 26;
static rgb_color InterfaceColors[NumInterfaceColors] = 
{
	{0, 65535, 0},
	{0, 5140, 0},
	{0, 0, 0},
	
	{0, 65535, 0},
	{0, 12956, 0},
	{0, 5100, 0},
	
	{9216, 24320, 41728},
	{65535, 0, 0},
	{45056, 0, 24064},
	{65535, 65535, 0},
	{60000, 60000, 60000},
	{62976, 22528, 0},
	{3072, 0, 65535},
	{0, 65535, 0},
	
	{65535, 65535, 65535},
	{0, 5140, 0},
	
	{10000, 0, 0},
	{65535, 0, 0},
	
	{0, 65535, 0},
	{65535, 65535, 65535},
	{65535, 0, 0},
	{0, 40000, 0},
	{0, 45232, 51657},
	{65535, 59367, 0},
	{45000, 0, 0},
	{3084, 0, 65535}
};

/* ------- Private prototypes */
static void load_interface_rectangles(void);
static Rect *_get_interface_rect(short index);
static short _get_font_height(TextSpec *font);
static short _get_font_line_spacing(TextSpec *font);
static void	load_screen_interface_colors(void);

/* -------- Code */
void initialize_screen_drawing(
	void)
{
	short loop;

	/* Load the rectangles */
	load_interface_rectangles();
	
	/* Load the colors */
	load_screen_interface_colors();
	
	/* load the font stuff. */
	for(loop=0; loop<NUMBER_OF_INTERFACE_FONTS; ++loop)
	{
		GetNewTextSpec(&interface_fonts.fonts[loop], finfFONTS, loop);
		interface_fonts.heights[loop]= _get_font_height(&interface_fonts.fonts[loop]);
		interface_fonts.line_spacing[loop]= _get_font_line_spacing(&interface_fonts.fonts[loop]);
	}
}

void _set_port_to_screen_window(
	void)
{
	assert(!old_graphics_port && !old_graphics_device && !destination_graphics_port);
	GetGWorld(&old_graphics_port, &old_graphics_device);
	SetGWorld((GWorldPtr) screen_window, NULL);
	destination_graphics_port= (GrafPtr) screen_window;
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
	CopyBits((BitMapPtr) *pixmap, &destination_graphics_port->portBits,
		&actual_source, (Rect *) destination, srcCopy, (RgnHandle) nil);

	/* Restore the colors.. */
	RGBForeColor(&old_fore);
	RGBBackColor(&old_back);
}

#define NUMBER_OF_PLAYER_COLORS 8
void _get_player_color(
	short color_index,
	RGBColor *color)
{
	assert(color_index>=0 && color_index<NUMBER_OF_PLAYER_COLORS);
	_get_interface_color(color_index+PLAYER_COLOR_BASE_INDEX, color);
}

void _get_interface_color(
	short color_index, 
	RGBColor *color)
{	
	// LP change: from internal color array
	assert(color_index>=0 && color_index<NumInterfaceColors);
	
	rgb_color &Color = InterfaceColors[color_index];
	color->red = Color.red;
	color->green = Color.green;
	color->blue = Color.blue;
	
	/*
	assert(screen_colors);
	assert(color_index>=0 && color_index<=(*screen_colors)->ctSize);

	*color= (*screen_colors)->ctTable[color_index].rgb;
	*/
	
	return;
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
	CopyBits((BitMapPtr) *pixmap, &destination_graphics_port->portBits, //&screen_window->portBits,
		&(*pixmap)->bounds, &destination, srcCopy, (RgnHandle) nil);

	/* Restore the colors.. */
	RGBForeColor(&old_fore);
	RGBBackColor(&old_back);
}

void _draw_screen_shape_centered(
	shape_descriptor shape,
	screen_rectangle *rectangle,
	short flags)
{
	PixMapHandle pixmap;
	RGBColor old_fore, old_back;
	Rect destination, source;
	short left_offset, top_offset;
return;
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

short _text_width(
	const char *buffer, 
	short font_id)
{
	TextSpec old_font;
	short width;

	assert(font_id>=0 && font_id<NUMBER_OF_INTERFACE_FONTS);
	
	GetFont(&old_font);
	SetFont(&interface_fonts.fonts[font_id]);
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
	SetFont(&interface_fonts.fonts[font_id]);

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
		short count= 0;

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
			new_destination.top+= interface_fonts.line_spacing[font_id];
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

		text_height= interface_fonts.heights[font_id];
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

		text_height= interface_fonts.heights[font_id];
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

screen_rectangle *get_interface_rectangle(
	short index)
{
	assert(index>=0 && index<NUMBER_OF_INTERFACE_RECTANGLES);
	return interface_rectangles+index;
}

void _erase_screen(
	short color_index)
{
	_fill_rect((screen_rectangle *) &screen_window->portRect, color_index);
}

short _get_font_line_height(
	short font_index)
{
	assert(font_index>=0 && font_index<NUMBER_OF_INTERFACE_FONTS);
	return interface_fonts.line_spacing[font_index];
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

void _offset_screen_rect(
	screen_rectangle *rect, 
	short dx, 
	short dy)
{
	OffsetRect((Rect *) rect, dx, dy);
}

TextSpec *_get_font_spec(
	short font_index)
{
	return &(interface_fonts.fonts[font_index]);
}


#ifdef OBSOLETE
extern short interface_bit_depth;
extern short bit_depth;

bool display_full_screen_pict_resource(
	WindowPtr window,
	OSType pict_resource_type,
	short pict_resource_number,
	long delay)
{
	PicHandle picture;
	bool picture_drawn= false;
	
	picture= (PicHandle) GetResource(pict_resource_type, 
		determine_pict_resource_id(pict_resource_type, pict_resource_number));
	if (picture)
	{
		CTabHandle clut;
		
		if (interface_bit_depth==8)
		{
#if 0
			PictInfo info;
			OSErr error;
			
			error= GetPictInfo(picture, &info, returnColorTable, 256, popularMethod, 0);
			assert(error==noErr);

			clut= info.theColorTable;
#endif
			clut= (CTabHandle) GetResource('clut', pict_resource_number);
			if (clut)
			{
				MoveHHi((Handle)clut);
				HLock((Handle)clut);
			}
		}
		else
		{
			clut= world_color_table;
		}
		
		if (clut)
		{
			GrafPtr old_port;

			GetPort(&old_port);
			SetPort(window);
			
			PaintRect(&window->portRect);
	
			if (interface_bit_depth==8) assert_world_color_table(clut, world_color_table);
			full_fade(_start_cinematic_fade_in, clut);

			draw_full_screen_pict_resource(window, pict_resource_type, pict_resource_number);
			picture_drawn= true;
	
			full_fade(_long_cinematic_fade_in, clut);

			if (delay>0)
			{
				wait_for_click_or_keypress(delay);
		
				full_fade(_cinematic_fade_out, clut);
				PaintRect(&window->portRect);
				full_fade(_end_cinematic_fade_out, clut);
				if (interface_bit_depth==8) assert_world_color_table(interface_color_table, world_color_table);
			}
			
			SetPort(old_port);
		}

		if (bit_depth==8 && clut) ReleaseResource((Handle)clut);
	}
	
	return picture_drawn;
}
#endif

/* ------- Private prototypes */
static void load_interface_rectangles(
	void) 
{
	// All this is now in XML hands
	/*
	Handle rect_handle;
	short number;
	
	rect_handle= GetResource('nrct', 128); // load the rectangles 
	assert(rect_handle);
	number=(*((short *) *rect_handle)); // The number of rects is the first thing 
	interface_rectangles= (screen_rectangle *) NewPtr(number*sizeof(Rect));

	assert(interface_rectangles);
	assert(number==NUMBER_OF_INTERFACE_RECTANGLES);	
	assert(GetHandleSize(rect_handle)==(number*sizeof(screen_rectangle))+sizeof(short));

	BlockMove(((*rect_handle)+sizeof(short)), interface_rectangles, number*sizeof(screen_rectangle));
	ReleaseResource(rect_handle);
	
	// Now center all rectangles in your screen...
	*/
}

static Rect *_get_interface_rect(
	short index)
{
	return (Rect *) get_interface_rectangle(index);
}

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

static void	load_screen_interface_colors(
	void)
{
	// All this is now in XML hands
	/*
	screen_colors= (CTabHandle) GetResource('clut', clutSCREEN_COLORS);
	assert(screen_colors);
	
	HNoPurge((Handle)screen_colors);
	
	return;
	*/
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
}


// Rectangle-parser object:
class XML_RectangleParser: public XML_ElementParser
{
	screen_rectangle TempRect;
	int Index;
	bool IsPresent[5];

public:
	screen_rectangle *RectList;
	int NumRectangles;
	
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_RectangleParser(): XML_ElementParser("rect") {}
};

bool XML_RectangleParser::Start()
{
	for (int k=0; k<5; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_RectangleParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,0,NumRectangles-1))
		{
			IsPresent[4] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"top") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.top))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"left") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.left))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"bottom") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.bottom))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"right") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.right))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_RectangleParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	for (int k=0; k<5; k++)
		if (!IsPresent[k]) AllPresent = false;
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place
	RectList[Index] = TempRect;
	return true;
}

static XML_RectangleParser RectangleParser;


// LP addition: set up the parser for the interface rectangles and get it
XML_ElementParser *InterfaceRectangles_GetParser()
{
	RectangleParser.RectList = interface_rectangles;
	RectangleParser.NumRectangles = NUMBER_OF_INTERFACE_RECTANGLES;
	return &RectangleParser;
}


void SetColorParserToScreenDrawing()
{
	Color_SetArray(InterfaceColors,NumInterfaceColors);
}


