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
#include "map.h"
#include "editor.h"
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
	File_AddChunks,
	File_ExtractChunks,
	File_Sep1,
	File_Quit
};

void ListChunks();
void AddChunks();
void ExtractChunks();

void HandleFileMenu(int WhichItem)
{
	switch(WhichItem)
	{
	case File_ListChunks:
		ListChunks();
		break;
	
	case File_AddChunks:
		AddChunks();
		break;
	
	case File_ExtractChunks:
		ExtractChunks();
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


struct WadContainer
{
	wad_data *Ptr;
	
	void Clear() {if (Ptr) free_wad(Ptr); Ptr = NULL;}
	void Set(wad_data *_Ptr) {Clear(); Ptr = _Ptr;}
	
	WadContainer(): Ptr(NULL) {}
	WadContainer(wad_data *_Ptr): Ptr(_Ptr) {}
	~WadContainer() {if (Ptr) free_wad(Ptr);}
};


struct InFileData
{
	FileSpecifier Spec;
	OpenedFile Opened;
	wad_header Header;
	
	// Their true values, determined from the directory-entry indices,
	// and in the order that they appear that they appear in the directory
	vector<int16> LevelIndices;
	
	bool Open(char *Prompt);
	
	short NumWads() {return Header.wad_count;}
	
	bool GetWad(short TrueIndex, WadContainer& Wad);
};


bool InFileData::Open(char *Prompt)
{
	LevelIndices.clear();
	
	if (!Spec.ReadDialog(_typecode_scenario,Prompt)) return false;
	
	if (!open_wad_file_for_reading(Spec,Opened)) return false;
	
	if (!read_wad_header(Opened,&Header)) return false;

	// Cribbed from calculate_directory_offset() in wad.cpp
	// and the definition of directory_entry in wad.h
	int IndexOffset = Header.directory_offset + 2*4;
	int IndexStep = Header.application_specific_directory_data_size +
		Header.directory_entry_base_size;
	
	LevelIndices.resize(Header.wad_count);
	for (int lvl=0; lvl<NumWads(); lvl++)
	{
		short TrueLevel;
		Opened.SetPosition(IndexOffset + lvl*IndexStep);
		Opened.Read(2,&LevelIndices[lvl]);
	}
	
	return true;
}


bool InFileData::GetWad(short TrueIndex, WadContainer& Wad)
{
	Wad.Set(read_indexed_wad_from_file(Opened, &Header, TrueIndex, true));
	
	return (Wad.Ptr != NULL);
}

void ListChunks()
{
	InFileData InFile;
	if (!InFile.Open("Report on?")) return;
	
	// Compose a name for the chunk-list report
	char Name[256];
	InFile.Spec.GetName(Name);
	
	strncat(Name," Report.txt",31);
	Name[31] = 0;
	
	FileSpecifier OutFileSpec;
	set_typecode(_typecode_patch,'TEXT');
	if (!OutFileSpec.WriteDialog(_typecode_patch,"What report file?",Name)) return;
	
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
	InFile.Spec.GetName(Name);
	fprintf(F,"Map File: %s\n\n",Name);
	
	fprintf(F,"Chunks:\n");
		
	for (int lvl=0; lvl<InFile.NumWads(); lvl++)
	{
		short TrueLevel = InFile.LevelIndices[lvl];
		
		fprintf(F,"\nLevel %d (%d):\n\n",lvl,TrueLevel);
		
		WadContainer Wad;
		if (!InFile.GetWad(TrueLevel,Wad)) continue;
		
		for (int itg=0; itg<Wad.Ptr->tag_count; itg++)
		{
			tag_data& Tag = Wad.Ptr->tag_data[itg];
			fprintf(F,"%s  %7d\n",GetTypeString(Tag.tag),Tag.length);
		}
	}
	fprintf(F,"\nResources:\n");
	
	// Try reading the resource file
	OpenedResourceFile InRes;
	InFile.Spec.Open(InRes);
	
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


void AddChunks()
{
	InFileData InFile;
	if (!InFile.Open("Move resources into?")) return;
	
	// Compose a name for the chunk-list report
	char Name[256];
	InFile.Spec.GetName(Name);
	
	strncat(Name," Chunked.sce",31);
	Name[31] = 0;
	
	FileSpecifier OutFileSpec;
	if (!OutFileSpec.WriteDialog(_typecode_scenario,"What chunked file?",Name)) return;
	
	// Simply try creating the file and moving into it the first time around
	wad_header Header;
	fill_default_wad_header(OutFileSpec, CURRENT_WADFILE_VERSION, EDITOR_MAP_VERSION,
		InFile.NumWads(), sizeof(directory_entry) - SIZEOF_directory_entry, &Header);
	
	vector<directory_entry> DirEntries(InFile.NumWads());
	
	if (!create_wadfile(OutFileSpec,_typecode_scenario)) return;
	
	OpenedFile OutFileOpened;
	if (!open_wad_file_for_writing(OutFileSpec,OutFileOpened)) return;
	
	if (!write_wad_header(OutFileOpened, &Header)) return;

	int32 Offset = SIZEOF_wad_header;
	
	for (int lvl=0; lvl<InFile.NumWads(); lvl++)
	{
		WadContainer InWad;
		short TrueLevel = InFile.LevelIndices[lvl];
		if (!InFile.GetWad(TrueLevel,InWad)) InWad.Set(create_empty_wad());
		
		WadContainer OutWad(create_empty_wad());
		
		for (int itg=0; itg<InWad.Ptr->tag_count; itg++)
		{
			tag_data& Tag = InWad.Ptr->tag_data[itg];
			int32 DataLength;
			append_data_to_wad(OutWad.Ptr, Tag.tag, Tag.data, Tag.length, 0);
		}
		
		int32 OutWadLength = calculate_wad_length(&Header, OutWad.Ptr);
		
		set_indexed_directory_offset_and_length(&Header, 
						&DirEntries[0], lvl, Offset, OutWadLength, TrueLevel);
		
		if (!write_wad(OutFileOpened, &Header, OutWad.Ptr, Offset)) return;
		
		Offset += OutWadLength;
	}
	
	write_directorys(OutFileOpened, &Header, &DirEntries[0]);
	
	// The header needs an update
	Header.directory_offset = Offset;
	Header.parent_checksum= read_wad_file_checksum(OutFileSpec);
	if (!write_wad_header(OutFileOpened, &Header)) return;
}

void ExtractChunks()
{
}
