// Model-Viewer Test Shell, by Loren Petrich, June 10, 2001
// This uses GLUT to create an OpenGL context for displaying a 3D model,
// in order to test model-reading code


#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>


// In case there is more than one GLUT window;
// however, Apple GLUT does not like more than one such window.
int MainWindowID;

// Callback for drawing that window
void DrawMainWindow()
{
	// No smearing allowed!
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// All done -- ready to show	
	glutSwapBuffers();
}

void ResizeMainWindow(int _Width, int _Height)
{

}

const char ESCAPE = 0x1b;

void KeyInMainWindow(unsigned char key, int x, int y)
{

}

void SpecialInMainWindow(int key, int x, int y)
{

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

// Background-color stuff: designed for use with GLUT color picker
int RedIndx = 0, GreenIndx = 0, BlueIndx = 0;
const int CIndxRange = 5;
const float CVals[CIndxRange] = {0, 0.25, 0.5, 0.75, 1};

void SetBackgroundColor()
{
	glClearColor(CVals[RedIndx],CVals[GreenIndx],CVals[BlueIndx],1);
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

const int OpenMenuItem = 1;
void RightButtonAction(int c) {
	// if (c == OpenMenuItem) Load();
}


// Preset display-window side:
const int MainWindowWidth = 512, MainWindowHeight = 512;

int main(int argc, char **argv)
{
	// Must be up here
	glutInit(&argc, argv);
	
	// Set up for creating the main window
	glutInitWindowSize(MainWindowWidth,MainWindowHeight);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	SetBackgroundColor();
	
	// Actually create that window
	MainWindowID = glutCreateWindow("Aleph One Model Viewer");
	
	// Set its main callbacks
	glutDisplayFunc(DrawMainWindow);
	glutReshapeFunc(ResizeMainWindow);
	glutKeyboardFunc(KeyInMainWindow);
	glutSpecialFunc(SpecialInMainWindow);
	
	// Create a GLUT Color Picker
	
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
		
	// Create right-button menu:
	int RightButtonMenu = glutCreateMenu(RightButtonAction);
	// glutAddMenuEntry("Open.../O", OpenMenuItem);
	glutAddSubMenu("Background Color", ColorMenu);
  	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	
	return 0;
}


#if 0
#include <stdio.h>
#include <stdlib.h>
#include <fstream.h>
#include <algorithm.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "OpenSave_Interface.h"
#include "TRRender.h"
#include "TR_ACParser.h"


typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;


// For opening the data files
const short OpenFileDialogID = 200;


// Renderer object
SmartPtr<TRRender> RPtr;


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

// Like the above, but on boolean values
void DoMenuItemChecking_Boolean(int MenuItem, char *ItemText, bool Selector, int Value) {
	char TextBuffer[256];
	if (Selector)
		strcpy(TextBuffer,CheckedText);
	else
		strcpy(TextBuffer,UncheckedText);
	strcat(TextBuffer," ");
	strcat(TextBuffer,ItemText);
	glutChangeToMenuEntry(MenuItem,TextBuffer,Value);
}

// Construct a crude color picker
int RedIndx = 0, GreenIndx = 0, BlueIndx = 0;
const int CIndxRange = 5;
const float CVals[CIndxRange] = {0, 0.25, 0.5, 0.75, 1};


void SetBackgroundColor() {
	glClearColor(CVals[RedIndx],CVals[GreenIndx],CVals[BlueIndx],1);
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


// Transformation-matrix cache:
// Projection of model into view space
GLdouble ModelProjMatrix[16];
// Text projection matrix (y increases downward)
GLdouble TextMatrix[16];
// Flipped text projection matrix (y increases upward)
GLdouble FlippedTextMatrix[16];


// Scale amount
double Scale;
void ResetScale() {Scale = 512;}

// Translation amounts
double XTrans, YTrans;
void ResetTrans() {XTrans = 0; YTrans = 0;}

// Rotation amounts
double Azimuth, Elevation;
void ResetRotation() {Azimuth = 0; Elevation = 0;}

// Combined reset
void ResetView() {
	ResetScale();
	ResetTrans();
	ResetRotation();
}


// Main-window stuff
int MainWindowID;
int Width, Height;

const int MainWindowWidth = 640;
const int MainWindowHeight = 576;


// What to Render:
enum RenderSelectEnum {
	RS_Mesh,
	RS_Movable,
	RS_Scenery,
	RS_NUM_TYPES
} RenderSelect = RS_Mesh;


// What menu
int RSMenu;

// Update the menu checkmarking
void UpdateRSMenu() {
	glutSetMenu(RSMenu);
	int Indx = 1;
	DoMenuItemChecking(Indx++,"Meshes",RenderSelect,RS_Mesh);
	DoMenuItemChecking(Indx++,"Movables",RenderSelect,RS_Movable);
	DoMenuItemChecking(Indx++,"Scenery",RenderSelect,RS_Scenery);
}

// Render style: wireframe, false color, textured

// What menu
int RTMenu;

int RenderStyle = TRRender::Style_Textured;

// Update the menu checkmarking
void UpdateRTMenu() {
	glutSetMenu(RTMenu);
	int Indx = 1;
	DoMenuItemChecking(Indx++,"Wireframe",RenderStyle,TRRender::Style_Wireframe);
	DoMenuItemChecking(Indx++,"False Color by Object",RenderStyle,TRRender::Style_FalseColorByObject);
	DoMenuItemChecking(Indx++,"False Color by Polygon",RenderStyle,TRRender::Style_FalseColorByPoly);
	DoMenuItemChecking(Indx++,"Textured",RenderStyle,TRRender::Style_Textured);
}


// Controls (originally MUI,
// but the MUI stuff had been removed as a cure worse than the disease):

// This also applies for meshes and scenery
// Frame selector:
// MUI object, range, which in range, frame in "absolute" terms
int Frame_Range, Frame_Which, CurrentFrame;

// Animation selector
// MUI object, range, which in range, anim in "absolute" terms
int Anim_Range, Anim_Which, CurrentAnim;


// List-member selection slider and its label
// It's associated with "SelectRange" (total range of values to select) and
// "SelectWhich" (which of them to select: 0 to SelectRange-1)
int SelectRange[RS_NUM_TYPES], SelectWhich[RS_NUM_TYPES];

void UpdateSelectLabel() {
	
	char Buffer[256];
	sprintf(Buffer,"Select: %d/%d;   Animation: %d/%d   Frame: %d/%d",
		SelectWhich[RenderSelect],SelectRange[RenderSelect],
			Anim_Which,Anim_Range, Frame_Which,Frame_Range);
}


// Stuff for frame selector

// If changed from outside
void UpdateFrame() {
	UpdateSelectLabel();
	
	CurrentFrame = RPtr->Data->Animations[CurrentAnim].FrameStart + Frame_Which;
}

void ResetFrame() {
	Frame_Range = Frame_Which = 0;
	UpdateFrame();
}

// This sets up the frame-selector values
void SetFrameSelect() {
	if (RenderSelect != RS_Movable) {
		Frame_Range = Frame_Which = 0;
		UpdateSelectLabel();
		return;
	}
	
	int MvblIndx = SelectWhich[RenderSelect];
	TR_Movable &M = RPtr->Data->Movables[MvblIndx];
	// Reject (-1) (no anims)
	if (M.Anim < 0)
		ResetFrame();
	else {
		// Get the range of frames
		Frame_Range = RPtr->Data->MvblAddls[MvblIndx].AnimAddls[Anim_Which].Frames.size();
		Frame_Which = 0;
		UpdateFrame();
	}
}


// State-change and animation-dispatch controls;
int WhichStateChange = 0, WhichAnimDispatch = 0;


bool PrevStateChange() {
	if (RenderSelect != RS_Movable) return false;
	if (CurrentAnim < 0) return false;
	TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
	if (A.NumStateChanges <= 1) return false;
	if (WhichStateChange > 0)
		WhichStateChange--;
	else
		WhichStateChange = A.NumStateChanges - 1;
	WhichAnimDispatch = 0;
	
	return true;
}

bool NextStateChange() {
	if (RenderSelect != RS_Movable) return false;
	if (CurrentAnim < 0) return false;
	TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
	if (A.NumStateChanges <= 1) return false;
	if (WhichStateChange < A.NumStateChanges-1)
		WhichStateChange++;
	else
		WhichStateChange = 0;
	WhichAnimDispatch = 0;
	
	return true;
}

bool PrevAnimDispatch() {
	if (RenderSelect != RS_Movable) return false;
	if (CurrentAnim < 0) return false;
	TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
	if (A.NumStateChanges <= 0) return false;
	TR_StateChange &SC = RPtr->Data->StateChanges[A.StateChange + WhichStateChange];
	if (SC.NumAD <= 1) return false;	
	if (WhichAnimDispatch > 0)
		WhichAnimDispatch--;
	else
		WhichAnimDispatch = SC.NumAD - 1;

	return true;
}

bool NextAnimDispatch() {
	if (RenderSelect != RS_Movable) return false;
	if (CurrentAnim < 0) return false;
	TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
	if (A.NumStateChanges <= 0) return false;
	TR_StateChange &SC = RPtr->Data->StateChanges[A.StateChange + WhichStateChange];
	if (SC.NumAD <= 1) return false;
	if (WhichAnimDispatch < SC.NumAD-1)
		WhichAnimDispatch++;
	else
		WhichAnimDispatch = 0;

	return true;
}


// Model-alias stuff
vector<int> ModelAliases;
int OriginalIndx, AliasIndx;


// Stuff for animation selector

// If changed from outside
void UpdateAnim() {
	
	int MvblIndx = SelectWhich[RenderSelect];
	TR_Movable &M = RPtr->Data->Movables[MvblIndx];
	CurrentAnim = M.Anim + Anim_Which;
	
	WhichStateChange = 0;
	WhichAnimDispatch = 0;
	
	SetFrameSelect();
}

void ResetAnims() {
	Anim_Range = Anim_Which = 0;
	UpdateAnim();
}

// This sets up the animation-selector values
// Frames are set up by animation calls
void SetAnimSelect() {
	if (RenderSelect != RS_Movable) {
		Anim_Range = Anim_Which = Frame_Range = Frame_Which = 0;
		UpdateSelectLabel();
		return;
	}
	
	// Search for model aliases
	int MvblIndx = SelectWhich[RenderSelect];
	TR_Movable &M = RPtr->Data->Movables[MvblIndx];
	int NumMeshes = M.NumMeshes;
	ModelAliases.clear();
	if (NumMeshes > 2) {
		// First pass: count the aliases; those that share topology with original
		int NumAliases = 0;
		int NumMvbls = RPtr->Data->Movables.size();
		long *MeshTree = &RPtr->Data->MeshTrees[M.MeshTree];
		for (int im=0; im<NumMvbls; im++) {
			TR_Movable &AM = RPtr->Data->Movables[im];
			if (AM.NumMeshes != NumMeshes) continue;
			long *AltMeshTree = &RPtr->Data->MeshTrees[AM.MeshTree];
			bool Matched = true;
			for (int inod=0; inod<NumMeshes-1; inod++) {
				if (AltMeshTree[4*inod] != MeshTree[4*inod]) {
					Matched = false;
					break;
				}
			}
			if (Matched) {
				if (im == MvblIndx) OriginalIndx = ModelAliases.size();
				ModelAliases.push_back(im);
			}
		}
		
		AliasIndx = OriginalIndx;
	}
	// Ensure that there is at least one entry
	if (ModelAliases.empty()) {
		ModelAliases.resize(1);
		ModelAliases[0] = MvblIndx;
		AliasIndx = OriginalIndx = 0;
	}
	
	// Reject (-1) (no anims)
	if (M.Anim < 0)
		ResetAnims();
	else {			
		Anim_Range = RPtr->Data->MvblAddls[MvblIndx].AnimAddls.size();
		Anim_Which = 0;
		UpdateAnim();
	}
}


// If SelectRange and SelectWhich are specified from outside, then update the label
// and the slider
void UpdateSelectors() {
	UpdateSelectLabel();
}

// What selection range
void ResetSelectors() {
	
	SelectRange[RS_Mesh] = RPtr->Data->MeshObjects.size();
	SelectRange[RS_Movable] = RPtr->Data->Movables.size();
	SelectRange[RS_Scenery] = RPtr->Data->SceneryDefs.size();

	SelectWhich[RS_Mesh] = 0;
	SelectWhich[RS_Movable] = 0;
	SelectWhich[RS_Scenery] = 0;
	
	UpdateSelectors();
}


// Count of ticks before advancing
int WhichTick = 0;

bool UseTickrate = false, IsPlaying = false, IsLooping = false;

void AdvanceToNextFrame() {
	// Don't do this operation if not showing movables
	if (RenderSelect != RS_Movable) return;
	
	if (UseTickrate) {
		// Be sure to avoid the no-frame and one-frame cases
		if (!(IsPlaying && Frame_Range > 1)) return;
		
		short AnimStart = RPtr->Data->Movables[SelectWhich[RenderSelect]].Anim;
		if (AnimStart < 0) return;
		
		TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
		if (WhichTick < short(A.Ticks)-1) {
			WhichTick++;
			glutSetWindow(MainWindowID);
			glutPostRedisplay();
			return;
		}
	}
	
	// Be sure to avoid the no-frame and one-frame cases
	if (!(IsPlaying && Frame_Range > 1)) return;
	
	// At the end?
	if (Frame_Which >= (Frame_Range - 1)) {
		// If at end, come around
		Frame_Which = 0;
	} else if (Frame_Which == (Frame_Range - 2)) {
		// If just before end and not looped; set up to stop at the end
		Frame_Which++;
		IsPlaying = IsLooping;
	} else
		Frame_Which++;
	
	// Reset tick count
	WhichTick = 0;
	
	UpdateFrame();
	glutSetWindow(MainWindowID);
	glutPostRedisplay();
}

// Whether to show the 3D and 2D reference points
bool Show3D = false, Show2D_XZ = false, Show2D_XY = false;


// Jump to the current animation's next animation and frame
// Returns whether or not the change could be done
bool JumpToNextAnim() {
	if (RenderSelect != RS_Movable) return false;
	if (CurrentAnim < 0) return false;
	
	// Current one:
	TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
	int NextAnim = A.NextAnim;
	int NextFrame = A.NextFrame;
	
	// Select the new animation:
	short AnimStart = RPtr->Data->Movables[SelectWhich[RenderSelect]].Anim;
	int NewAnimRelIndx = NextAnim - AnimStart;
	if (NewAnimRelIndx < 0) return false;
	if (NewAnimRelIndx >= Anim_Range) return false;
	Anim_Which = NewAnimRelIndx;
	UpdateAnim();
	
	// Select the new frame:
	TR_Animation &NA = RPtr->Data->Animations[CurrentAnim];
	int NewFrameRelIndx = NextFrame - NA.FrameStart;
	NewFrameRelIndx = min(max(NewFrameRelIndx,0),Frame_Range-1);
	Frame_Which = NewFrameRelIndx;
	UpdateFrame();
	return true;
}


// Jump to the current animation dispatch's next animation and frame
// Returns whether or not the change could be done
bool GotoAnimDispatch() {
	if (RenderSelect != RS_Movable) return false;
	if (CurrentAnim < 0) return false;
	TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
	if (A.NumStateChanges <= 0) return false;
	TR_StateChange &SC = RPtr->Data->StateChanges[A.StateChange + WhichStateChange];
	if (SC.NumAD <= 0) return false;
	TR_AnimDispatch &AD = RPtr->Data->AnimDispatches[SC.ADStart + WhichAnimDispatch];
	int NextAnim = AD.NextAnim;
	int NextFrame = AD.NextFrame;
	
	// Select the new animation:
	short AnimStart = RPtr->Data->Movables[SelectWhich[RenderSelect]].Anim;
	int NewAnimRelIndx = NextAnim - AnimStart;
	if (NewAnimRelIndx < 0) return false;
	if (NewAnimRelIndx >= Anim_Range) return false;
	Anim_Which = NewAnimRelIndx;
	UpdateAnim();
	
	// Select the new frame:
	TR_Animation &NA = RPtr->Data->Animations[CurrentAnim];
	int NewFrameRelIndx = NextFrame - NA.FrameStart;
	NewFrameRelIndx = min(max(NewFrameRelIndx,0),Frame_Range-1);
	Frame_Which = NewFrameRelIndx;
	UpdateFrame();
	return true;
}

	
// Renderers...

long MeshBoundingRadius;

bool UseNormals;

int ArraySize;

bool Show_BBoxes = false;

void RenderMesh() {
	// Render the mesh proper
	if (!RPtr->RenderMeshObject(SelectWhich[RenderSelect])) return;
	
	// Indicate the sort of lighting
	TR_MeshObject &M = RPtr->Data->MeshObjects[SelectWhich[RenderSelect]];
	UseNormals = M.UseNormals;
	
	// Render the bounding sphere
	if (!Show_BBoxes) return;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	// The center of the bounding sphere
	MeshBoundingRadius = M.BoundingRadius;
	glTranslated(M.Center.x,M.Center.y,M.Center.z);
	
	// Checp way to do resizing
	double BoundSphereScale = double(MeshBoundingRadius);
	glScaled(BoundSphereScale,BoundSphereScale,BoundSphereScale);
	
	const int NSubDiv = 96;
	const double TwoPI = 8*atan(1.0);
	
	glColor3f(1,1,1);
	glBegin(GL_LINE_LOOP);
	for (int k=0; k<NSubDiv; k++) {
		double Angle = k*(TwoPI/NSubDiv);
		glVertex3d(cos(Angle),sin(Angle),0);
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (int k=0; k<NSubDiv; k++) {
		double Angle = k*(TwoPI/NSubDiv);
		glVertex3d(0,cos(Angle),sin(Angle));
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (int k=0; k<NSubDiv; k++) {
		double Angle = k*(TwoPI/NSubDiv);
		glVertex3d(sin(Angle),0,cos(Angle));
	}
	glEnd();
	
	glPopMatrix();
}


// Designed to work with some indices into an array;
// it draws a quadrangle and its diagonal lines;
// the input indices must be go in a circle around the perimeter
void DrawCrossedQuad(const GLushort *QuadIndices) {
	GLushort LineIndices[4];
	
	LineIndices[0] = QuadIndices[0];
	LineIndices[1] = QuadIndices[1];
	LineIndices[2] = QuadIndices[2];
	LineIndices[3] = QuadIndices[3];
	glDrawElements(GL_LINE_LOOP,4,GL_UNSIGNED_SHORT,LineIndices);
	
	LineIndices[0] = QuadIndices[0];
	LineIndices[1] = QuadIndices[2];
	LineIndices[2] = QuadIndices[1];
	LineIndices[3] = QuadIndices[3];
	glDrawElements(GL_LINES,4,GL_UNSIGNED_SHORT,LineIndices);
}

// Draws a bounding box
void DrawBoundingBox(TR_BoundingBox& BBox) {
	
	TR_Vector Vertices[8];
	for (int iv=0; iv<8; iv++) {
		if (iv & 0x01)
			Vertices[iv].x = BBox.xmax;
		else
			Vertices[iv].x = BBox.xmin;
			
		if (iv & 0x02)
			Vertices[iv].y = BBox.ymax;
		else
			Vertices[iv].y = BBox.ymin;
			
		if (iv & 0x04)
			Vertices[iv].z = BBox.zmax;
		else
			Vertices[iv].z = BBox.zmin;
	}
	glVertexPointer(3,GL_SHORT,sizeof(TR_Vector),Vertices);
			
	const GLushort FaceVertList[6][4] = {
		{0,1,3,2},
		{1,0,4,5},
		{3,1,5,7},
		{2,3,7,6},
		{0,2,6,4},
		{4,6,7,5}
	};
	for (int iq=0; iq<6; iq++)
		DrawCrossedQuad(FaceVertList[iq]);
	/*
	glPointSize(3);
	glColor3f(1,1,1);
	GLushort EndPoints[2] = {0,7};
	glDrawElements(GL_POINTS,2,GL_UNSIGNED_SHORT,EndPoints);
	*/
}

inline void TR_Vector_Add(TR_Vector &a, TR_Vector &b, TR_Vector &c) {
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
}
inline void TR_Vector_Sub(TR_Vector &a, TR_Vector &b, TR_Vector &c) {
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
}

TR_Movable *MovPtr = 0; // Appropriate init: none
int AnimIndx;

void RenderMovable() {
	// Get objects
	int MvblIndx = SelectWhich[RenderSelect];
	TR_Movable &M = RPtr->Data->Movables[MvblIndx];
	bool AnimPresent = M.Anim >= 0;
	MovPtr = 0;
	
	// Render the movable proper
	if (!RPtr->RenderMovable(ModelAliases[AliasIndx],AnimPresent,MvblIndx,Anim_Which,Frame_Which)) return;
	MovPtr = &M;
	
	// Now the bounding boxes
	if (Show_BBoxes && AnimPresent) {
		glColor3f(1,1,1);
		DrawBoundingBox(RPtr->Data->MvblAddls[MvblIndx].AnimAddls[Anim_Which].Frames[Frame_Which].BoundingBox);
	}
	
	// Render the position references
	if (AnimPresent) {
		struct TR_RenderPosRef: public TR_ACParser {
			
			// Whether to show these:
			bool Show3D, Show2D_XZ, Show2D_XY; 
			
			// Lengths of reference lines
			double FullLength, TickLength;
			
			// Which to make:
			// Bit 2 (4) -- x
			// Bit 1 (2) -- y
			// Bit 0 (1) -- z
			void MakeCross(short x, short y, short z, short Which) {
				glBegin(GL_LINES);
				if (Which & 0x0004) {
					glVertex3d(x-TickLength,y,z);
					glVertex3d(x+TickLength,y,z);
				}
				if (Which & 0x0002) {
					glVertex3d(x,y-TickLength,z);
					glVertex3d(x,y+TickLength,z);
				}
				if (Which & 0x0001) {
					glVertex3d(x,y,z-TickLength);
					glVertex3d(x,y,z+TickLength);
				}
				glEnd();
			}
			
			// Opcode 1: 3D Position Reference: args are the coordinates
			void Action_3DPos(short x, short y, short z) {
				glColor3f(1,1,1);
				if (Show3D)
					MakeCross(x,y,z,7);
			}
			
			// Opcode 2: 3d Position Reference: args are the coordinates
			void Action_2DPos(short w1, short w2) {
				glColor3f(1,1,1);
				if (Show2D_XZ) {
					glBegin(GL_LINES);
					glVertex3d(w1,-FullLength,w2);
					glVertex3d(w1,FullLength,w2);
					glEnd();
					MakeCross(w1,-FullLength,w2,5);
					MakeCross(w1,0,w2,5);
					MakeCross(w1,FullLength,w2,5);
					/*
					glBegin(GL_LINES);
					glVertex3d(w2,-FullLength,w1);
					glVertex3d(w2,FullLength,w1);
					glEnd();
					MakeCross(w2,-FullLength,w1,5);
					MakeCross(w2,0,w1,5);
					MakeCross(w2,FullLength,w1,5);
					*/
				}
				if (Show2D_XY) {
					glBegin(GL_LINES);
					glVertex3d(w1,w2,-FullLength);
					glVertex3d(w1,w2,FullLength);
					glEnd();
					MakeCross(w1,w2,-FullLength,6);
					MakeCross(w1,w2,0,6);
					MakeCross(w1,w2,FullLength,6);
				}
			}
		} RenderPosRef;
		RenderPosRef.AnimCommands = &RPtr->Data->AnimCommands[0];
		RenderPosRef.FullLength = Scale;
		RenderPosRef.TickLength = Scale/16;
		RenderPosRef.Show3D = Show3D;
		RenderPosRef.Show2D_XZ = Show2D_XZ;
		RenderPosRef.Show2D_XY = Show2D_XY;
		RenderPosRef.DoParse(RPtr->Data->Animations[CurrentAnim]);
	}
}


TR_SceneryDef *ScePtr = 0; // Appropriate init: none

void RenderScenery() {
	// Get objects
	ScePtr = 0;
	TR_SceneryDef &S = RPtr->Data->SceneryDefs[SelectWhich[RenderSelect]];
	ScePtr = &S;

	// Render the scenery object
	RPtr->RenderMeshObject(S.Mesh);
	TR_MeshObject &M = RPtr->Data->MeshObjects[S.Mesh];
	
	// Indicate what sort of lighting
	UseNormals = M.UseNormals;
	
	// Now the bounding boxes
	if (Show_BBoxes) {
		
		glColor3f(0,1,1);
		DrawBoundingBox(S.VisibilityBBox);
		glColor3f(1,1,0);
		DrawBoundingBox(S.CollisionalBBox);
	}
}


// Generic text renderer: makes a drop shadow for easier readability
void RenderText(int HPos, int VPos, char *Text, bool Hilite = false) {
	int TxtLen = strlen(Text);
	
	glColor3f(0,0,0);
	glRasterPos2i(HPos+1,VPos+1);
	for (int q=0; q<TxtLen; q++)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13,Text[q]);
	if (Hilite) {
		int Top = VPos - 12;
		int Left = HPos - 3;
		int Bottom = VPos + 2;
		int Right = HPos + 8*TxtLen+2;
		Top++; Left++; Bottom++; Right++;
		glBegin(GL_LINE_LOOP);
		glVertex2i(Left,Top);
		glVertex2i(Right,Top);
		glVertex2i(Right,Bottom);
		glVertex2i(Left,Bottom);
		glEnd();
	}
	
	glColor3f(1,1,1);
	glRasterPos2i(HPos,VPos);
	for (int q=0; q<TxtLen; q++)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13,Text[q]);
	if (Hilite) {
		int Top = VPos - 12;
		int Left = HPos - 3;
		int Bottom = VPos + 2;
		int Right = HPos + 8*TxtLen+2;
		glBegin(GL_LINE_LOOP);
		glVertex2i(Left,Top);
		glVertex2i(Right,Top);
		glVertex2i(Right,Bottom);
		glVertex2i(Left,Bottom);
		glEnd();
	}
}

void RenderMeshText() {
	// Text-position tracker
	int HPos = 16, VPos = 16;
	
	char Buffer[256];
	sprintf(Buffer,"Mesh: %d/%d",SelectWhich[RenderSelect],SelectRange[RenderSelect]);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	sprintf(Buffer,"Bounding Radius: %ld",MeshBoundingRadius);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
		
	if (UseNormals) {
		sprintf(Buffer,"Lighting: External");
		RenderText(HPos,VPos,Buffer);
	} else {
		sprintf(Buffer,"Lighting: Internal");
		RenderText(HPos,VPos,Buffer);
	}
}


void RenderModelAliasText() {
	// No need if only one
	if (ModelAliases.size() <= 1) return;

	// Text-position tracker
	int HPos = Width-80, VPos = 16;
	RenderText(HPos,VPos,"Aliases:");
	
	int MvblIndx = SelectWhich[RenderSelect];
	
	char Buffer[256];
	for (int im=0; im<ModelAliases.size(); im++) {
		VPos += 16;
		
		int MAIndx = ModelAliases[im];
		long ID = RPtr->Data->Movables[MAIndx].ID;
		sprintf(Buffer,"%ld %ld",MAIndx,ID);
		RenderText(HPos,VPos,Buffer,im==AliasIndx);
	}
}


void RenderMovableText() {
	// Check on it
	if (MovPtr == 0) return;
	
	RenderModelAliasText();
	
	// Text-position tracker
	int HPos = 16, VPos = 16;
	
	char Buffer[256];
	sprintf(Buffer,"Movable: %d/%d",SelectWhich[RenderSelect],SelectRange[RenderSelect]);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	sprintf(Buffer,"Type ID: %ld",MovPtr->ID);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	sprintf(Buffer,"Num Meshes: %d",MovPtr->NumMeshes);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	if (CurrentAnim < 0) {
		sprintf(Buffer,"No Animations");
		RenderText(HPos,VPos,Buffer);
		return;
	}
	
	TR_Animation &A = RPtr->Data->Animations[CurrentAnim];
	
	sprintf(Buffer,"Anim, Frame: %ld %ld",CurrentAnim, CurrentFrame);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;	
	
	sprintf(Buffer,"State ID: %u",A.State);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;	
	
	sprintf(Buffer,"Ticks: %u",u_short(A.Ticks));
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;	
	
	sprintf(Buffer,"Frames: %u %u",A.FrameStart,A.FrameEnd);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;	
	
	sprintf(Buffer,"Next A, F: %u %u",A.NextAnim,A.NextFrame);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;	
	
	sprintf(Buffer,"Vel, Accel:");
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;	
	
	sprintf(Buffer,"%lf %lf",A.Velocity/65536.0,A.Acceleration/65536.0);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	sprintf(Buffer,"%.4hx %.4hx %.4hx %.4hx",
		A.Unknown[0],A.Unknown[1],A.Unknown[2],A.Unknown[3]);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;	
	
	// Parse the animation commands
	VPos += 8;
	struct TR_ShowAC: public TR_ACParser {
		
		int FrameIndx;
		int *HPosPtr, *VPosPtr;
		char *Buffer;
		
		void Show(bool Hilite) {
			*VPosPtr += 16;
			RenderText(*HPosPtr,*VPosPtr,Buffer,Hilite);
		}
		
		// Opcode 1: 3D Position Reference: args are the coordinates
		void Action_3DPos(short x, short y, short z) {
			sprintf(Buffer,"3D Ref: %d %d %d",x,y,z);
			Show(false);
		}
		
		// Opcode 2: 2D Position Reference: args are the coordinates
		void Action_2DPos(short w1, short w2) {
			sprintf(Buffer,"2D Ref: %d %d",w1,w2);
			Show(false);
		}
		
		// Opcode 3: For slaved animations?
		void Action_Op3() {
			sprintf(Buffer,"#3");
			Show(false);
		}
		
		// Opcode 4: For becoming stationary?
		void Action_Op4() {
			sprintf(Buffer,"#4");
			Show(false);
		}
		
		// Opcode 5: Sound: args are the frame and the internal sound index
		void Action_Sound(short frame, short snd) {
			sprintf(Buffer,"Sound: %d %d %d",frame,TopBits(snd),RestBits(snd));
			Show(frame == FrameIndx);
		}
		
		// Opcode 6: Miscellaneous: args are the frame and the miscellaneous action
		void Action_Misc(short frame, short misc) {
			sprintf(Buffer,"Misc: %d %d %d",frame,TopBits(misc),RestBits(misc));
			Show(frame == FrameIndx);
		}
		
		// Invalid-opcode action
		void InvalidOpcode(short opcode) {
			sprintf(Buffer,"Bad opcode: %d",opcode);
			Show(false);
		}
		
		// Too-many-commands action
		void TooManyCommands(uint16 num_commands) {
			sprintf(Buffer,"Number of commands: %.4hx",num_commands);
			Show(false);
		}
	} ShowAC;
	ShowAC.AnimCommands = &RPtr->Data->AnimCommands[0];
	ShowAC.HPosPtr = &HPos;
	ShowAC.VPosPtr = &VPos;
	ShowAC.FrameIndx = CurrentFrame;
	ShowAC.Buffer = Buffer;
	ShowAC.DoParse(A);
	
	// Display the state changes;
	VPos += 24;
	RenderText(HPos,VPos,"State Changes:");
	
	u_short NumAD = 0, ADStart = 0;
	
	// Set up display window
	const int StateChangeWindowSize = 3;	
	int MinStateChange = 0;
	int AltMinStateChange = WhichStateChange - StateChangeWindowSize;
	int MaxStateChange = A.NumStateChanges - 1;
	int AltMaxStateChange = WhichStateChange + StateChangeWindowSize;
	
	if (AltMaxStateChange > MaxStateChange) {
		int Adjust = MaxStateChange - AltMaxStateChange;
		AltMinStateChange += Adjust;
		AltMaxStateChange += Adjust;
	}
	
	if (AltMinStateChange < MinStateChange) {
		int Adjust = MinStateChange - AltMinStateChange;
		AltMinStateChange += Adjust;
		AltMaxStateChange += Adjust;
	}
	
	// Set up "off edge" flags for rendering list-end text
	bool MinOffEdge = AltMinStateChange > MinStateChange;
	bool MaxOffEdge = AltMaxStateChange < MaxStateChange;
	
	if (MinOffEdge) MinStateChange = AltMinStateChange;
	if (MaxOffEdge) MaxStateChange = AltMaxStateChange;
	
	// Beginning of list	
	VPos += 16;
	if (MinOffEdge) {
		sprintf(Buffer,"*More*");
	} else {
		sprintf(Buffer,"  --  ");
	}
	RenderText(HPos,VPos,Buffer);

	for (int isc=MinStateChange; isc<=MaxStateChange; isc++) {
		VPos += 16;
		
		TR_StateChange &SC = RPtr->Data->StateChanges[A.StateChange + isc];
		char SelectChar;
		if (isc == WhichStateChange) {
			NumAD = SC.NumAD;
			ADStart = SC.ADStart;
			SelectChar = '>';
		} else
			SelectChar = ' ';
		sprintf(Buffer,"%c %d",SelectChar,SC.State);
		RenderText(HPos,VPos,Buffer);
	}
	
	// End of list
	VPos += 16;
	if (MaxOffEdge) {
		sprintf(Buffer,"*More*");
	} else {
		sprintf(Buffer,"  --  ");
	}
	RenderText(HPos,VPos,Buffer);
	
	// Display the animation dispatches
	VPos += 24;
	RenderText(HPos,VPos,"Dispatches:");
	VPos += 16;
	RenderText(HPos,VPos,"LoF HiF -> A F");
	
	if (A.NumStateChanges <= 0) return;
	for (int ina=0; ina<NumAD; ina++) {
		VPos += 16;
		
		TR_AnimDispatch &AD = RPtr->Data->AnimDispatches[ADStart + ina];
		char SelectChar = (ina == WhichAnimDispatch) ? '>' : ' ';
		sprintf(Buffer,"%c %d %d %d %d",SelectChar,AD.Low,AD.High,AD.NextAnim,AD.NextFrame);
		bool Hilite = (CurrentFrame >= AD.Low) && (CurrentFrame < AD.High);
		RenderText(HPos,VPos,Buffer,Hilite);
	}
}


void RenderSceneryText() {
	// Check on it
	if (ScePtr == 0) return;
	
	// Text-position tracker
	int HPos = 16, VPos = 16;
	
	char Buffer[256];
	sprintf(Buffer,"Scenery: %d/%d",SelectWhich[RenderSelect],SelectRange[RenderSelect]);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	sprintf(Buffer,"ID: %ld",ScePtr->ID);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	sprintf(Buffer,"Flags: %hd",ScePtr->attr);
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	if (ScePtr->attr&TR_SceneryDef::IsVisible)
		sprintf(Buffer,"Visible");
	else
		sprintf(Buffer,"Invisible");
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	if (ScePtr->attr&TR_SceneryDef::IsNotSolid)
		sprintf(Buffer,"Pass-Through");
	else
		sprintf(Buffer,"Solid");
	RenderText(HPos,VPos,Buffer);
	
	VPos += 16;
	
	if (UseNormals) {
		sprintf(Buffer,"Lighting: External");
		RenderText(HPos,VPos,Buffer);
	} else {
		sprintf(Buffer,"Lighting: Internal");
		RenderText(HPos,VPos,Buffer);
	}
}

// Render the coordinate axes
void RenderCoords() {
	// Make the lines
	glColor3f(1,0,0);
	glBegin(GL_LINES);
	glVertex3d(-Scale,0,0);
	glVertex3d(Scale,0,0);
	glEnd();
	
	glColor3f(0,1,0);
	glBegin(GL_LINES);
	glVertex3d(0,-Scale,0);
	glVertex3d(0,Scale,0);
	glEnd();

	glColor3f(0,0,1);
	glBegin(GL_LINES);
	glVertex3d(0,0,-Scale);
	glVertex3d(0,0,Scale);
	glEnd();
	
	// Make the text
	GLdouble CurrMVMatrix[16], CurrProjMatrix[16];
	GLint CurrViewport[4];
	GLdouble x,y,z;
	const GLdouble RelDisp = 0.75;
	
	glGetDoublev(GL_MODELVIEW_MATRIX,CurrMVMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX,CurrProjMatrix);
	glGetIntegerv(GL_VIEWPORT,CurrViewport);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(FlippedTextMatrix);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glColor3f(1,0,0);
	if (gluProject(-RelDisp*Scale,0,0,CurrMVMatrix,CurrProjMatrix,CurrViewport,&x,&y,&z)) {
		glRasterPos3d(x,y,z);
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'-');
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'X');
	}
	if (gluProject(RelDisp*Scale,0,0,CurrMVMatrix,CurrProjMatrix,CurrViewport,&x,&y,&z)) {
		glRasterPos3d(x,y,z);
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'+');
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'X');
	}
	
	glColor3f(0,1,0);
	if (gluProject(0,-RelDisp*Scale,0,CurrMVMatrix,CurrProjMatrix,CurrViewport,&x,&y,&z)) {
		glRasterPos3d(x,y,z);
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'-');
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'Y');
	}
	if (gluProject(0,RelDisp*Scale,0,CurrMVMatrix,CurrProjMatrix,CurrViewport,&x,&y,&z)) {
		glRasterPos3d(x,y,z);
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'+');
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'Y');
	}
	
	glColor3f(0,0,1);
	if (gluProject(0,0,-RelDisp*Scale,CurrMVMatrix,CurrProjMatrix,CurrViewport,&x,&y,&z)) {
		glRasterPos3d(x,y,z);
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'-');
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'Z');
	}
	if (gluProject(0,0,RelDisp*Scale,CurrMVMatrix,CurrProjMatrix,CurrViewport,&x,&y,&z)) {
		glRasterPos3d(x,y,z);
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'+');
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,'Z');
	}
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

