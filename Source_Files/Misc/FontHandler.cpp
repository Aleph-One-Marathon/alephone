
/*
	Font handler
	by Loren Petrich,
	December 17, 2000
	
	This is for specifying and working with text fonts

Dec 25, 2000 (Loren Petrich):
	Added OpenGL-rendering support

Dec 31, 2000 (Loren Petrich):
	Switched to a 32-bit intermediate GWorld, so that text antialiasing
	will work properly.
*/

#include <GL/gl.h>
#include <math.h>
#include <string.h>
#include "FontHandler.h"


// MacOS-specific: stuff that gets reused
// static CTabHandle Grays = NULL;

// Font-specifier equality and assignment:

bool FontSpecifier::operator==(FontSpecifier& F)
{
	if (strncmp(NameSet,F.NameSet,NameSetLen) != 0) return false;
	if (Size != F.Size) return false;
	if (Style != F.Style) return false;
	if (strncmp(File,F.File,NameSetLen) != 0) return false;
	return true;
}

FontSpecifier& FontSpecifier::operator=(FontSpecifier& F)
{
	memcpy(NameSet,F.NameSet,NameSetLen);
	Size = F.Size;
	Style = F.Style;
	memcpy(File,F.File,NameSetLen);
	return *this;
}

// Initializer: call before using because of difficulties in setting up a proper constructor:

void FontSpecifier::Init()
{
	Update();
	
	OGL_Texture = NULL;
}


// MacOS-specific:
// Cribbed from _get_font_line_spacing() and similar functions in screen_drawing.cpp

void FontSpecifier::Update()
{
	// csfonts -- push old font
	TextSpec OldFont;
	GetFont(&OldFont);
	
	// Parse the font spec to find the font ID;
	// if it is not set, then it is assumed to be the system font
	ID = 0;
	
	Str255 Name;
	
	char *NamePtr = FindNextName(NameSet);
	
	while(NamePtr)
	{
		char *NameEndPtr = FindNameEnd(NamePtr);
		
		// Make a Pascal string out of the name
		int NameLen = MIN(NameEndPtr - NamePtr, 255);
		Name[0] = NameLen;
		memcpy(Name+1,NamePtr,NameLen);
		Name[NameLen+1] = 0;
		// dprintf("Name, Len: <%s>, %d",Name+1,NameLen);
		
		// MacOS ID->name translation
		GetFNum(Name,&ID);
		// dprintf("ID = %d",ID);
		if (ID != 0) break;
		
		NamePtr = FindNextName(NameEndPtr);
	}
	
	// Get the other font features
	Use();
	FontInfo Info;
	GetFontInfo(&Info);
	
	Height = Info.ascent+Info.leading;
	LineSpacing = Info.ascent+Info.descent+Info.leading;
	
	// pop old font
	SetFont(&OldFont);
}


void FontSpecifier::Use()
{
	// MacOS-specific:
	TextFont(ID);
	TextFace(Style);
	TextSize(Size);
}


int FontSpecifier::TextWidth(char *Text)
{
	int Len = MIN(strlen(Text),255);
	// MacOS-specific; note the :: for getting the top-level function instead of a member one
	return int(::TextWidth(Text,0,Len));
}


// Next power of 2; since OpenGL prefers powers of 2, it is necessary to work out
// the next one up for each texture dimension.
inline int NextPowerOfTwo(int n)
{
	int p = 1;
	while(p < n) {p <<= 1;}
	return p;
}


