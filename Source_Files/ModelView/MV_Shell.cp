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

	Model-Viewer Test Shell, by Loren Petrich, June 10, 2001
	This uses GLUT to create an OpenGL context for displaying a 3D model,
	in order to test model-reading code
*/

#include <math.h>
#include <string.h>
#include <ctype.h>
#if defined (__APPLE__) && defined (__MACH__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <OpenGL/glut.h>
# include <Movies.h>
#elif defined mac
# include <gl.h>
# include <glu.h>
# include <glut.h>
# include <Movies.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif
#include "cseries.h"
#include "FileHandler.h"
#include "ImageLoader.h"
#include "ModelRenderer.h"
#include "Dim3_Loader.h"
#include "QD3D_Loader.h"
#include "StudioLoader.h"
#include "WavefrontLoader.h"


// Test for Macintosh-specific stuff: Quicktime (for loading images)
// and Navigation Services (fancy dialog boxes)

bool HasQuicktime = false;
bool HasNavServices = false;
void InitMacServices()
{
	// Cribbed from shell_macintosh.cpp
	long response;
	OSErr error;

	error= Gestalt(gestaltQuickTime, &response);
	if(!error) 
	{
		/* No error finding QuickTime.  Check for ICM so we can use Standard Preview */
		error= Gestalt(gestaltCompressionMgr, &response);
		if(!error) 
		{
			// Now initialize Quicktime's Movie Toolbox,
			// since the soundtrack player will need it
			error = EnterMovies();
			if (!error) {
				HasQuicktime = true;
			}
		}
	}
	
	if ((void*)NavLoad != (void*)kUnresolvedCFragSymbolAddress)
	{
		NavLoad();
		HasNavServices = true;
	}
}

bool machine_has_quicktime() 
{
	return HasQuicktime;
}

bool machine_has_nav_services()
{
	return HasNavServices;
}

void global_idle_proc() {}	// Do nothing

// GLUT window ID in case there is more than one GLUT window;
// however, Apple GLUT does not like more than one such window,
// so this program will be one-window-only for now.
int MainWindowID;

void ResizeMainWindow(int _Width, int _Height);


// Model and skin objects;
// a model is saved in case it is edited, as when vertices are split
Model3D Model, SavedModel;
ImageDescriptor Image;
bool ImageSemitransparent = false;
ModelRenderer Renderer;
ModelRenderShader Shaders[2];

// For loading models and skins
enum {
	ModelWavefront = 1,
	ModelStudio,
	Model_QD3D,
	Model_Dim3_First,
	Model_Dim3_Rest
};
const int LoadSkinItem = 1;

// When the model or the skin is changed, then do this
void InvalidateTexture();

