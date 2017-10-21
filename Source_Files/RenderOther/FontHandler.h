#ifndef _FONT_HANDLER_
#define _FONT_HANDLER_

/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Font handler
	by Loren Petrich,
	December 17, 2000
	
	This is for specifying and working with text fonts;
	specifying them, changing them with XML, and creating a font brush
	for OpenGL rendering

Jan 5, 2001 (Loren Petrich):
	Added a "file" field, for containing font filenames

Jan 14, 2001 (Loren Petrich):
	Removed the style definitions; will use styleNormal, styleBold, etc. from csfonts.h 
*/


#include "cseries.h"

#include "sdl_fonts.h"

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#endif

#include <set>

struct screen_rectangle;

class FontSpecifier;

class FontSpecifier
{
public:
	// Parameters:
	std::string NameSet; // unused

	short Size;
	short Style;
	short AdjustLineHeight;
	
	// For SDL font support: a font filename
	std::string File;
	
	// Derived quantities: sync with parameters by calling Update()
	short Height;			// How tall is it?
	short LineSpacing;		// From same positions in each line
	short Ascent, Descent, Leading;
	short Widths[256];
	
	font_info *Info;
	
	// Initialize: call this before calling anything else;
	// this is from not having a proper constructor for this object.
	void Init();
	
	// Do the updating: must be called before using the font; however, it is called by Init(),
	// and it will be called by the XML parser if it updates the parameters
	void Update();
	
	// Get text width for text that must be centered (map title)
	int TextWidth(const char *Text);

	// Get width of one character
	int CharWidth(char c) const { return Widths[static_cast<int>(c)]; }
	
#ifdef HAVE_OPENGL	
	// Reset the OpenGL fonts; its arg indicates whether this is for starting an OpenGL session
	// (this is to avoid texture and display-list memory leaks and other such things)
	void OGL_Reset(bool IsStarting);
	
	// Renders a C-style string in OpenGL.
	// assumes screen coordinates and that the left baseline point is at (0,0).
	// Alters the modelview matrix so that the next characters will be drawn at the proper place.
	// One can surround it with glPushMatrix() and glPopMatrix() to remember the original.
	void OGL_Render(const char *Text);

	// Renders text a la _draw_screen_text() (see screen_drawing.h), with
	// alignment and wrapping. Modelview matrix is unaffected.
	void OGL_DrawText(const char *Text, const screen_rectangle &r, short flags);
	
	// Calls OGL_Reset() on all fonts. This is used when the OpenGL context
	// is changing, so that textures and display lists are cleaned up.
	static void OGL_ResetFonts(bool IsStarting);	

	// Add or remove an instance from the registry of in-use OpenGL fonts.
	// To recycle OpenGL assets properly on context switches, the set
	// m_font_registry tracks all active fonts.
	static void OGL_Register(FontSpecifier *F);
	static void OGL_Deregister(FontSpecifier *F);
	
#endif

	// Draw text without worrying about OpenGL vs. SDL mode.
	int DrawText(SDL_Surface *s, const char *text, int x, int y, uint32 pixel, bool utf8 = false);

	~FontSpecifier();

	// Equality and assignment operators
	bool operator==(FontSpecifier& F);
	bool operator!=(FontSpecifier& F)
		{return (!((*this) == F));}
	FontSpecifier& operator=(FontSpecifier& F);
	
	// Not sure what kind of explicit constructor would be consistent with the way
	// that fonts' initial values are specified, as {nameset-string, size, style}.
	// Also, private members are inconsistent with that sort of initialization.

#ifdef HAVE_OPENGL
	// Stuff for OpenGL font rendering: the font texture and a display list for font rendering;
	// if OGL_Texture is NULL, then there is no OpenGL font texture to render.
	uint8 *OGL_Texture;
	short TxtrWidth, TxtrHeight;
	int GetTxtrSize() {return int(TxtrWidth)*int(TxtrHeight);}
	GLuint TxtrID;
	uint32 DispList;
	static std::set<FontSpecifier*> *m_font_registry;
#endif
};


#endif