#ifdef HAVE_OPENGL		
// Reset the OpenGL fonts; its arg indicates whether this is for starting an OpenGL session
// (this is to avoid texture and display-list memory leaks and other such things)
void FontSpecifier::OGL_Reset(bool IsStarting)
{
	// Don't delete these if there is no valid texture;
	// that indicates that there are no valid texture and display-list ID's.
	if (!IsStarting && !OGL_Texture)
	{
		glDeleteTextures(1,&TxtrID);
		glDeleteLists(DispList,256);
	}

	// Invalidates whatever texture had been present
	if (OGL_Texture)
	{
		delete[]OGL_Texture;
		OGL_Texture = NULL;
	}
	
	// Get horizontal and vertical extents of the glyphs;
	// will assume only 1-byte fonts.
	short Ascent = 0, Descent = 0;
	short Widths[256];
	
	// Some MacOS-specific stuff
	TextSpec OldFont;
	GetFont(&OldFont);
	
	Use();
	FontInfo Info;
	GetFontInfo(&Info);
	
	Ascent = Info.ascent;
	Descent = Info.descent;
	for (int k=0; k<256; k++)
		Widths[k] = CharWidth(k);
	
	SetFont(&OldFont);
	// End some MacOS-specific stuff
	
	// Put some padding around each glyph so as to avoid clipping it
	const short Pad = 1;
	Ascent += Pad;
	Descent += Pad;
	for (int k=0; k<256; k++)
		Widths[k] += 2*Pad;
	
	// Now for the totals and dimensions
	int TotalWidth = 0;
	for (int k=0; k<256; k++)
		TotalWidth += Widths[k];
	
	// For an empty font, clear out
	if (TotalWidth <= 0) return;
	
	int GlyphHeight = Ascent + Descent;
	
	int EstDim = int(sqrt(TotalWidth*GlyphHeight) + 0.5);
	TxtrWidth = MAX(128, NextPowerOfTwo(EstDim));
	
	// Find the character starting points and counts
	short CharStarts[256], CharCounts[256];
	int LastLine = 0;
	CharStarts[LastLine] = 0;
	CharCounts[LastLine] = 0;
	short Pos = 0;
	for (int k=0; k<256; k++)
	{
		// Over the edge? If so, then start a new line
		short NewPos = Pos + Widths[k];
		if (NewPos > TxtrWidth)
		{
			LastLine++;
			CharStarts[LastLine] = k;
			Pos = Widths[k];
			CharCounts[LastLine] = 1;
		} else {
			Pos = NewPos;
			CharCounts[LastLine]++;
		}
	}
	TxtrHeight = MAX(128, NextPowerOfTwo(GlyphHeight*(LastLine+1)));
	
	// MacOS-specific: create a grayscale color table if necessary
	/*
	if (!Grays)
	{
		CTabHandle SystemColors = GetCTable(8);
		int SCSize = GetHandleSize(Handle(SystemColors));
		Grays = (CTabHandle)NewHandle(SCSize);
		assert(Grays);
		HLock(Handle(Grays));
		HLock(Handle(SystemColors));
		CTabPtr GrayPtr = *Grays;
		CTabPtr SCPtr = *SystemColors;
		memcpy(GrayPtr,SCPtr,SCSize);
		for (int k=0; k<256; k++)
		{
			ColorSpec& Spec = GrayPtr->ctTable[k];
			Spec.rgb.red = Spec.rgb.green = Spec.rgb.blue = (k << 8) + k;
		}
		HUnlock(Handle(Grays));
		HUnlock(Handle(SystemColors));
		DisposeCTable(SystemColors);
	}
	*/
	
	// MacOS-specific: render the font glyphs onto a GWorld,
	// and use it as the source of the font texture.
	Rect ImgRect;
	SetRect(&ImgRect,0,0,TxtrWidth,TxtrHeight);
	GWorldPtr FTGW;
	OSErr Err = NewGWorld(&FTGW,32,&ImgRect,0,0,0);
	// OSErr Err = NewGWorld(&FTGW,8,&ImgRect,Grays,0,0);
	if (Err != noErr) return;
	PixMapHandle Pxls = GetGWorldPixMap(FTGW);
	LockPixels(Pxls);
	
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld(&OrigPort,&OrigDevice);
 	SetGWorld(FTGW,0);
	
	BackColor(blackColor);
	ForeColor(whiteColor);
 	EraseRect(&ImgRect);
 	
 	Use();	// The GWorld needs its font set for it
 	
 	for (int k=0; k<=LastLine; k++)
 	{
 		int Which = CharStarts[k];
 		int VPos = k*GlyphHeight + Ascent;
 		int HPos = Pad;
 		for (int m=0; m<CharCounts[k]; m++)
 		{
 			MoveTo(HPos,VPos);
 			DrawChar(Which);
 			HPos += Widths[Which++];
 		}
 	}
 	
 	// Non-MacOS-specific: allocate the texture buffer
 	// Its format is LA, where L is the luminosity and A is the alpha channel
 	// The font value will go into A.
 	OGL_Texture = new uint8[2*GetTxtrSize()];
	
	// Now copy from the GWorld into the OpenGL texture	
	uint8 *PixBase = (byte *)GetPixBaseAddr(Pxls);
 	int Stride = int((**Pxls).rowBytes & 0x7fff);
 	
 	for (int k=0; k<TxtrHeight; k++)
 	{
 		uint8 *SrcPxl = PixBase + k*Stride + 1;	// Use red channel
 		uint8 *DstPxl = OGL_Texture + 2*k*TxtrWidth;
 		for (int m=0; m<TxtrWidth; m++)
 		{
 			*(DstPxl++) = 0xff;	// Base color: white (will be modified with glColorxxx())
 			*(DstPxl++) = *SrcPxl;
 			SrcPxl += 4;
 			// *(DstPxl++) = *(SrcPxl++);
 		}
 	}
 	
 	UnlockPixels(Pxls);
 	SetGWorld(OrigPort,OrigDevice);
 	DisposeGWorld(FTGW);
 	// Absolute end of MacOS-specific stuff; OpenGL stuff starts here
 	
 	// Load texture
 	glGenTextures(1,&TxtrID);
 	glBindTexture(GL_TEXTURE_2D,TxtrID);
 	
 	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, TxtrWidth, TxtrHeight,
		0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, OGL_Texture);
 	
 	// Allocate and create display lists of rendering commands
 	DispList = glGenLists(256);
 	GLfloat TWidNorm = GLfloat(1)/TxtrWidth;
 	GLfloat THtNorm = GLfloat(1)/TxtrHeight;
 	for (int k=0; k<=LastLine; k++)
 	{
 		int Which = CharStarts[k];
 		GLfloat Top = k*(THtNorm*GlyphHeight);
 		GLfloat Bottom = (k+1)*(THtNorm*GlyphHeight);
 		int Pos = 0;
 		for (int m=0; m<CharCounts[k]; m++)
 		{
 			short Width = Widths[Which];
 			int NewPos = Pos + Width;
 			GLfloat Left = TWidNorm*Pos;
 			GLfloat Right = TWidNorm*NewPos;
 			
 			glNewList(DispList + Which, GL_COMPILE);
 			
 			// Draw the glyph rectangle
 			glBegin(GL_POLYGON);
 			
 			glTexCoord2f(Left,Top);
  			glVertex2s(0,-Ascent);
  			
 			glTexCoord2f(Right,Top);
  			glVertex2s(Width,-Ascent);
  			
 			glTexCoord2f(Right,Bottom);
 			glVertex2s(Width,Descent);
 			
 			glTexCoord2f(Left,Bottom);
			glVertex2s(0,Descent);
			
			glEnd();
			
			// Move to the next glyph's position
			glTranslatef(Width-2*Pad,0,0);
			
 			glEndList();
 			
 			// For next one
 			Pos = NewPos;
 			Which++;
 		}
 	}
}