void LoadModelAction(int ModelType)
{
	// Get the model file
	FileSpecifier File;
	bool Success = false;
	
	// Specialize the dialog for what kind of model file
	switch(ModelType)
	{
	case ModelWavefront:
		set_typecode(_typecode_patch,'TEXT');
		if (File.ReadDialog(_typecode_patch,"Model Type: Alias|Wavefront"))
			Success = LoadModel_Wavefront(File, Model);
		break;
	case ModelStudio:
		// No canonical MacOS assignment of this model type
		if (File.ReadDialog(_typecode_unknown,"Model Type: 3D Studio Max"))
			Success = LoadModel_Studio(File, Model);
		break;
	case Model_QD3D:
		set_typecode(_typecode_patch,'3DMF');
		if (File.ReadDialog(_typecode_patch,"Model Type: QuickDraw 3D"))
			Success = LoadModel_QD3D(File, Model);
		break;
	case Model_Dim3_First:
		set_typecode(_typecode_patch,'TEXT');
		if (File.ReadDialog(_typecode_patch,"Model Type: Dim3 (First Part)"))
			Success = LoadModel_Dim3(File, Model, LoadModelDim3_First);
		break;
	case Model_Dim3_Rest:
		set_typecode(_typecode_patch,'TEXT');
		if (File.ReadDialog(_typecode_patch,"Model Type: Dim3 (Later Parts)"))
			Success = LoadModel_Dim3(File, Model,LoadModelDim3_Rest);
		break;
	}
	
	if (!Success) Model.Clear();
	
	glutSetWindow(MainWindowID);
	char Name[256];
	File.GetName(Name);
	glutSetWindowTitle(Name);
	
	// Neutral positions will always be initially present.
	// Assume that a boned model already has a bounding box;
	// most boneless (static) models do not have such bounding boxes.
	if (Model.Bones.empty())
		Model.FindBoundingBox();
	
	// Just in case they are off...
	Model.NormalizeNormals();
	
	ResizeMainWindow(glutGet(GLUT_WINDOW_WIDTH),glutGet(GLUT_WINDOW_HEIGHT));
	
	// Dump stats
	printf("Vertex Positions: %d\n",Model.Positions.size()/3);
	printf("Txtr Coords: %d\n",Model.TxtrCoords.size()/2);
	printf("Normals: %d\n",Model.Normals.size()/3);
	printf("Colors: %d\n",Model.Colors.size()/3);
	printf("Triangles: %d\n",Model.VertIndices.size()/3);
	printf("Bounding Box\n");
	for (int c=0; c<3; c++)
		printf("%9f %9f\n",Model.BoundingBox[0][c],Model.BoundingBox[1][c]);
	
	SavedModel = Model;
	
	// Invalidate the current texture, so it will be reloaded for the model
	InvalidateTexture();
}

// Z-Buffering
bool Use_Z_Buffer = true;

// Bounding box?
bool Show_Bounding_Box = false;

// Texture management
bool TxtrIsPresent = false;
GLuint TxtrID;

// External lighting?
bool Use_Light = false;

// Model overall transform?
bool Use_Model_Transform = false;

// How many rendering passes
int NumRenderPasses = 1;

// What to render: neutral, frames, or sequences? And which of each?
enum {
	ShowAnim_Neutral,
	ShowAnim_Frames,
	ShowAnim_Sequences
} ShowAnim = ShowAnim_Neutral;

int ThisFrame = 0;
int ThisSeq = 0;

// Also the mixture between the current and next frames
float MixFrac = 0;


void InvalidateTexture()
{
	if (TxtrIsPresent)
	{
		glDeleteTextures(1,&TxtrID);
		TxtrIsPresent = false;
	}
}


void LoadSkinAction(int SkinType)
{
	// Get the skin file
	FileSpecifier File;
	
	bool Success = false;
	if (File.ReadDialog(_typecode_unknown,"Skin Image"))
		Success = LoadImageFromFile(Image,File,SkinType);
	if (!Success) return;
	
	const int BufferSize = 256;
	char Buffer[BufferSize];
	File.GetName(Buffer);
	printf("Read skin file %s\n",Buffer);
	switch(SkinType)
	{
		case ImageLoader_Colors:
		ImageSemitransparent = false;	// Initially, no nonzero transparency
		printf("For colors\n");
		break;
		
		case ImageLoader_Opacity:
		ImageSemitransparent = true;	// Nonzero transparency
		printf("For opacity\n");
		break;
	}
	
	// Invalidate the current texture
	InvalidateTexture();
	
	glutPostRedisplay();
}


float VertexSplitThreshold = 0;


void NormalTypeAction(int NormAction)
{
	Model = SavedModel;
	Model.AdjustNormals(NormAction,VertexSplitThreshold);
	glutPostRedisplay();
}


void ShaderCallback(void *Data)
{
	(void)(Data);
	if (!Image.IsPresent()) return;
	
	// Setting up the textures each time
	if (TxtrIsPresent)
		glBindTexture(GL_TEXTURE_2D,TxtrID);
	else
	{
		// Load the texture
		glGenTextures(1,&TxtrID);
		glBindTexture(GL_TEXTURE_2D,TxtrID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8,
			Image.GetWidth(), Image.GetHeight(),
			GL_RGBA, GL_UNSIGNED_BYTE,
			Image.GetPixelBasePtr());
		TxtrIsPresent = true;
	}
}

	
// Needed for external lighting;
// the first index is the color channel,
// while the second index is in two parts,
// the first three (0-2) multipled by the normal vector in dot-product fasion
// and the fourth (3) added

