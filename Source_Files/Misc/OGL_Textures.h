/*
	
	OpenGL Texture Manager,
	by Loren Petrich,
	March 12, 2000

	This contains functions for handling the texture management for OpenGL.
*/

#ifndef _OGL_TEXTURES
#define _OGL_TEXTURES

// Initialize the texture accounting
void OGL_StartTextures();

// Done with the texture accounting
void OGL_StopTextures();


// State of an individual texture set:
struct TextureState
{
	// Which member textures?
	enum
	{
		Normal,		// Used for all normally-shaded and shadeless textures
		Glowing,	// Used for self-luminous textures
		NUMBER_OF_TEXTURES
	};
	GLuint IDs[NUMBER_OF_TEXTURES];		// Texture ID's
	bool IsUsed;						// Is the texture set being used?
	bool IsGlowing;						// Does the texture have a glow map?
	bool IDsInUse[NUMBER_OF_TEXTURES];	// Which ID's are being used?
	
	TextureState() {IsUsed = IsGlowing = IDsInUse[Normal] = IDsInUse[Glowing] = false;}
	~TextureState() {if (IsUsed) glDeleteTextures(NUMBER_OF_TEXTURES,IDs);}
	
	// Allocate some textures and indicate whether an allocation had happened.
	bool Allocate();
	
	// These indicate that some texture
	bool Use(int Which);
	bool UseNormal() {return Use(Normal);}
	bool UseGlowing() {return Use(Glowing);}
	
	// Reset the texture to unused and force a reload if necessary
	void Reset();
};


/*
	This is the collected texture states per collection bitmap;
	it contains both texture-mapping info and the state for each color-table value.

	The GLdouble stuff is for setting up offsets of the texture coordinates, using:
	Texture Coordinate = (offset) + (scale) * (clip position)
	where (scale) = (sprite size) / (texture size)
	and (clip position) = ((clip screen position) - (left screen position)) /
		((right screen position) - (left screen position))	
*/
struct CollBitmapTextureState
{
	TextureState CTStates[NUMBER_OF_OPENGL_BITMAP_SETS];
	GLdouble U_Scale, V_Scale, U_Offset, V_Offset;
	
	// Sensible default
	CollBitmapTextureState() {U_Scale = V_Scale = 1; U_Offset = V_Offset = 0;}
};



// Intended to be the no-texture default argument
const int BadTextureType = 32767;

/*
	Texture-manager object: contains all the handling of textures;
	it finds their setting up, their dimensions, and so forth.
	
	It is designed to be created and destroyed
	once each rendering go-around with each texture.
*/

class TextureManager
{	
	// Width: along scanlines; height; from scanline to scanline
	// Wall textures and sprites are horizontal-vertical flipped

	// Various texture ID's:
	short Collection;
	short CTable;
	short Frame;
	short Bitmap;

	// Info transmitted from the setting-up phase
	int TextureType;
	bool IsGlowing;
			
	// Width and height and whether to do RLE
	// These are, in order, for the original texture, for the OpenGL texture,
	// and the offsets in the original in the OpenGL texture.
	// Also, the loaded-texture dimensions, in case the textures get shrunk.
	short BaseTxtrWidth, BaseTxtrHeight;
	short TxtrWidth, TxtrHeight;
	short WidthOffset, HeightOffset;
	short LoadedWidth, LoadedHeight;
	
	// Color table: need it for going from indexed to true color;
	// both normal and glow-mapped versions are needed
	GLuint NormalColorTable[MAXIMUM_SHADING_TABLE_INDEXES];
	GLuint GlowColorTable[MAXIMUM_SHADING_TABLE_INDEXES];
	
	// Texture buffers for OpenGL
	GLuint *NormalBuffer, *GlowBuffer;
	
	// Pointer to the appropriate texture-state object
	TextureState *TxtrStatePtr;
	
	// Pointer to texture-options object
	OGL_TextureOptions *TxtrOptsPtr;
		
	// Private methods
	
	// This one finds the width, height, etc. of a texture type;
	// it returns "false" if some texture's dimensions do not fit.
	bool SetupTextureGeometry();
	
	// This one finds the color tables
	void FindColorTables();
	
	// This one allocates an OpenGL texture buffer and uses a color table
	GLuint *GetOGLTexture(GLuint *ColorTable);
	
	// This one creates a fake landscape
	GLuint *GetFakeLandscape();
	
	// This is for shrinking a texture
	GLuint *Shrink(GLuint *Buffer);
	