// Renders a C-style string in OpenGL.
// assumes screen coordinates and that the left baseline point is at (0,0).
// Alters the modelview matrix so that the next characters will be drawn at the proper place.
// One can surround it with glPushMatrix() and glPopMatrix() to remember the original.
void FontSpecifier::OGL_Render(char *Text)
{
	// Bug out if no texture to render
	if (!OGL_Texture) return;
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	glBindTexture(GL_TEXTURE_2D,TxtrID);
	
	int Len = MIN(strlen(Text),255);
	for (int k=0; k<Len; k++)
	{
		unsigned char c = Text[k];
		glCallList(DispList+c);
	}
	
	glPopAttrib();
}
#endif

	
// Given a pointer to somewhere in a name set, returns the pointer
// to the start of the next name, or NULL if it did not find any
char *FontSpecifier::FindNextName(char *NamePtr)
{
	char *NextNamePtr = NamePtr;
	char c;
	
	// Continue as long as one's in the string
	while((c = *NextNamePtr) != 0)
	{
		// dprintf("Nxt %c",c);
		// Check for whitespace and commas and semicolons
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',' || c == ';')
		{
			NextNamePtr++;
			continue;
		}
		return NextNamePtr;
	}
	return NULL;
}

// Given a pointer to the beginning of a name, finds the pointer to the character
// just after the end of that name
char *FontSpecifier::FindNameEnd(char *NamePtr)
{
	char *NameEndPtr = NamePtr;
	char c;
	
	// Continue as long as one's in the string
	while((c = *NameEndPtr) != 0)
	{
		// dprintf("End %c",c);
		// Delimiter: comma or semicolon
		if (c == ',' || c == ';') break;
		NameEndPtr++;
	}
	return NameEndPtr;
}


