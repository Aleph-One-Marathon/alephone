/*
	Model-Viewer Test Shell, by Loren Petrich, June 10, 2001
	This uses GLUT to create an OpenGL context for displaying a 3D model,
	in order to test model-reading code
*/

#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "FileHandler.h"
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

// Intended typecode of model/skin files to open
OSType TypeCode;

OSType get_typecode(int TypeCodeIndex)
{
	// Either that intended typecode or the creator code (index 0)
 	return TypeCodeIndex ? TypeCode : 'APPL';
}

void global_idle_proc() {} // Do nothing



// Model and skin objects:
Model3D Model;


// For loading models and skins
enum {
	ModelWavefront = 1
};
const int LoadSkinItem = 1;

void LoadModelAction(int ModelType)
{
	// Get the model file
	FileSpecifier File;
	
	// Specialize the dialog for what kind of model file
	switch(ModelType)
	{
	case ModelWavefront:
		TypeCode = 'TEXT';
		if (File.ReadDialog(1,"Model Type: Alias|Wavefront"))
			LoadModel_Wavefront(File, Model);
		break;	
	}
}


void LoadSkin()
{
	// Get the skin file
}


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
	if (c == LoadSkinItem) LoadSkin();
}


// Preset display-window side:
const int MainWindowWidth = 512, MainWindowHeight = 512;

int main(int argc, char **argv)
{
	// Must be up here
	glutInit(&argc, argv);
	
	// See if QT and NavSvcs (MacOS) are present
	InitMacServices();
	SetDebugOutput_Wavefront(stdout);
	
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
	
	// Create model and skin menu items
	int ModelMenu = glutCreateMenu(LoadModelAction);
	glutAddMenuEntry("Alias-Wavefront...",ModelWavefront);
	
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
	glutAddSubMenu("Load Model...", ModelMenu);
	glutAddMenuEntry("Load Skin...", LoadSkinItem);
	glutAddSubMenu("Background Color", ColorMenu);
  	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	
	return 0;
}