GLfloat ExternalLight[3][4];


void LightingCallback(void *Data, unsigned long NumVerts, GLfloat *Normals, GLfloat *Positions, GLfloat *Colors)
{
	(void)(Data);
	
// Attempting to keep CB's code on hand
#if 0

	// Register usage:
	// mm0/mm1: ExternalLight[0] (4 components)
	// mm2/mm3: ExternalLight[1] (4 components)
	// mm4/mm5: ExternalLight[2] (4 components)
	// mm6:     Normal[0]/Normal[1]
	// mm7:     Normal[2]/1.0

	GLfloat tmp[2] = {0.0, 1.0};
	__asm__ __volatile__("
			femms\n
			movq	0x00(%3),%%mm0\n
			movq	0x08(%3),%%mm1\n
			movq	0x10(%3),%%mm2\n
			movq	0x18(%3),%%mm3\n
			movq	0x20(%3),%%mm4\n
			movq	0x28(%3),%%mm5\n
			movl	8(%0),%%eax\n
			movl	%%eax,(%4)\n
		0:\n
			prefetch 192(%0)\n
			movq	(%0),%%mm6\n
			movq	(%4),%%mm7\n
			pfmul	%%mm0,%%mm6\n
			pfmul	%%mm1,%%mm7\n
			pfacc	%%mm6,%%mm7\n
			pfacc	%%mm6,%%mm7\n
			movd	%%mm7,(%1)\n
			movq	(%0),%%mm6\n
			movq	(%4),%%mm7\n
			pfmul	%%mm2,%%mm6\n
			pfmul	%%mm3,%%mm7\n
			pfacc	%%mm6,%%mm7\n
			pfacc	%%mm6,%%mm7\n
			movd	%%mm7,4(%1)\n
			movq	(%0),%%mm6\n
			movq	(%4),%%mm7\n
			pfmul	%%mm4,%%mm6\n
			pfmul	%%mm5,%%mm7\n
			pfacc	%%mm6,%%mm7\n
			pfacc	%%mm6,%%mm7\n
			movd	%%mm7,8(%1)\n
		\n
			add		$0x0c,%0\n
			movl	8(%0),%%eax\n
			movl	%%eax,(%4)\n
			add		$0x0c,%1\n
			dec		%2\n
			jne		0b\n
			femms\n"
		:
		: "g" (Normals), "g" (ColorPtr), "g" (NumVerts), "g" (ExternalLight), "g" (&tmp)
		: "eax", "memory"
	);

#else

	GLfloat *el0 = ExternalLight[0], *el1 = ExternalLight[1], *el2 = ExternalLight[2];
	for (int k=0; k<NumVerts; k++)
	{
		GLfloat N0 = *(Normals++);
		GLfloat N1 = *(Normals++);
		GLfloat N2 = *(Normals++);
		*(Colors++) =
			el0[0]*N0 + 
			el0[1]*N1 + 
			el0[2]*N2 + 
			el0[3];
		*(Colors++) =
			el1[0]*N0 + 
			el1[1]*N1 + 
			el1[2]*N2 + 
			el1[3];
		*(Colors++) =
			el2[0]*N0 + 
			el2[1]*N1 + 
			el2[2]*N2 + 
			el2[3];
	}

#endif
}


