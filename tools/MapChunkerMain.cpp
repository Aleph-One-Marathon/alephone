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
#include "crc.h"
#include "map.h"
#include "editor.h"
#include "FileHandler.h"
#include "wad.h"


// Parallel resource/chunk ID's to handle with this app:

const int NumMovedTypes = 4;
const OSType MovedResourceTypes[NumMovedTypes] = {'PICT','clut','snd ','TEXT'};
const OSType MovedChunkTypes[NumMovedTypes] = {'pict','clut','snd ','text'};


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
	
	void Clear() {if (Ptr) free_wad(Ptr); Ptr = create_empty_wad();}
	void Set(wad_data *_Ptr) {if (Ptr) free_wad(Ptr); Ptr = _Ptr;}
	
	WadContainer(): Ptr(create_empty_wad()) {}
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
		if (!Opened.SetPosition(IndexOffset + lvl*IndexStep)) return false;
		if (!Opened.Read(2,&LevelIndices[lvl])) return false;
	}
	
	return true;
}


bool InFileData::GetWad(short TrueIndex, WadContainer& Wad)
{
	Wad.Set(read_indexed_wad_from_file(Opened, &Header, TrueIndex, true));
	
	return (Wad.Ptr != NULL);
}


struct OutFileData
{
	FileSpecifier Spec;
	OpenedFile Opened;
	wad_header Header;
	
	vector<directory_entry> DirEntries;
	int Offset;

	bool Open(char *Prompt, char *Default);
	
	int NumWads() {return DirEntries.size();}
	
	bool AddWad(short TrueIndex, WadContainer& Wad);
	
	bool Finish();
};


bool OutFileData::Open(char *Prompt, char *Default)
{
	if (!Spec.WriteDialog(_typecode_scenario,Prompt,Default)) return false;
	
	fill_default_wad_header(Spec, CURRENT_WADFILE_VERSION, EDITOR_MAP_VERSION,
		0, sizeof(directory_entry) - SIZEOF_directory_entry, &Header);
	
	if (!create_wadfile(Spec,_typecode_scenario)) return false;
	
	if (!open_wad_file_for_writing(Spec,Opened)) return false;
	
	if (!write_wad_header(Opened, &Header)) return false;
	
	Offset = SIZEOF_wad_header;
	
	return true;
}


bool OutFileData::AddWad(short TrueIndex, WadContainer& Wad)
{		
	// Add the new wad if it contains anything
	int32 WadLength = calculate_wad_length(&Header, Wad.Ptr);
	
	if (WadLength > 0)
	{
		directory_entry NewDirEntry;
		DirEntries.push_back(NewDirEntry);
		
		set_indexed_directory_offset_and_length(&Header, 
			&DirEntries[0], NumWads()-1, Offset, WadLength, TrueIndex);
			
		if (!write_wad(Opened, &Header, Wad.Ptr, Offset)) return false;
		
		Offset += WadLength;
	}
	
	return true;
}


bool OutFileData::Finish()
{
	
	// Reset the size
	Header.wad_count = NumWads();
		
	// Need to know where to write the directory
	Header.directory_offset = Offset;
	
	if (!write_directorys(Opened, &Header, &DirEntries[0])) return false;
	
	Header.checksum = calculate_crc_for_opened_file(Opened);
	if (!write_wad_header(Opened, &Header)) return false;
	
	return true;
}