	// This si for placing a texture in OpenGL
	void PlaceTexture(bool IsOverlaid, GLuint *Buffer);
	
public:

	// Inputs: get off of texture object passed to scottish_textures.
	shape_descriptor ShapeDesc;
	bitmap_definition *Texture;
	void *ShadingTables;
	short TransferMode;
	short TransferData;
	bool IsShadeless;
	
	// The width of a landscape texture will be 2^(-Landscape_AspRatExp) * (the height)
	short Landscape_AspRatExp;
	
	// Sets up all the texture stuff:	
	bool Setup(int TextureType0, int TextureType1=BadTextureType);
	
	// Results:
	
	// Various texture ID's:
	short GetCollection() {return Collection;}
	short GetCTable() {return CTable;}
	short GetFrame() {return Frame;}
	short GetBitmap() {return Bitmap;}

	// Info transmitted from the setting-up phase:
	// texture type, whether it is glowmapped,
	// and whether the textures are blended rather than all-or-nothing crisp-edged
	int GetTextureType() {return TextureType;}	
	bool IsGlowMapped() {return IsGlowing;}
	bool IsBlended() {return (TxtrOptsPtr->OpacityType != OGL_OpacType_Crisp);}
	bool VoidVisible() {return (TxtrOptsPtr->VoidVisible);}
	
	// Scaling and offset of the current texture;
	// important for sprites, which will be padded to make them OpenGL-friendly.
	GLdouble U_Scale, V_Scale, U_Offset, V_Offset;
	
	// What to render:
	// "IsOverlaid" is to be on if this one is to be done simultaneously,
	// using ARB_multitexture
	
	// Always call this one and call it first; safe to allocate texture ID's in it
	void RenderNormal();
	// Call this one after RenderNormal()
	void RenderGlowing(bool IsOverlaid);
	
	TextureManager();
	~TextureManager();
};


// Color-transformation macros:

// Five-to-eight translation:
// takes a 5-bit color value and expands it to 8 bytes
inline byte FiveToEight(byte x) {return (x << 3) | ((x >> 2) & 0x07);}

// 16-bit-to-32-bit with opacity = 1;
// ARGB 1555 to RGBA 8888
inline GLuint Convert_16to32(uint16 InPxl)
{
#ifdef LITTLE_ENDIAN
	// Alpha preset
	GLuint OutPxl = 0xff000000;
	GLuint Chan;
	// Red
	Chan = FiveToEight(InPxl >> 10);
	OutPxl |= Chan;
	// Green
	Chan = FiveToEight(InPxl >> 5);
	OutPxl |= Chan << 8;
	// Blue
	Chan = FiveToEight(InPxl);
	OutPxl |= Chan << 16;
#else
	// Alpha preset
	GLuint OutPxl = 0x000000ff;
	GLuint Chan;
	// Red
	Chan = FiveToEight(InPxl >> 10);
	OutPxl |= Chan << 24;
	// Green
	Chan = FiveToEight(InPxl >> 5);
	OutPxl |= Chan << 16;
	// Blue
	Chan = FiveToEight(InPxl);
	OutPxl |= Chan << 8;
#endif
	
	return OutPxl;
}


// Make floating-point colors
inline void MakeFloatColor(RGBColor& InColor, GLfloat *OutColor)
{
	OutColor[0] = InColor.red/65535.0;
	OutColor[1] = InColor.green/65535.0;
	OutColor[2] = InColor.blue/65535.0;
}

inline void MakeFloatColor(rgb_color& InColor, GLfloat *OutColor)
{
	OutColor[0] = InColor.red/65535.0;
	OutColor[1] = InColor.green/65535.0;
	OutColor[2] = InColor.blue/65535.0;
}

/*
// Stuff for doing 16->32 pixel-format conversion, 1555 ARGB to 8888 RGBA
// The table is created only when necessary, and retained as long as it is necessary
extern GLuint *ConversionTable_16to32;

extern void MakeConversion_16to32(int BitDepth);
*/

// Infravision (I'm blue, are you?)
bool& IsInfravisionActive();

// Sets the infravision tinting color for a shapes collection, and whether to use such tinting;
// the color values are from 0 to 1.
bool SetInfravisionTint(short Collection, bool IsTinted, float Red, float Green, float Blue);

// Finds the infravision version of a color for some collection set;
// it makes no change if infravision is inactive.
void FindInfravisionVersion(short Collection, GLfloat *Color);


#endif