// Callback for drawing that window
void DrawMainWindow()
{
	// No smearing allowed!
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Render the model
	if (!Model.Positions.empty())
	{
		// In the absence of a skin...
		const GLfloat FalseColors[12][3] =
		{
			{  1,   0,   0},
			{  1, 0.5,   0},
			{  1,   1,   0},
			
			{0.5,   1,   0},
			{  0,   1,   0},
			{  0,   1, 0.5},
			
			{  0,   1,   1},
			{  0, 0.5,   1},
			{  0,   0,   1},
			
			{0.5,   0,   1},
			{  1,   0,   1},
			{  1,   0, 0.5}
		};
		
		// Replace the colors if there isn't an already-present color array
		int NumColors = Model.Positions.size()/3;
		if (Model.Colors.size() != 3*NumColors)
		{
			Model.Colors.resize(3*NumColors);
			for (int k=0; k<NumColors; k++)
			{
				const GLfloat *FalseColor = FalseColors[k%12];
				GLfloat *Color = &Model.Colors[3*k];
				for (int c=0; c<3; c++)
					Color[c] = FalseColor[c];
			}
		}
				
		// Extract the view direction from the matrix
		// Since it is pure rotation, transpose = inverse
		GLfloat ModelViewMatrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrix);
		for (int k=0; k<3; k++)
			Renderer.ViewDirection[k] = - ModelViewMatrix[4*k + 2];
		
		// Compose a set of external lights:
		for (int c=0; c<3; c++)
		{
			for (int k=0; k<3; k++)
				ExternalLight[c][k] = 0.5*ModelViewMatrix[4*k + c];
			ExternalLight[c][3] = 0;
		}
		
		if (Use_Z_Buffer)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		// glEnable(GL_CULL_FACE);
		// glCullFace(GL_BACK);
		// glFrontFace(GL_CW);
		
		int NextFrame = 0;
		switch(ShowAnim)
		{
		case ShowAnim_Neutral:
			Model.FindPositions_Neutral(Use_Model_Transform);
			break;
		
		case ShowAnim_Frames:
			if (MixFrac != 0)
			{
				if ((NextFrame = ThisFrame + 1) >= Model.TrueNumFrames())
					NextFrame = 0;
			}
			Model.FindPositions_Frame(Use_Model_Transform,ThisFrame,MixFrac,NextFrame);
			break;
		
		case ShowAnim_Sequences:
			if (MixFrac != 0)
			{
				if ((NextFrame = ThisFrame + 1) >= Model.NumSeqFrames(ThisSeq))
					NextFrame = 0;
			}
			Model.FindPositions_Sequence(Use_Model_Transform,ThisSeq,ThisFrame,MixFrac,NextFrame);
			break;
		}
		
		// Draw the bounding box
		const GLfloat EdgeColor[3] = {1,1,0};
		const GLfloat DiagColor[3] = {0,1,1};
		if (Show_Bounding_Box) Model.RenderBoundingBox(EdgeColor,DiagColor);
		glColor3f(1,1,1);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		unsigned int Flags = 0;
		SET_FLAG(Flags,ModelRenderer::Textured,TxtrIsPresent);
		SET_FLAG(Flags,ModelRenderer::Colored,!TxtrIsPresent);
		SET_FLAG(Flags,ModelRenderer::ExtLight,Use_Light);
		Shaders[1].Flags = Shaders[0].Flags = Flags;
		int NumOpaqueRenderPasses = ImageSemitransparent ? NumRenderPasses : 0;
		Renderer.Render(Model,Shaders,NumRenderPasses,NumOpaqueRenderPasses,Use_Z_Buffer);
	}
	
	// All done -- ready to show	
	glutSwapBuffers();
}


void ResizeMainWindow(int _Width, int _Height)
{
	if (!Model.Positions.empty())
	{
		// For getting the aspect ratio straight
		int MinDim = MAX(MIN(_Width,_Height),1);
		
		// Bounding-box "radius"	
		GLfloat BBRadius = 0;
		for (int m=0; m<3; m++)
		{

			GLfloat Dist = MAX(-Model.BoundingBox[0][m],Model.BoundingBox[1][m]);
			BBRadius += Dist*Dist;
		}
		BBRadius = sqrt(BBRadius);
		
		GLfloat XDim = BBRadius*_Width/MinDim;
		GLfloat YDim = BBRadius*_Height/MinDim;
		GLfloat ZDim = 2*BBRadius;
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-XDim,XDim,-YDim,YDim,-ZDim,ZDim);
		glutPostRedisplay();
		// glTranslatef(0.6*BBRadius,0.3*BBRadius,0);
	}
}

