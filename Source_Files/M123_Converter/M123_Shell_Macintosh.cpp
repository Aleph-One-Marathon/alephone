/*

	Marathon 1-2-3 Converter: Its purpose is to convert Marathon 1 data files into a form
	that Marathon-2-style engines can use.
	
	This and all related code is issued under the terms of the GNU General Public License.
	
	User-Interface Shell -- MacOS version. Created by Loren Petrich, Nov. 21, 2000
*/


#include <iostream.h>
#include <SIOUX.h>


// Set this in a dialog box; escapes from the main event loop
bool QuitRequested = false;

// Dialog-box stuff
DialogPtr MainDialogPtr = NULL;

const short MainDialogID = 200;

const short MainDialog_Quit = 2;

// Procedural, but an object-oriented abstraction is overkill for a single dialog box
void InitMainDialog();
void HandleMainDialogEvent(short ItemHit);


int main()
{
	cout << "Starting the 1-2-3 Converter" << endl;

	// Create main dialog box
	MainDialogPtr = GetNewDialog(MainDialogID, nil, WindowPtr(-1));
	if (!MainDialogPtr)
	{
		cout << "Error in creating main dialog box" << endl;
		return -1;
	}
	
	InitMainDialog();
	
	// How many ticks to give up control?
	long WaitTime = 5;
	
	// Main event loop
	while(!QuitRequested)
	{
		EventRecord EvRec;
		
		// Get new event; if none, then re-poll
		if (!WaitNextEvent(everyEvent, &EvRec, WaitTime, nil)) continue;
		
		// If SIOUX had handled it, then we are done with it
		if (SIOUXHandleOneEvent(&EvRec)) continue;
		
		// What is being clicked on?
		if (EvRec.what == mouseDown)
		{
			WindowPtr ThisWindow;
			short Where = FindWindow(EvRec.where,&ThisWindow);
			
			// Bring the window to the front if it is not already there
			if (ThisWindow != FrontWindow())
			{
				SelectWindow(ThisWindow);
				continue;
			}
			
			// Drag the window if clicked in draggable area
			if (Where == inDrag)
			{
				DragWindow(ThisWindow,EvRec.where,&(*GetGrayRgn())->rgnBBox);
				continue;
			}
			
			// No other processing necessary
		}
		
		// In a dialog box?
		if (IsDialogEvent(&EvRec))
		{
			DialogPtr DPtr;
			short ItemHit;
			if (DialogSelect (&EvRec, &DPtr, &ItemHit))
			{
				if (DPtr == MainDialogPtr) HandleMainDialogEvent(ItemHit);
			}
			continue;
		}
		
		// Ignore all others
	}
	
	// Clean up
	DisposeDialog(MainDialogPtr);
	
	cout << "All done" << endl;
	return 0;
}


void InitMainDialog()
{
	SelectWindow(MainDialogPtr);
	ShowWindow(MainDialogPtr);
}


void HandleMainDialogEvent(short ItemHit)
{
	switch(ItemHit)
	{
	case MainDialog_Quit:
		QuitRequested = true;
		break;
	}
}
