/*
	"Map Handler" utility that does a variety of tasks with map files


	Created by Loren Petrich on July 1, 2001
*/

#include <Dialogs.h>
#include <Fonts.h>
#include <MacWindows.h>
#include <Menus.h>
#include <QuickDraw.h>
#include <TextEdit.h>

#include


// Just want to do menubar handling

const short MainMenuBarID = 128;

const short AppleMenuID = 128;
const short FileMenuID = 129;

// File menu items
enum
{
	File_ListChunks = 1,
	File_Quit
};

void ListChunks();

void HandleFileMenu(int WhichItem)
{
	switch(WhichItem)
	{
	case File_ListChunks:
		ListChunks();
	
	case File_Quit:
		ExitToShell();
		break;
	}
}


void HandleAppleMenu(int WhichItem)
{
	Str255 ItemName;

	if (WhichItem == 1)
		Alert(MainMenuBarID,NULL);
	else
	{
		GetMenuItemText(GetMenuHandle(AppleMenuID), WhichItem, ItemName);
		short DA_RefNum = OpenDeskAcc(ItemName);
	}
}



void HandleMenuEvent(int MenuEvent)
{
	short WhichMenu = HiWord(MenuEvent);
	short WhichItem = LoWord(MenuEvent);
	
	switch(WhichMenu)
	{
	case AppleMenuID:
		HandleAppleMenu(WhichItem);
		break;
	
	case FileMenuID:
		HandleFileMenu(WhichItem);
		break;
	}
}


void HandleMouseDown(EventRecord& Event)
{
	WindowPtr WhichWindow;
	short Part = FindWindow(Event.where, &WhichWindow);
	
	switch(Part)
	{
	case inMenuBar:
		HandleMenuEvent(MenuSelect(Event.where));
		break;
	
	case inSysWindow:
		// Inside of Apple Menu
		SystemClick(&Event, WhichWindow);
		break;
	}
}


void HandleKey(EventRecord& Event)
{
	char Key = Event.message & charCodeMask;
	if (Event.modifiers & cmdKey)
		HandleMenuEvent(MenuKey(Key));
}


void HandleEvent(EventRecord& Event)
{
	switch(Event.what)
	{
	case mouseDown:
		HandleMouseDown(Event);
		break;
	
	case keyDown:
	case autoKey:
		HandleKey(Event);
		break;
	}
}


void main(void)
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
	// Create main menu
	Handle MainMenuHdl = GetNewMBar(MainMenuBarID);
	if (!MainMenuHdl) ExitToShell();
	SetMenuBar(MainMenuHdl);
	
	// Get the Apple Menu going
	AppendResMenu(GetMenuHandle(AppleMenuID), 'DRVR');
	
	DrawMenuBar();
	
	const short EventMask = mDownMask | keyDownMask | autoKeyMask;
	
	while(true)
	{
		EventRecord Event;
		bool GotEvent = WaitNextEvent(EventMask, &Event, 2, (RgnHandle) NULL);
		if (GotEvent) HandleEvent(Event);
	}
}


// User-interface stuff done; actual file-handling starts here

void ListChunks()
{

}