// Perform a "view side" rotate of the modelview matrix;
// rotations are normally applied to the matrix's "model side"
static void ViewSideRotate(GLfloat Angle, GLfloat X, GLfloat Y, GLfloat Z)
{
	GLfloat Matrix[16];
	glMatrixMode(GL_MODELVIEW);
	glGetFloatv(GL_MODELVIEW_MATRIX,Matrix);
	glLoadIdentity();
	glRotatef(Angle,X,Y,Z);
	glMultMatrixf(Matrix);
}

// Convenient standard value: 1/4 of a right angel
const GLfloat RotationAngle = 22.5;

const char ESCAPE = 0x1b;

void KeyInMainWindow(unsigned char key, int x, int y)
{
	
	key = toupper(key);
	
	switch(key)
	{
		case '6':
			ViewSideRotate(RotationAngle,0,1,0);
			glutPostRedisplay();
			break;
		
		case '4':
			ViewSideRotate(-RotationAngle,0,1,0);
			glutPostRedisplay();
			break;
		
		case '8':
			ViewSideRotate(RotationAngle,1,0,0);
			glutPostRedisplay();
			break;
		
		case '2':
			ViewSideRotate(-RotationAngle,1,0,0);
			glutPostRedisplay();
			break;
		
		case '9':
			ViewSideRotate(RotationAngle,0,0,-1);
			glutPostRedisplay();
			break;
		
		case '7':
			ViewSideRotate(-RotationAngle,0,0,-1);
			glutPostRedisplay();
			break;
		
		case '0':
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glutPostRedisplay();
			break;
		
		case 'Z':
			Use_Z_Buffer = !Use_Z_Buffer;
			if (Use_Z_Buffer)
				printf("Z-Buffer On\n");
			else
				printf("Z-Buffer Off\n");
			glutPostRedisplay();
			break;
		
		case 'B':
			Show_Bounding_Box = !Show_Bounding_Box;
			if (Show_Bounding_Box)
				printf("Bounding Box On\n");
			else
				printf("Bounding Box Off\n");
			glutPostRedisplay();
			break;
		
		case 'S':
			if (NumRenderPasses == 1)
				NumRenderPasses = 2;
			else
				NumRenderPasses = 1;
			printf("Render passes: %d\n",NumRenderPasses);
			glutPostRedisplay();
			break;
		
		case 'L':
			Use_Light = !Use_Light;
			if (Use_Light)
				printf("Light On\n");
			else
				printf("Light Off\n");
			glutPostRedisplay();
			break;
		
		case 'T':
			Use_Model_Transform = !Use_Model_Transform;
			if (Use_Model_Transform)
				printf("Model Transform On\n");
			else
				printf("Model Transform Off\n");
			glutPostRedisplay();
			break;
		
		case 'P':
			switch(ShowAnim)
			{
			case ShowAnim_Neutral:
				ShowAnim = ShowAnim_Frames;
				if (Model.Frames.empty())
					ShowAnim = ShowAnim_Neutral;
				break;
				
			case ShowAnim_Frames:
				ShowAnim = ShowAnim_Sequences;
				if (Model.Frames.empty())
					ShowAnim = ShowAnim_Neutral;
				else if (Model.SeqFrames.empty())
					ShowAnim = ShowAnim_Neutral;
				break;
				
			case ShowAnim_Sequences:
				ShowAnim = ShowAnim_Neutral;
				break;
			}
			
			switch(ShowAnim)
			{
			case ShowAnim_Neutral:
				printf("Neutral\n");
				break;
			case ShowAnim_Frames:
				ThisFrame = 0;
				printf("Frame %d\n",ThisFrame);
				break;
			case ShowAnim_Sequences:
				ThisSeq = 0;
				ThisFrame = 0;
				printf("Sequence %d %d\n",ThisSeq,ThisFrame);
				break;
			}
			
			glutPostRedisplay();
			break;
		
		case 'M':
			if ((MixFrac += 0.25) > 1) MixFrac = 0;
			break;
		
		case '[':
		case '{':
			if (--ThisSeq < 0)
				ThisSeq = Model.TrueNumSeqs() - 1;
			ThisFrame = 0;
			printf("Sequence %d %d\n",ThisSeq,ThisFrame);
			glutPostRedisplay();
			break;
		
		case ']':
		case '}':
			if (++ThisSeq >= Model.TrueNumSeqs())
				ThisSeq = 0;
			ThisFrame = 0;
			printf("Sequence %d %d\n",ThisSeq,ThisFrame);
			glutPostRedisplay();
			break;
		
		case '-':
		case '_':
			switch(ShowAnim)
			{
			case ShowAnim_Frames:
				if (--ThisFrame < 0)
					ThisFrame = Model.TrueNumFrames()-1;
				printf("Frame %d\n",ThisFrame);
				break;
			
			case ShowAnim_Sequences:
				if (--ThisFrame < 0)
					ThisFrame = Model.NumSeqFrames(ThisSeq)-1;
				printf("Sequence %d %d\n",ThisSeq,ThisFrame);
				break;
			}
			
			glutPostRedisplay();
			break;
		
		case '=':
		case '+':
			switch(ShowAnim)
			{
			case ShowAnim_Frames:
				if (++ThisFrame >= Model.TrueNumFrames())
					ThisFrame = 0;
				printf("Frame %d\n",ThisFrame);
				break;
			
			case ShowAnim_Sequences:
				if (++ThisFrame >= Model.NumSeqFrames(ThisSeq))
					ThisFrame = 0;
				printf("Sequence %d %d\n",ThisSeq,ThisFrame);
				break;
			}
			
			glutPostRedisplay();
			break;
	}
}