bool Show_Coords = false;

bool ShowLighting = false;

bool SmoothTextures = true;

void ReloadTextures() {
	if (SmoothTextures) {
		RPtr->NearTxtrFiltering = GL_LINEAR;
		RPtr->FarTxtrFiltering = GL_LINEAR;
		// RPtr->FindTxtrMaps(TRRender::TMap_Center);
	} else {
		RPtr->NearTxtrFiltering = GL_NEAREST;
		RPtr->FarTxtrFiltering = GL_NEAREST;
		// RPtr->FindTxtrMaps(TRRender::TMap_FullOut);
	}
	RPtr->FindTxtrMaps(TRRender::TMap_Center);
	RPtr->ForceReload();
}


void DrawMainWindow() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Anything to render?
	// If not, then leave a blank screen
	if (SelectRange[RenderSelect] <= 0) {
		glutSwapBuffers();
		return;
	}
	
	// Set up for rendering the model
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(ModelProjMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	double ScaleRecip = 1/Scale;
	glScaled(ScaleRecip,ScaleRecip,ScaleRecip);
	glTranslated(XTrans,YTrans,0);
	glRotated(Elevation,1,0,0);
	glRotated(Azimuth,0,1,0);
	
	// Go from TR coordinates (y downwards, z outwards) to OpenGL coordinates
	// (y upwards, z inwards)
	glScaled(-1.0,-1.0,-1.0);
	
	RPtr->ShowLighting = ShowLighting;
	
	// Light comes from above
	RPtr->ToSource[0] = 0;
	RPtr->ToSource[1] = 1;
	RPtr->ToSource[2] = 0;
	
	// Render!
	RPtr->BeginObjectRender();
	
	switch(RenderSelect) {
	case RS_Mesh:
		RenderMesh();
		break;
	case RS_Movable:
		RenderMovable();
		break;
	case RS_Scenery:
		RenderScenery();
		break;
	}
	
	// Make this affected by semitransparency
	if (Show_Coords) RenderCoords();
	
	RPtr->EndObjectRender();
	
	// Do the text
	glPopAttrib();
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(TextMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	switch(RenderSelect) {
	case RS_Mesh:
		RenderMeshText();
		break;
	case RS_Movable:
		RenderMovableText();
		break;
	case RS_Scenery:
		RenderSceneryText();
		break;
	}
	glEnable(GL_DEPTH_TEST);
	
	// All done -- ready to show	
	glutSwapBuffers();
	
	// Playing?
	AdvanceToNextFrame();
}

enum ShowEnum
{
	Opt_ShowLighting,
	Opt_ShowCoords,
	Opt_ShowBBoxes,
	Opt_Show3D,
	Opt_Show2D_XZ,
	Opt_Show2D_XY
};

// Handle the show-stuff menu
int Opt_ShowMenu;

// Update the menu checkmarking
void UpdateShowMenu() {
	glutSetMenu(Opt_ShowMenu);
	int Indx = 1;
	DoMenuItemChecking_Boolean(Indx++,"Lighting",ShowLighting,Opt_ShowLighting);
	DoMenuItemChecking_Boolean(Indx++,"Coordinate Axes",Show_Coords,Opt_ShowCoords);
	DoMenuItemChecking_Boolean(Indx++,"Bounding Boxes",Show_BBoxes,Opt_ShowBBoxes);
	DoMenuItemChecking_Boolean(Indx++,"3D",Show3D,Opt_Show3D);
	DoMenuItemChecking_Boolean(Indx++,"2D XZ",Show2D_XZ,Opt_Show2D_XZ);
	DoMenuItemChecking_Boolean(Indx++,"2D XY",Show2D_XY,Opt_Show2D_XY);
}

enum OptionsEnum
{
	Options_Smooth,
	Options_UseTickrate,
	Options_IsLooping
};

// Handle the options menu
int OptionsMenu;

// Update the menu checkmarking
void UpdateOptionsMenu() {
	glutSetMenu(OptionsMenu);
	int Indx = 1;
	DoMenuItemChecking_Boolean(Indx++,"Smooth Textures",SmoothTextures,Options_Smooth);
	DoMenuItemChecking_Boolean(Indx++,"Use Tickrate",UseTickrate,Options_UseTickrate);
	DoMenuItemChecking_Boolean(Indx++,"Animations Looped",IsLooping,Options_IsLooping);
}


void ResizeMainWindow(int _Width, int _Height) {
	
	// Load these global variables
	Width = _Width;
	Height = _Height;

	glViewport(0, 0, Width, Height);
	
	glMatrixMode(GL_PROJECTION);
	
	// Set up the view matrix for the model rendering
	
	glLoadIdentity();
		
	int MinDim = min(Width,Height);
	double wid = double(MinDim)/double(Width);
	double ht = double(MinDim)/double(Height);
	
	glScaled(wid,ht,0.25); // Shrunk in z direction so that a model will fit inside of clip range
	
	glGetDoublev(GL_PROJECTION_MATRIX,ModelProjMatrix);

	// Set up the view matrix for the text rendering
	
	// y increasing downward
	
	glLoadIdentity();
	gluOrtho2D(0,Width,Height,0);
	
	glGetDoublev(GL_PROJECTION_MATRIX,TextMatrix);
	
	// y increasing upward
	
	glLoadIdentity();
	gluOrtho2D(0,Width,0,Height);
	
	glGetDoublev(GL_PROJECTION_MATRIX,FlippedTextMatrix);
}


// Load the data and set defaults
bool Load() {
	FileOpener FO;
	
	// What kinds of files to read
	FO.NumTypes = 3;
	FO.TypeList[0] = 'levl';
	FO.TypeList[1] = 'data';
	FO.TypeList[2] = 'AIFF';

	if (!FO.Request(OpenFileDialogID)) return false;
	char *FileSpec = FO.GetPath();
	
	cout << "Reading file: " << FileSpec << endl;
	
	ifstream RF(FileSpec,ios::in|ios::binary);
	TRRender *NewRPtr = TRRender::Make(RF);
	RF.close();
	
	if (NewRPtr == 0) {
		cout << "Readin failed" << endl;
		return false;
	}
	RPtr = NewRPtr;

	glutSetWindowTitle(FileSpec);

	// Do remaining setup:
	ResetSelectors();
	SetAnimSelect();
	ReloadTextures();
	return true;
}


const char ESCAPE = 0x1b;

// What fraction of the screen to translate a model:
const double TransFrac = 0.25;

void KeyInMainWindow(unsigned char key, int x, int y) {

	(void)(x,y);

	// Avoid distinguishing case
	int key_uppercase = toupper((int)key);
	
	switch(key_uppercase) {
	case ESCAPE:
		// Load a new level file
		Load();
		break;
		
	// Move model in view window
	case 'W':
		YTrans += TransFrac*Scale;	
		break;
	
	case 'S':
		YTrans -= TransFrac*Scale;
		break;
	
	case 'A':
		XTrans -= TransFrac*Scale;
		break;
	
	case 'D':
		XTrans += TransFrac*Scale;
		break;
	
	case 'X':
		XTrans = 0;
		YTrans = 0;
		break;
	
	case 'C':
		ResetScale();
		break;
	
	case 'Z':
		ResetRotation();
		break;
	
	// Change the selection
	case '+':
	case '=':
		if (SelectWhich[RenderSelect] < SelectRange[RenderSelect]-1)
			SelectWhich[RenderSelect]++;
		else
			SelectWhich[RenderSelect] = 0;
		UpdateSelectors();
		SetAnimSelect();
		break;
		
	case '-':
	case '_':
		if (SelectRange[RenderSelect] <= 0) return;
		if (SelectWhich[RenderSelect] > 0)
			SelectWhich[RenderSelect]--;
		else
			SelectWhich[RenderSelect] = SelectRange[RenderSelect]-1;
		UpdateSelectors();
		SetAnimSelect();
		break;
	
	// Change the animation
	case ']':
	case '}':
		if (RenderSelect != RS_Movable) return;
		if (Anim_Which < Anim_Range-1)
			Anim_Which++;
		else
			Anim_Which = 0;
		UpdateAnim();
		break;
		
	case '[':
	case '{':
		if (RenderSelect != RS_Movable) return;
		if (Anim_Range <= 0) return;
		if (Anim_Which > 0)
			Anim_Which--;
		else
			Anim_Which = Anim_Range-1;
		UpdateAnim();
		break;
	
	// Change the frame
	case '\'':
	case '"':
		if (RenderSelect != RS_Movable) return;
		if (Frame_Which < Frame_Range-1)
			Frame_Which++;
		else
			Frame_Which = 0;
		UpdateFrame();
		break;
	
	case ';':
	case ':':
		if (RenderSelect != RS_Movable) return;
		if (Frame_Range <= 0) return;
		if (Frame_Which > 0)
			Frame_Which--;
		else
			Frame_Which = Frame_Range-1;
		UpdateFrame();
		break;
	
	// Play/pause
	case ' ':
		if (RenderSelect != RS_Movable) return;
		IsPlaying = !IsPlaying;
		break;
	
	// Go to next anim/frame
	case 'G':
		if (!JumpToNextAnim()) return;
		break;
	
	// Previous state change
	case 'I':
		if (!PrevStateChange()) return;
		break;
		
	// Next state change
	case 'O':
		if (!NextStateChange()) return;
		break;
	
	// Previous animation dispatch
	case 'K':
		if (!PrevAnimDispatch()) return;
		break;
		
	// Next animation dispatch
	case 'L':
		if (!NextAnimDispatch()) return;
		break;
	
	// Go to dispatched animation
	case 'M':
		if (!GotoAnimDispatch()) return;
		break;
	
	// Previous alias
	case 'Y':
		if (RenderSelect != RS_Movable) return;
		if (ModelAliases.size() <= 1) return;
		if (AliasIndx > 0)
			AliasIndx--;
		else
			AliasIndx = ModelAliases.size() - 1;
		break;
			
	// Next alias
	case 'U':
		if (RenderSelect != RS_Movable) return;
		if (ModelAliases.size() <= 1) return;
		if (AliasIndx < ModelAliases.size()-1)
			AliasIndx++;
		else
			AliasIndx = 0;
		break;
			
	// Alias's original
	case 'J':
		if (RenderSelect != RS_Movable) return;
		if (ModelAliases.size() <= 1) return;
		AliasIndx = OriginalIndx;
	}
	
	// Will always be done after a keypress
	glutPostRedisplay();
}

const double AngleChange = 22.5;

void SpecialInMainWindow(int key, int x, int y) {

	(void)(x,y);
	
	switch(key) {
		// In MacOS Mesa GLUT, the help/fwd-delete/home/end/pgup/pgdn keys don't work :-P
	case GLUT_KEY_UP:
		Elevation -= AngleChange;
		break;
		
	case GLUT_KEY_DOWN:
		Elevation += AngleChange;
		break;
		
	case GLUT_KEY_LEFT:
		Azimuth += AngleChange;
		break;
		
	case GLUT_KEY_RIGHT:
		Azimuth -= AngleChange;
		break;
		
	// Scale the model in the view window
	case GLUT_KEY_PAGE_UP:
		Scale /= 2;
		break;
	
	case GLUT_KEY_PAGE_DOWN:
		Scale *= 2;
		break;
	}
	
	// Will always be done after a keypress
	glutPostRedisplay();
}

// Sets type of objects to render

void SetRenderSelection(int t) {

	RenderSelect = (RenderSelectEnum)t;
	UpdateRSMenu();
	SetAnimSelect();
	
	glutPostRedisplay();
}


// Sets what to show

void SetShowSelection(int t) {

	switch(t) {
	case Opt_ShowLighting:
		ShowLighting = !ShowLighting;
		break;
		
	case Opt_ShowCoords:
		Show_Coords = !Show_Coords;
		break;
		
	case Opt_ShowBBoxes:
		Show_BBoxes = !Show_BBoxes;
		break;
		
	case Opt_Show3D:
		Show3D = !Show3D;
		break;
		
	case Opt_Show2D_XZ:
		Show2D_XZ = !Show2D_XZ;
		break;
		
	case Opt_Show2D_XY:
		Show2D_XY = !Show2D_XY;
		break;
	}
	UpdateShowMenu();
	
	glutPostRedisplay();
}


// Sets what option to toggle

void SetOptionSelection(int t) {

	switch(t) {
	case Options_Smooth:
		SmoothTextures = !SmoothTextures;
		ReloadTextures();
		break;		
		
	case Options_UseTickrate:
		UseTickrate = !UseTickrate;
		break;
		
	case Options_IsLooping:
		IsLooping = !IsLooping;
		break;
	}
	UpdateOptionsMenu();

	glutPostRedisplay();
}

// Sets style of objects to render

void SetRenderStyle(int t) {

	RenderStyle = t;
	if (RPtr) {
		RPtr->InhabStyle = RenderStyle;
		
		switch(RenderStyle) {
		case TRRender::Style_Wireframe:
			RPtr->WireframeColor[0] = RPtr->WireframeColor[1] = RPtr->WireframeColor[2] = 1;
			break;
			
		case TRRender::Style_FalseColorByObject:
			RPtr->WireframeColor[0] = RPtr->WireframeColor[1] = RPtr->WireframeColor[2] = 0;
			RPtr->ObjectColor[0] = RPtr->ObjectColor[1] = RPtr->ObjectColor[2] = 0.5;
			break;
		}
	}
	UpdateRTMenu();
	
	glutPostRedisplay();
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


const int OpenMenuItem = 1;
void RightButtonAction(int c) {
	if (c == OpenMenuItem) Load();
}


int main(int argc, char **argv) {
	// Must be up here
	glutInit(&argc, argv);
	
	// Set up for creating the main window
	glutInitWindowSize(MainWindowWidth,MainWindowHeight);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	SetBackgroundColor();
	
	// Actually create that window
	MainWindowID = glutCreateWindow("Treasure Room");
	
	// Set its main callbacks
	glutDisplayFunc(DrawMainWindow);
	glutReshapeFunc(ResizeMainWindow);
	glutKeyboardFunc(KeyInMainWindow);
	glutSpecialFunc(SpecialInMainWindow);
	
	// Create object-selection submenus
	RSMenu = glutCreateMenu(SetRenderSelection);
	glutSetMenu(RSMenu);
	glutAddMenuEntry("Meshes",RS_Mesh);
	glutAddMenuEntry("Movables",RS_Movable);
	glutAddMenuEntry("Scenery",RS_Scenery);
	UpdateRSMenu();
	
	// Create what-to-show menu
	Opt_ShowMenu = glutCreateMenu(SetShowSelection);
	glutSetMenu(Opt_ShowMenu);
	glutAddMenuEntry("Lighting",Opt_ShowLighting);
	glutAddMenuEntry("Coordinate Axes",Opt_ShowCoords);
	glutAddMenuEntry("Bounding Boxes",Opt_ShowBBoxes);
	glutAddMenuEntry("3D",Opt_Show3D);
	glutAddMenuEntry("2D XZ",Opt_Show2D_XZ);
	glutAddMenuEntry("2D XY",Opt_Show2D_XY);
	UpdateShowMenu();
	
	// Create display-option submenus
	OptionsMenu = glutCreateMenu(SetOptionSelection);
	glutSetMenu(OptionsMenu);
	glutAddMenuEntry("Smooth Textures",Options_Smooth);
	glutAddMenuEntry("Use Tickrate",Options_UseTickrate);
	glutAddMenuEntry("Animations Looped",Options_IsLooping);
	UpdateOptionsMenu();
	
	RTMenu = glutCreateMenu(SetRenderStyle);
	glutSetMenu(RTMenu);
	glutAddMenuEntry("Wireframe",TRRender::Style_Wireframe);
	glutAddMenuEntry("False Color by Object",TRRender::Style_FalseColorByObject);
	glutAddMenuEntry("False Color by Polygon",TRRender::Style_FalseColorByPoly);
	glutAddMenuEntry("Textured",TRRender::Style_Textured);
	UpdateRTMenu();
	
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
		
	// Create right-button menu:
	int RightButtonMenu = glutCreateMenu(RightButtonAction);
	glutAddMenuEntry("Open.../O", OpenMenuItem);
	glutAddSubMenu("Object Type to Render", RSMenu);
	glutAddSubMenu("Show", Opt_ShowMenu);
	glutAddSubMenu("Options", OptionsMenu);
	glutAddSubMenu("Rendering Style", RTMenu);
	glutAddSubMenu("Background Color", ColorMenu);
  	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	if (!Load()) return -1;
	
	// Set everything to defaults before continuing
	TRRender::SetDefaults();
	ResetView();

	glutMainLoop();
	
	return 0;
}
#endif
