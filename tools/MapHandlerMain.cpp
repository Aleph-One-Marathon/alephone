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
#include <Movies.h>
#include <stdio.h>
#include <stdlib.h>

#include "cseries.h"
#include "FileHandler.h"
#include "wad.h"


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


// Some extra stuff that A1 uses

void global_idle_proc() {}

void *level_transition_malloc(size_t size) {return malloc(size);}

unsigned char *TS_GetString(short ID, short Index) {return NULL;}


// Just want to do menubar handling

const short MainMenuBarID = 128;

const short AppleMenuID = 128;
const short FileMenuID = 129;

// File menu items
enum
{
	File_ListChunks = 1,
	File_Sep1,
	File_Quit
};

void ListChunks();

void HandleFileMenu(int WhichItem)
{
	switch(WhichItem)
	{
	case File_ListChunks:
		ListChunks();
		break;
	
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
	
	InitMacServices();
	
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

char *GetTagString(WadDataType Tag)
{
	static char TagString[5];
	
	TagString[0] = (unsigned char)(Tag >> 24);
	TagString[1] = (unsigned char)(Tag >> 16);
	TagString[2] = (unsigned char)(Tag >> 8);
	TagString[3] = (unsigned char)(Tag);
	TagString[4] = 0;
	
	return TagString;
}


void ListChunks()
{
	FileSpecifier InFileSpec;
	if (!InFileSpec.ReadDialog(_typecode_scenario,"List Chunks of")) return;
	
	OpenedFile InFile;
	if (!open_wad_file_for_reading(InFileSpec,InFile)) return;
	
	wad_header Header;
	if (!read_wad_header(InFile,&Header)) return;
	
	for (int lvl = 0; lvl < Header.wad_count; lvl++)
	{
		fdprintf("\nLevel %d",lvl);
		wad_data *Wad = read_indexed_wad_from_file(InFile, &Header, lvl, true);
		if (!Wad) continue;
		
		for (int itg = 0; itg < Wad->tag_count; itg++)
		{
			tag_data& Tag = Wad->tag_data[itg];
			fdprintf("%s",GetTagString(Tag.tag));
			fdprintf("%8d",Tag.length);
		}
		
		// All done!
		free_wad(Wad);
	}
	fdprintf("");
}