void SpecialInMainWindow(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_RIGHT:
			ViewSideRotate(RotationAngle,0,1,0);
			glutPostRedisplay();
			break;
		
		case GLUT_KEY_LEFT:
			ViewSideRotate(-RotationAngle,0,1,0);
			glutPostRedisplay();
			break;
		
		case GLUT_KEY_UP:
			ViewSideRotate(RotationAngle,1,0,0);
			glutPostRedisplay();
			break;
		
		case GLUT_KEY_DOWN:
			ViewSideRotate(-RotationAngle,1,0,0);
			glutPostRedisplay();
			break;
	}
}


// Create checked/unchecked menu item
char *CheckedText = "*";
char *UncheckedText = "_";

void DoMenuItemChecking(int MenuItem, char *ItemText, int Selector, int Value) {
	char TextBuffer[256];
	if (Selector == Value)
		strcpy(TextBuffer,CheckedText);
	else
		strcpy(TextBuffer,UncheckedText);
	strcat(TextBuffer," ");
	strcat(TextBuffer,ItemText);
	glutChangeToMenuEntry(MenuItem,TextBuffer,Value);
}

// Separate from NormalTypeAction to be below the checking/unchecking mechanism

enum
{
	VS_00,
	VS_01,
	VS_03,
	VS_10,
	VS_30
};

int VertexSplitThresholdIndex = VS_00;

int VertexSplitMenu;

void UpdateVertexSplitMenu()
{
	glutSetMenu(VertexSplitMenu);
	int Indx = 1;
	DoMenuItemChecking(Indx++,"0",VertexSplitThresholdIndex,VS_00);
	DoMenuItemChecking(Indx++,"0.1",VertexSplitThresholdIndex,VS_01);
	DoMenuItemChecking(Indx++,"0.3",VertexSplitThresholdIndex,VS_03);
	DoMenuItemChecking(Indx++,"1.0",VertexSplitThresholdIndex,VS_10);
	DoMenuItemChecking(Indx++,"3.0",VertexSplitThresholdIndex,VS_30);
	
	switch(VertexSplitThresholdIndex)
	{
	case VS_00:
		VertexSplitThreshold = 0.0;
		break;
	case VS_01:
		VertexSplitThreshold = 0.1;
		break;
	case VS_03:
		VertexSplitThreshold = 0.3;
		break;
	case VS_10:
		VertexSplitThreshold = 1.0;
		break;
	case VS_30:
		VertexSplitThreshold = 3.0;
		break;
	}
}


