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

char *GetTypeString(int32 Type)
{
	static char TypeString[5];
	
	TypeString[0] = (unsigned char)(Type >> 24);
	TypeString[1] = (unsigned char)(Type >> 16);
	TypeString[2] = (unsigned char)(Type >> 8);
	TypeString[3] = (unsigned char)(Type);
	TypeString[4] = 0;
	
	return TypeString;
}


void ListChunks()
{
	FileSpecifier InFileSpec;
	if (!InFileSpec.ReadDialog(_typecode_scenario,"List Chunks of")) return;
	
	// Compose a name for the chunk-list report
	char Name[256];
	InFileSpec.GetName(Name);
	
	strncat(Name," Report.txt",31);
	Name[31] = 0;
	
	FileSpecifier OutFileSpec;
	set_typecode(_typecode_patch,'TEXT');
	if (!OutFileSpec.WriteDialog(_typecode_patch,"What report file?",Name)) return;
	
	OpenedFile InFile;
	if (!open_wad_file_for_reading(InFileSpec,InFile)) return;
	
	// Mac stuff: save old default directory
	short OldVRefNum;
	long OldParID;
	HGetVol(nil,&OldVRefNum,&OldParID);
	
	// Set default directory to the selected-output directory
	if (HSetVol(nil, OutFileSpec.GetSpec().vRefNum, OutFileSpec.GetSpec().parID) != noErr) return;
	
	// Open the file
	OutFileSpec.GetName(Name);
	FILE *F = fopen(Name,"w");
	
	// All done with this fancy footwork
	HSetVol(nil,OldVRefNum,OldParID);
	
	// What it's about
	InFileSpec.GetName(Name);
	fprintf(F,"Map File: %s\n\n",Name);
	
	wad_header Header;
	if (!read_wad_header(InFile,&Header)) {fclose(F); return;}
	
	fprintf(F,"Chunks:\n");
	
	// Cribbed from calculate_directory_offset() in wad.cpp
	// and the definition of directory_entry in wad.h
	int IndexOffset = Header.directory_offset + 2*4;
	int IndexStep = Header.application_specific_directory_data_size +
		Header.directory_entry_base_size;
	
	for (int lvl = 0; lvl < Header.wad_count; lvl++)
	{
		short TrueLevel;
		InFile.SetPosition(IndexOffset + lvl*IndexStep);
		InFile.Read(2,&TrueLevel);
		
		fprintf(F,"\nLevel %d (%d):\n\n",lvl,TrueLevel);
		
		wad_data *Wad = read_indexed_wad_from_file(InFile, &Header, TrueLevel, true);
		if (!Wad) continue;
		
		for (int itg = 0; itg < Wad->tag_count; itg++)
		{
			tag_data& Tag = Wad->tag_data[itg];
			fprintf(F,"%s  %7d\n",GetTypeString(Tag.tag),Tag.length);
		}
		
		// All done!
		free_wad(Wad);
	}
	fprintf(F,"\nResources:\n");
	
	// Try reading the resource file
	OpenedResourceFile InRes;
	InFileSpec.Open(InRes);
	
	vector<uint32> ResourceTypes;
	InRes.GetTypeList(ResourceTypes);
	
	for (int it=0; it<ResourceTypes.size(); it++)
	{
		int32 Type = ResourceTypes[it];
		fprintf(F,"\nType: %s\n",GetTypeString(Type));
		
		vector<int16> ResourceIDs;
		InRes.GetIDList(Type,ResourceIDs);
		
		for (int ir=0; ir<ResourceIDs.size(); ir++)
		{
			InRes.Push();
			SetResLoad(false);
			short ID = ResourceIDs[ir];
			Handle ResourceHandle = Get1Resource(Type,ID);
			ResType _Type;
			short _ID;
			Str255 _Name;
			GetResInfo(ResourceHandle,&_ID,&_Type,_Name);
			int Size = GetResourceSizeOnDisk(ResourceHandle);
			memcpy(Name,_Name+1,_Name[0]);
			Name[_Name[0]] = 0;
						
			fprintf(F,"  %6hd  %7d  %s\n",ID,Size,Name);
			
			ReleaseResource(ResourceHandle);
			SetResLoad(true);
			InRes.Pop();
		}
	}
	fclose(F);
}