void ListChunks()
{
	InFileData InFile;
	if (!InFile.Open("Report on?")) return;
	
	// Compose a name for the chunk-list report
	char Name[256];
	InFile.Spec.GetName(Name);
	
	strncat(Name," Report",31);
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
	if (!InFile.Spec.Open(InRes))
	{
		fprintf(F,"\n*** NONE ***\n");
		fclose(F);
		return;
	}
	
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


int FindInIDList(vector<int16>& IDList, int16 ID)
{
	int NumIDs = IDList.size();
	if (NumIDs == 0) return NONE;
	else if (NumIDs == 1)
	{
		if (ID == IDList[0]) return 0;
		else return NONE;
	}
	// NumIDs >= 2
	
	int16 Val;
	
	int MinIndx = 0;
	Val = IDList[MinIndx];
	if (ID < Val) return NONE;
	else if (ID == Val) return MinIndx;
	
	int MaxIndx = NumIDs - 1;
	Val = IDList[MaxIndx];
	if (ID > Val) return NONE;
	else if (ID == Val) return MaxIndx;
	
	while (MaxIndx > (MinIndx + 1))
	{
		int NewIndx = (MinIndx + MaxIndx) >> 1;
		Val = IDList[NewIndx];
		if (ID > Val)
			MinIndx = NewIndx;
		else if (ID < Val)
			MaxIndx = NewIndx;
		else
			return NewIndx;
	}
	return NONE;
}


void LoadResourceIntoArray(int TypeIndex, int16 RsrcID, OpenedResourceFile InRes,
	vector<uint8>& Data)
{	
	LoadedResource Rsrc;
	if (!InRes.Get(MovedResourceTypes[TypeIndex],RsrcID,Rsrc)) return;
	
	int RsrcLen = Rsrc.GetLength();
	if (RsrcLen <= 0) return;
	
	// Details cribbed from images.cpp in the engine code
	switch(TypeIndex)
	{
	case 0:
		// PICT
		// The most complicated :-)
		// Simply copy over
		Data.resize(RsrcLen);
		memcpy(&Data[0],Rsrc.GetPointer(),RsrcLen);
		break;
		
	case 1:
		// clut
		if (RsrcLen != (8 + 256 * 8)) return;
		Data.resize(6 + 256 * 6);
		objlist_clear(&Data[0],Data.size());
		{
			uint8 *In = (uint8 *)(Rsrc.GetPointer());
			uint8 *Out = &Data[0];
			
			Out[0] = In[6];
			Out[1] = In[7];
			In += 8;
			Out += 6;
			
			for (int i=0; i<256; i++)
			{
				In++;	// Index value
				In++;
				*Out++ = *In++;	// Red
				*Out++ = *In++;
				*Out++ = *In++;	// Green
				*Out++ = *In++;
				*Out++ = *In++;	// Blue
				*Out++ = *In++;
			}
		}
		break;
		
	case 2:
		// snd
	case 3:
		// TEXT
		// Simply copy over
		Data.resize(RsrcLen);
		memcpy(&Data[0],Rsrc.GetPointer(),RsrcLen);
		break;
	}
}


bool LoadDataIntoResource(int TypeIndex, byte *Data, int32 Length, OpenedResourceFile OutRes,
	int16 RsrcID)
{
	if (Length <= 0) return false;
	
	bool Success = false;
	OutRes.Push();
	
	Handle Rsrc = NULL;

	// Details cribbed from images.cpp in the engine code
	switch(TypeIndex)
	{
	case 0:
		// PICT
		// The most complicated :-)
		// Simply copy over
		PtrToHand(Data,&Rsrc,Length);
		break;
		
	case 1:
		// clut
		if (Length != (6 + 256 * 6))
		{
			Success = false;
			break;
		}
		Rsrc = NewHandleClear(8 + 256 * 8);
		{
			uint8 *In = Data;
			uint8 *Out = (uint8 *)(*Rsrc);
			
			Out[6] = In[0];
			Out[7] = In[1];
			In += 6;
			Out += 8;
			
			for (int i=0; i<256; i++)
			{
				Out++;	// Index value
				*Out++ = i;
				*Out++ = *In++;	// Red
				*Out++ = *In++;
				*Out++ = *In++;	// Green
				*Out++ = *In++;
				*Out++ = *In++;	// Blue
				*Out++ = *In++;
			}
		}
		break;
		
	case 2:
		// snd
	case 3:
		// TEXT
		// Simply copy over
		PtrToHand(Data,&Rsrc,Length);
		break;
	}
	
	if (Success)
	{
		AddResource(Rsrc, MovedResourceTypes[TypeIndex], RsrcID, "\p");
		Success = (ResError() == noErr);
	}
	if (Rsrc) DisposeHandle(Rsrc);
	
	OutRes.Pop();
	return Success;
}


void AddChunks()
{
	InFileData InFile;
	if (!InFile.Open("Move resources into?")) return;
	
	vector<int16> ResourceIDs[NumMovedTypes];
	vector<uint8> NotYetUsed[NumMovedTypes];
	
	OpenedResourceFile InRes;
	if (InFile.Spec.Open(InRes))
	{
		for (int it=0; it<NumMovedTypes; it++)
		{
			OSType Type = MovedResourceTypes[it];
			InRes.GetIDList(Type,ResourceIDs[it]);
			NotYetUsed[it].resize(ResourceIDs[it].size(),1);
			
			// Ignore any resources below ID 128
			for (int ix=0; ix<ResourceIDs[it].size(); ix++)
			{
				if (ResourceIDs[it][ix] < 128)
					NotYetUsed[it][ix] = 0;
			}
		}
	}
	
	// Compose a name for the chunked level
	char Name[256];
	InFile.Spec.GetName(Name);
	
	strncat(Name," Chunked",31);
	Name[31] = 0;
	
	OutFileData OutFile;
	
	if (!OutFile.Open("What chunked file?",Name)) return;
	
	for (int lvl=0; lvl<InFile.NumWads(); lvl++)
	{
		WadContainer InWad;
		short TrueLevel = InFile.LevelIndices[lvl];
		InFile.GetWad(TrueLevel,InWad);
		
		// Load the resources to be chunked
		vector<uint8> LoadedResources[NumMovedTypes];
		for (int it=0; it<NumMovedTypes; it++)
		{
			int Index = FindInIDList(ResourceIDs[it],TrueLevel);
			if (Index != NONE)
			{
				if (!NotYetUsed[it][Index])
					Index = NONE;
			}
			if (Index != NONE)
			{
				LoadResourceIntoArray(it,TrueLevel,InRes,LoadedResources[it]);
				// Kludge for forcing the program to read all the map file's resources
				InRes.Close();
				InFile.Spec.Open(InRes);
				
				NotYetUsed[it][Index] = 0;
			}
		}
		
		WadContainer OutWad;
		
		// Load the already-existing chunks; be sure to replace those that have
		// corresponding resources
		for (int itg=0; itg<InWad.Ptr->tag_count; itg++)
		{
			tag_data& Tag = InWad.Ptr->tag_data[itg];
			
			bool WasFound = false;
			for (int it = 0; it<NumMovedTypes; it++)
			{
				if (Tag.tag == MovedChunkTypes[it] && (!LoadedResources[it].empty()))
				{
					WasFound = true;
					break;
				}
			}
			
			if (!WasFound)
				append_data_to_wad(OutWad.Ptr, Tag.tag, Tag.data, Tag.length, 0);
		}
		
		for (int it = 0; it<NumMovedTypes; it++)
		{
			int DataLen = LoadedResources[it].size();
			if (DataLen > 0)
				append_data_to_wad(OutWad.Ptr, MovedChunkTypes[it], &LoadedResources[it][0], DataLen, 0);
		}
		
		OutFile.AddWad(TrueLevel,OutWad);
	}
	
	// Write out the remaining resources
	
	// Set to before the first one
	int Indices[NumMovedTypes];
	bool Advanceable[NumMovedTypes];
	for (int it = 0; it<NumMovedTypes; it++)
	{
		bool CanAdvance = false;
		for (int i = 0; i<ResourceIDs[it].size(); i++)
		{
			if (NotYetUsed[it][i])
			{
				Indices[it] = i;
				CanAdvance = true;
				break;
			}
		}
		Advanceable[it] = CanAdvance;
	}
	
	// Now advance through the indices
	while(true)
	{	
		// Find the minimum of all the current-index values;
		// if none could be found, then quit;
		
		bool ValueFound = false;
		int16 MinID;
		for (int it = 0; it<NumMovedTypes; it++)
		{
			if (Advanceable[it])
			{
				int16 RsrcID = ResourceIDs[it][Indices[it]];
				
				if (ValueFound)
				{
					MinID = min(MinID,RsrcID);
				}
				else
				{
					MinID = RsrcID;
					ValueFound = true;
				}
			}
		}
		
		if (!ValueFound) break;
		
		// Load the resources to be chunked
		vector<uint8> LoadedResources[NumMovedTypes];
		WadContainer OutWad;
		
		for (int it = 0; it<NumMovedTypes; it++)
		{
			if (ResourceIDs[it][Indices[it]] == MinID)
			{
				LoadResourceIntoArray(it,MinID,InRes,LoadedResources[it]);
				// Kludge for forcing the program to read all the map file's resources
				InRes.Close();
				InFile.Spec.Open(InRes);
				
				int DataLen = LoadedResources[it].size();
				if (DataLen > 0)
					append_data_to_wad(OutWad.Ptr, MovedChunkTypes[it], &LoadedResources[it][0], DataLen, 0);
				
				// Advance to next one when done
				NotYetUsed[it][Indices[it]] = 0;
				bool CanAdvance = false;
				for (int i = Indices[it]+1; i<ResourceIDs[it].size(); i++)
				{
					if (NotYetUsed[it][i])
					{
						Indices[it] = i;
						CanAdvance = true;
						break;
					}
				}
				Advanceable[it] = CanAdvance;
			}
		}
		
		// Add the new wad if it contains anything
		
		OutFile.AddWad(MinID,OutWad);
	}
	
	OutFile.Finish();
}

void ExtractChunks()
{
	InFileData InFile;
	if (!InFile.Open("Move into resources from?")) return;
	
	// Compose a name for the unchunked level
	char Name[256];
	InFile.Spec.GetName(Name);
	
	strncat(Name," Unchunked",31);
	Name[31] = 0;
	
	OutFileData OutFile;
	
	if (!OutFile.Open("What unchunked file?",Name)) return;

	OpenedResourceFile OutRes;
	if (!OutFile.Spec.Open(OutRes,true)) return;
	
	for (int lvl=0; lvl<InFile.NumWads(); lvl++)
	{
		WadContainer InWad, OutWad;
		short TrueLevel = InFile.LevelIndices[lvl];
		InFile.GetWad(TrueLevel,InWad);
		
		// Load the already-existing chunks; be sure to replace those that have
		// corresponding resources
		for (int itg=0; itg<InWad.Ptr->tag_count; itg++)
		{
			tag_data& Tag = InWad.Ptr->tag_data[itg];
			
			bool WasMoved = false;
			for (int it = 0; it<NumMovedTypes; it++)
			{
				if (Tag.tag == MovedChunkTypes[it])
				{
					// Move the chunk into the resource fork
					WasMoved = LoadDataIntoResource(it,Tag.data,Tag.length,OutRes,TrueLevel);
					break;
				}
			}
			
			if (!WasMoved)
				append_data_to_wad(OutWad.Ptr, Tag.tag, Tag.data, Tag.length, 0);
		}
		
		OutFile.AddWad(TrueLevel,OutWad);
	}
	
	OutFile.Finish();	
}