void VertexSplitAction(int c)
{
	VertexSplitThresholdIndex = c;
	UpdateVertexSplitMenu();
}



// Background-color stuff: designed for use with GLUT color picker
int RedIndx = 0, GreenIndx = 0, BlueIndx = 0;
const int CIndxRange = 5;
const float CVals[CIndxRange] = {0, 0.25, 0.5, 0.75, 1};

void SetBackgroundColor()
{
	// static GLfloat Red = CVals[RedIndx];
	// static GLfloat Green = CVals[GreenIndx];
	// static GLfloat Blue = CVals[BlueIndx];
	// static GLfloat Alpha = 1;
	// glClearColor(Red,Green,Blue,Alpha);
	// glClearColor(CVals[RedIndx],CVals[GreenIndx],CVals[BlueIndx],1);
}

// Update the color-selection menus

void UpdateColorMenu(int ColorIndx) {
	int Indx = 1;
	DoMenuItemChecking(Indx++,"0",ColorIndx,0);
	DoMenuItemChecking(Indx++,"0.25",ColorIndx,1);
	DoMenuItemChecking(Indx++,"0.5",ColorIndx,2);
	DoMenuItemChecking(Indx++,"0.75",ColorIndx,3);
	DoMenuItemChecking(Indx++,"1",ColorIndx,4);
}

int RedMenu;

void UpdateRedMenu() {
	glutSetMenu(RedMenu);
	UpdateColorMenu(RedIndx);
}

int GreenMenu;

void UpdateGreenMenu() {
	glutSetMenu(GreenMenu);
	UpdateColorMenu(GreenIndx);
}

int BlueMenu;

void UpdateBlueMenu() {
	glutSetMenu(BlueMenu);
	UpdateColorMenu(BlueIndx);
}

// Sets the window's background color

void SetRedSelection(int c) {
	RedIndx = c;
	UpdateRedMenu();
	SetBackgroundColor();
	glutPostRedisplay();
}

void SetGreenSelection(int c) {
	GreenIndx = c;
	UpdateGreenMenu();
	SetBackgroundColor();
	glutPostRedisplay();
}

void SetBlueSelection(int c) {
	BlueIndx = c;
	UpdateBlueMenu();
	SetBackgroundColor();
	glutPostRedisplay();
}

void SetColorSelection(int c) {
	// Nothing
	(void)(c);
}

// Top-level action function

void RightButtonAction(int c) {}

// Preset display-window side:
const int MainWindowWidth = 512, MainWindowHeight = 512;

