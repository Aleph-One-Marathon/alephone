#ifndef _OGL_FADERS_
#define _OGL_FADERS_
/*
	
	OpenGL Renderer,
	by Loren Petrich,
	May 30, 2000

	This contains code for doing fader stuff.	
*/


// Indicates whether OpenGL-rendering faders will be used
bool OGL_FaderActive();

// Which kinds of faders in the fader queue?
enum
{
	FaderQueue_Liquid,
	FaderQueue_Other,
	NUMBER_OF_FADER_QUEUE_ENTRIES
};

// Fader data
struct OGL_Fader
{
	// Which type of fade to do
	short Type;
	// The three color channels and a transparency channel
	float Color[4];
	
	OGL_Fader(): Type(NONE) {}
};

// Fader=queue accessor
OGL_Fader *GetOGL_FaderQueueEntry(int Index);

// Fader renderer; returns whether or not OpenGL faders were active.
bool OGL_DoFades(float Left, float Top, float Right, float Bottom);

#endif