// Font-parser object:
class XML_FontParser: public XML_ElementParser
{
	FontSpecifier TempFont;
	int Index;
	bool IsPresent[5];

public:
	FontSpecifier *FontList;
	int NumFonts;
	
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_FontParser(): XML_ElementParser("font"), FontList(NULL) {}
};

bool XML_FontParser::Start()
{
	for (int k=0; k<5; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_FontParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (NumFonts > 0)
	{
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,0,NumFonts-1))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	}
	if (strcmp(Tag,"name") == 0)
	{
		IsPresent[0] = true;
		strncpy(TempFont.NameSet,Value,FontSpecifier::NameSetLen);
		TempFont.NameSet[FontSpecifier::NameSetLen-1] = 0;	// For making the set always a C string
		return true;
	}
	else if (strcmp(Tag,"size") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempFont.Size))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"style") == 0)
	{
		float CVal;
		if (ReadNumericalValue(Value,"%hd",TempFont.Style))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"file") == 0)
	{
		IsPresent[4] = true;
		strncpy(TempFont.File,Value,FontSpecifier::NameSetLen);
		TempFont.File[FontSpecifier::NameSetLen-1] = 0;	// For making the set always a C string
		return true;
	}
	UnrecognizedTag();
	return false;
}

bool XML_FontParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	if (NumFonts <= 0)
	{
		IsPresent[3] = true;	// Convenient fakery: no index -- always present
		Index = 0;
	}	
	if (!IsPresent[3])
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place and update if necessary
	assert(FontList);
	bool DoUpdate = false;
	if (IsPresent[0])
	{
		strncpy(FontList[Index].NameSet,TempFont.NameSet,FontSpecifier::NameSetLen);
		DoUpdate = true;
	}
	if (IsPresent[1])
	{
		FontList[Index].Size = TempFont.Size;
		DoUpdate = true;
	}
	if (IsPresent[2])
	{
		FontList[Index].Style = TempFont.Style;
		DoUpdate = true;
	}
	if (IsPresent[4])
	{
		strncpy(FontList[Index].File,TempFont.File,FontSpecifier::NameSetLen);
		DoUpdate = true;
	}
	if (DoUpdate) FontList[Index].Update();
	return true;
}

static XML_FontParser FontParser;



// Returns a parser for the colors;
// several elements may have colors, so this ought to be callable several times.
XML_ElementParser *Font_GetParser() {return &FontParser;}

// This sets the array of colors to be read into.
// Its args are the pointer to that array and the number of colors in it.
// If that number is <= 0, then the color value is assumed to be non-indexed,
// and no "index" attribute will be searched for.
void Font_SetArray(FontSpecifier *FontList, int NumFonts)
{FontParser.FontList = FontList; FontParser.NumFonts = NumFonts;}