int main(int argc, char **argv)
{
	// Setting up for using frames and sequences
	Model3D::BuildTrigTables();

	// For debugging of use of overall model transforms: inverts the model	
	for (int k=0; k<3; k++)
		Model.TransformPos.M[k][k] = Model.TransformNorm.M[k][k] = -1;
	
	// Must be up here
	glutInit(&argc, argv);
	
	// See if QT and NavSvcs (MacOS) are present
	InitMacServices();
	
	// Direct debug output to the console:
	SetDebugOutput_Wavefront(stdout);
	SetDebugOutput_Studio(stdout);
	SetDebugOutput_QD3D(stdout);
	SetDebugOutput_Dim3(stdout);

	// Set up shader object
	Shaders[0].Flags = 0;
	Shaders[0].TextureCallback = ShaderCallback;
	Shaders[0].LightingCallback = LightingCallback;
	Shaders[1].Flags = 0;
	Shaders[1].TextureCallback = ShaderCallback;
	Shaders[1].LightingCallback = LightingCallback;
	
	// Set up for creating the main window
	glutInitWindowSize(MainWindowWidth,MainWindowHeight);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	SetBackgroundColor();
	
	// Actually create that window
	MainWindowID = glutCreateWindow("Pfhormz");
	
	// Set its main callbacks
	glutDisplayFunc(DrawMainWindow);
	glutReshapeFunc(ResizeMainWindow);
	glutKeyboardFunc(KeyInMainWindow);
	glutSpecialFunc(SpecialInMainWindow);
	
	// Create model and skin menu items
	int ModelMenu = glutCreateMenu(LoadModelAction);
	glutAddMenuEntry("Alias-Wavefront...",ModelWavefront);
	glutAddMenuEntry("3D Studio Max...",ModelStudio);
	glutAddMenuEntry("QuickDraw 3D...",Model_QD3D);
	glutAddMenuEntry("Dim3 [First]...",Model_Dim3_First);
	glutAddMenuEntry("Dim3 [Rest]...",Model_Dim3_Rest);

	int SkinMenu = glutCreateMenu(LoadSkinAction);
	glutAddMenuEntry("Colors...",ImageLoader_Colors);
	glutAddMenuEntry("Opacity...",ImageLoader_Opacity);
	
	// Create normal-modification menu items
	int NormalTypeMenu = glutCreateMenu(NormalTypeAction);
	glutAddMenuEntry("None",Model3D::None);
	glutAddMenuEntry("Originals",Model3D::Original);
	glutAddMenuEntry("Reversed Originals",Model3D::Reversed);
	glutAddMenuEntry("Clockwise Sides",Model3D::ClockwiseSide);
	glutAddMenuEntry("Counterclockwise Sides",Model3D::CounterclockwiseSide);
	
	// Create a vertex-splitting-threshold menu item
	VertexSplitMenu = glutCreateMenu(VertexSplitAction);
	glutAddMenuEntry("0.0",VS_00);
	glutAddMenuEntry("0.1",VS_01);
	glutAddMenuEntry("0.3",VS_03);
	glutAddMenuEntry("1.0",VS_10);
	glutAddMenuEntry("3.0",VS_30);
	UpdateVertexSplitMenu();
	
	// Create a GLUT Color Picker
	// LP: suppressed because glClearColor() is broken
	#ifdef USE_BACKGROUND_COLOR
	// Create color-component submenus
	RedMenu = glutCreateMenu(SetRedSelection);
	glutAddMenuEntry("0",0);
	glutAddMenuEntry("0.25",1);
	glutAddMenuEntry("0.5",2);
	glutAddMenuEntry("0.75",3);
	glutAddMenuEntry("1",4);
	UpdateRedMenu();

	GreenMenu = glutCreateMenu(SetGreenSelection);
	glutAddMenuEntry("0",0);
	glutAddMenuEntry("0.25",1);
	glutAddMenuEntry("0.5",2);
	glutAddMenuEntry("0.75",3);
	glutAddMenuEntry("1",4);
	UpdateGreenMenu();

	BlueMenu = glutCreateMenu(SetBlueSelection);
	glutAddMenuEntry("0",0);
	glutAddMenuEntry("0.25",1);
	glutAddMenuEntry("0.5",2);
	glutAddMenuEntry("0.75",3);
	glutAddMenuEntry("1",4);
	UpdateBlueMenu();

	// Create background-color submenu
	int ColorMenu = glutCreateMenu(SetColorSelection);
	glutAddSubMenu("Red", RedMenu);
	glutAddSubMenu("Green", GreenMenu);
	glutAddSubMenu("Blue", BlueMenu);
	#endif
	
	// Create right-button menu:
	int RightButtonMenu = glutCreateMenu(RightButtonAction);
	glutAddSubMenu("Load Model...", ModelMenu);
	glutAddSubMenu("Load Skin...", SkinMenu);
	glutAddSubMenu("Normal Type", NormalTypeMenu);
	glutAddSubMenu("Vertex-Split Threshold", VertexSplitMenu);
	#ifdef USE_BACKGROUND_COLOR
	glutAddSubMenu("Background Color", ColorMenu);
	#endif
  	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	
	return 0;
}
