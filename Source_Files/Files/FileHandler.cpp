/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
	Implementations of Macintosh versions of file-handler class
	by Loren Petrich,
	August 11, 2000

[From macintosh_files.c, the source of much of this code]
	Tuesday, August 29, 1995 3:18:54 PM- rdm created.

Feb 3, 2000 (Loren Petrich):
	Suppressed "dprintf" in open_file_for_reading() and open_file_for_writing(),
		since file-opening errors are handled without any need for
		debug interrupts.

Nov 5, 2000 (Loren Petrich):
	Added initializer of RsrcHandle to LoadedResource constructor.

Feb 15, 2001 (Loren Petrich):
	Added event flushing for saving, so that using "return" as the action key
	does not cause unwanted saves.
	
Jun 13, 2001 (Loren Petrich):
	The write dialogs are designed to be used with safe saves,
	so they will not delete the previous files

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Added accessors for datafields now opaque in Carbon
	Standard File Dialogs now assert under carbon

March 18, 2002 (Br'fin (Jeremy Parsons)):
	Added FileSpecifier::SetParentToResources for Carbon
*/

#if defined(mac) || ( defined(SDL) && defined(SDL_RFORK_HACK) )
#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
/*
#else
#include <string.h>
#include <Aliases.h>
#include <Folders.h>
#include <Navigation.h>
*/
#endif
#include <algorithm>
#include "cseries.h"
#include "game_errors.h"
#include "shell.h"
#include "FileHandler.h"

// Parses a filespec or a directory spec with a path.
// Args:
// Name: filename or directory name with a path
// Spec:
//   In: FSSpec record containing original directory's vRefNum and parID
//   Out: FSSpec record of file, or vRefNum and parID of directory
// WantDirectory: whether one wants a directory instead of a file
// Returns whether the function operation had been successful
// The path separator is '/' the Unix/URL convention, instead of MacOS ':'
static bool ParsePath_MacOS(const char *NameWithPath, FSSpec &Spec, bool WantDirectory);

// Root-directory stuff; one needs to know whether it had been set.
// Alternative approach: have a subclass of DirectorySpecifier with
// a constructor that sets it to the app's directory.
// Not sure how that will coexist with MacOS init, however.
static bool RootDirectorySet = false;
static DirectorySpecifier RootDirectory;


/*
	Abstraction for opened files; it does reading, writing, and closing of such files,
	without doing anything to the files' specifications
*/

OpenedFile::OpenedFile() : RefNum(RefNum_Closed), Err(noErr) {}	// Set the file to initially closed

bool OpenedFile::IsOpen()
{
	return (RefNum != RefNum_Closed);
}

bool OpenedFile::Close()
{
	if (!IsOpen()) return true;
	
	Err = FSClose(RefNum);
	RefNum = RefNum_Closed;
	
	return (Err == noErr);
}

bool OpenedFile::GetPosition(long& Position)
{
	Err = GetFPos(RefNum, &Position);	
	
	return (Err == noErr);
}

bool OpenedFile::SetPosition(long Position)
{
	Err = SetFPos(RefNum, fsFromStart, Position);	
	
	return (Err == noErr);
}

bool OpenedFile::GetLength(long& Length)
{
	Err = GetEOF(RefNum, &Length);

	return (Err == noErr);
}

bool OpenedFile::SetLength(long Length)
{
	Err = SetEOF(RefNum, Length);
	
	return (Err == noErr);
}

bool OpenedFile::Read(long Count, void *Buffer)
{
	long _Count = Count;
	Err = FSRead(RefNum, &_Count, Buffer);
	
	return ((Err == noErr) && (_Count == Count));
}

bool OpenedFile::Write(long Count, void *Buffer)
{
	long _Count = Count;
	Err = FSWrite(RefNum, &_Count, Buffer);
	
	return ((Err == noErr) && (_Count == Count));
}

/*
    Abstraction for loaded resources:
	this object will release that resource when it finishes.
	MacOS resource handles will be assumed to be locked.
*/

LoadedResource::LoadedResource(): RsrcHandle(NULL)
{
}

bool LoadedResource::IsLoaded()
{
	return (RsrcHandle != NULL);
}

void LoadedResource::Unload()
{
	if (RsrcHandle)
	{
		HUnlock(RsrcHandle);
		ReleaseResource(RsrcHandle);
		RsrcHandle = NULL;
	}
}

size_t LoadedResource::GetLength()
{
	return (RsrcHandle) ? GetHandleSize(RsrcHandle) : 0;
}

void *LoadedResource::GetPointer(bool DoDetach)
{
	if (RsrcHandle)
	{
		void *ret = *RsrcHandle;
		if (DoDetach) Detach();
		return ret;
	} else
		return NULL;
}

void LoadedResource::SetData(void *data, size_t length)
{
	Unload();
	PtrToHand(data, &RsrcHandle, length);
	free(data);
}

void LoadedResource::Detach()
{
	if (RsrcHandle)
	{
		HUnlock(RsrcHandle);
		RsrcHandle = NULL;
	}
}


/*
	Abstraction for opened resource files:
	it does opening, setting, and closing of such files;
	also getting "LoadedResource" objects that return pointers
*/

OpenedResourceFile::OpenedResourceFile() : RefNum(RefNum_Closed), Err(noErr), SavedRefNum(RefNum_Closed) {}

// MacOS resource files are globally open; push and pop the current top one
bool OpenedResourceFile::Push()
{
	SavedRefNum = CurResFile();
	if (RefNum != SavedRefNum)
	{
		UseResFile(RefNum);
		Err = ResError();
		return (Err == noErr);
	}
	return true;
}

bool OpenedResourceFile::Pop()
{
	if (RefNum != SavedRefNum)
	{
		UseResFile(SavedRefNum);
		Err = ResError();
		return (Err == noErr);
	}
	return true;
}

bool OpenedResourceFile::Check(uint32 Type, int16 ID)
{
	Push();
	
	SetResLoad(false);
	Handle RsrcHandle = Get1Resource(Type,ID);
	Err = ResError();
	
	bool RsrcPresent = (RsrcHandle != NULL);
	if (RsrcPresent)
	{
		ReleaseResource(RsrcHandle);
		RsrcHandle = NULL;
		RsrcPresent = (Err == noErr);
	}
	
	SetResLoad(true);
	
	Pop();
	return RsrcPresent;
}

bool OpenedResourceFile::Get(uint32 Type, int16 ID, LoadedResource& Rsrc)
{
	Rsrc.Unload();
	
	Push();
	
	SetResLoad(true);
	Handle RsrcHandle = Get1Resource(Type,ID);
	Err = ResError();
	
	bool RsrcLoaded = (RsrcHandle != NULL);
	if (RsrcLoaded)
	{
		if (Err == noErr)
			Rsrc.RsrcHandle = RsrcHandle;
		else
		{
			ReleaseResource(RsrcHandle);
			RsrcLoaded = false;
		}
	}
	
	Pop();
	return RsrcLoaded;
}

bool OpenedResourceFile::GetTypeList(vector<uint32>& TypeList)
{
	TypeList.clear();
	Push();
	
	// How many?
	short NumTypes = Count1Types();
	if (NumTypes <= 0)
	{
		Pop();
		return false;
	}
	
	TypeList.resize(NumTypes);
	
	// Get the type ID's
	
	for (int it=0; it<NumTypes; it++)
		Get1IndType(&TypeList[it],it+1);	// 0-based to 1-based
	
	// Sort them!
	sort(TypeList.begin(),TypeList.end());
	
	Pop();
	return true;
}

bool OpenedResourceFile::GetIDList(uint32 Type, vector<int16>& IDList)
{
	IDList.clear();
	Push();
	
	// How many?
	short NumResources = Count1Resources(Type);
	if (NumResources <= 0)
	{
		Pop();
		return false;
	}
	
	IDList.resize(NumResources);

	// Get the resource ID's
	
	SetResLoad(false);
	
	for (int ir=0; ir<NumResources; ir++)
	{
		// Zero-based to one-based indexing
		Handle ResourceHandle = Get1IndResource(Type,ir+1);	// 0-based to 1-based
		ResType _Type;
		Str255 Name;
		GetResInfo(ResourceHandle,&IDList[ir],&_Type,Name);
		ReleaseResource(ResourceHandle);
	}
	
	SetResLoad(true);
	
	// Sort them!
	sort(IDList.begin(),IDList.end());
	
	Pop();
	return true;
}

bool OpenedResourceFile::IsOpen()
{
	return (RefNum != RefNum_Closed);
}

bool OpenedResourceFile::Close()
{
	if (!IsOpen()) return true;
	
	CloseResFile(RefNum);
	Err = ResError();
	RefNum = RefNum_Closed;
	
	return (Err == noErr);
}


// Can go to subdirectory of subdirectory;
// uses Unix-style path syntax (/ instead of : as the separator)
bool DirectorySpecifier::SetToSubdirectory(const char *NameWithPath)
{
	Files_GetRootDirectory(*this);
	
	FSSpec Spec;
	Spec.vRefNum = vRefNum;
	Spec.parID = parID;
	if (!ParsePath_MacOS(NameWithPath, Spec, true)) return false;
	
	vRefNum = Spec.vRefNum;
	parID = Spec.parID;
	return true;
}


bool DirectorySpecifier::SetToAppParent()
{
	FileSpecifier F;

	F.SetToApp();	
	F.ToDirectory(*this);
	
	Err = F.GetError();
	return (Err == noErr);
}

bool DirectorySpecifier::SetToPreferencesParent()
{
	Err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		&vRefNum, &parID);
	
	return (Err == noErr);
}


// Useful for scenarios -- submit the subdirectory string.
// Originally the app's parent directory. To revert to that, submit an empty string.
// Returns the level of success.
bool Files_SetRootDirectory(const char *NameWithPath)
{
	if (strlen(NameWithPath) > 0)
	{
		// Necessary because Files_GetRootDirectory() will be called in SetToSubdirectory,
		// and we want to tell it to refer to the app parent.
		RootDirectorySet = false;
		
		RootDirectorySet = RootDirectory.SetToSubdirectory(NameWithPath);
	}
	else
	{
		RootDirectorySet = false;
	}
	return true;
}

void Files_GetRootDirectory(DirectorySpecifier& Dir)
{
	if (RootDirectorySet)
		Dir = RootDirectory;
	else
		Dir.SetToAppParent();
}

	
// The name as a C string:
// assumes enough space to hold it if getting (max. 256 bytes)

inline void CString_ToFilename(const char *Source, unsigned char *Destination)
{
	// Null pointer -> zero-length string
	if (!Source)
	{
		Destination[0] = 0;
		return;
	}
	int Len = MIN(strlen(Source),31);
	Destination[0] = Len;
	memcpy(Destination+1,Source,Len);
}
inline void MacFilename_To_CString(const unsigned char *Source, char *Destination)
{
	// Null pointer -> zero-length string
	if (!Source)
	{
		Destination[0] = 0;
		return;
	}
	int Len = Source[0];
	memcpy(Destination,Source+1,Len);
	Destination[Len] = 0;
}

// Typecode in range
inline bool InRange(int Type) {return (Type >= 0 && Type < NUMBER_OF_TYPECODES);}


// FileSpecifier constructor moved here so as to blank out the FSSpec
FileSpecifier::FileSpecifier(): Err(noErr) {obj_clear(Spec);}

void FileSpecifier::GetName(char *Name) const
{
	MacFilename_To_CString(Spec.name,Name);
}
void FileSpecifier::SetName(const char *Name, int Type)
{
	CString_ToFilename(Name,Spec.name);
}


// Parses the directory path and updates the parent directory appropriately
bool FileSpecifier:: SetNameWithPath(const char *NameWithPath)
{
	DirectorySpecifier Dir;
	Files_GetRootDirectory(Dir);
	
	Spec.vRefNum = Dir.vRefNum;
	Spec.parID = Dir.parID;
	
	return ParsePath_MacOS(NameWithPath, Spec, false);
}


// Filespec management:

const FileSpecifier &FileSpecifier::operator=(const FileSpecifier &other)
{
	if (this != &other)
		obj_copy(Spec,other.Spec);
	return *this;
}

void FileSpecifier::SetSpec(FSSpec& _Spec)
{
	obj_copy(Spec,_Spec);
}

// Parent directories:

void FileSpecifier::ToDirectory(DirectorySpecifier& Dir)
{
	Dir.vRefNum = Spec.vRefNum;
	Dir.parID = Spec.parID;
}

void FileSpecifier::FromDirectory(DirectorySpecifier& Dir)
{
	Spec.vRefNum = Dir.vRefNum;
	Spec.parID = Dir.parID;
}

bool FileSpecifier::SetToApp()
{
	get_my_fsspec(&Spec);
	
	return (Err == noErr);
}

#if defined(TARGET_API_MAC_CARBON)
bool FileSpecifier::SetParentToResources()
{
	FSSpec appSpec;
	char name[256];
	OSStatus err;
	
	get_my_fsspec(&appSpec);
	CopyPascalStringToC(appSpec.name, temporary);
	sprintf(name, ":%s:Contents:Resources:AlephOne.rsrc", temporary);
	CopyCStringToPascal(name, ptemporary);
	
	err = FSMakeFSSpec(appSpec.vRefNum, appSpec.parID, ptemporary, &Spec);
	return (err == noErr);
}
#endif

bool FileSpecifier::SetParentToPreferences()
{
	DirectorySpecifier D;
	
	D.SetToPreferencesParent();
	FromDirectory(D);
	
	Err = D.GetError();
	
	return (Err == noErr);
}

// Much of the content taken from portable_files.c:

bool FileSpecifier::Create(Typecode Type)
{
	OSType TypeCode = InRange(Type) ? get_typecode(Type) : '????';
	OSType CreatorCode = get_typecode(_typecode_creator);
	return Create(TypeCode,CreatorCode);
}

bool FileSpecifier::Create(OSType TypeCode, OSType CreatorCode)
{
	/* Assume that they already confirmed this is going to die.. */
	Err = FSpDelete(&Spec);

	if (Err == noErr || Err == fnfErr)
	{
		Err= FSpCreate(&Spec, CreatorCode, TypeCode, smSystemScript);
		// No resource forks to be created unless necessary -- they are MacOS-specific
	}

	return (Err == noErr);
}

// For resolving MacOS aliases:
inline bool ResolveFile(FSSpec &Spec)
{
	Boolean IsFolder = false, WasAliased = false;
	ResolveAliasFile(&Spec, true, &IsFolder, &WasAliased);
	return (!IsFolder);
}

// Sets the MacOS permission value (either read/write or plain read)
inline int WhatPermission(bool Writable) {return Writable ? fsRdWrPerm : fsRdPerm;}


// The main functions:

bool FileSpecifier::Open(OpenedFile& OFile, bool Writable)
{
	OFile.Close();
	
	if (!ResolveFile(Spec)) return false;
	short RefNum;
	Err = FSpOpenDF(&Spec, WhatPermission(Writable), &RefNum);
	if (Err != noErr) 
	{
		RefNum = RefNum_Closed;
		set_game_error(systemError, Err);
		return false;
	}
		
	OFile.RefNum = RefNum;
	OFile.Err = noErr;

	return true;
}

bool FileSpecifier::Open(OpenedResourceFile& OFile, bool Writable)
{
	OFile.Close();
	Spec.name[Spec.name[0]+1] = 0;
	
	if (!ResolveFile(Spec)) return false;
        short RefNum;
	RefNum = FSpOpenResFile(&Spec, WhatPermission(Writable));
	Err = ResError();
	if (Err != noErr) 
	{
		RefNum = RefNum_Closed;
		set_game_error(systemError, Err);
		return false;
	}
	
	OFile.RefNum = RefNum;
	OFile.Err = noErr;
	
	return true;
}

// Navigation Services extra routines

// LP: AlexJLS's Nav Services code:
pascal void NavIdler(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUD)
{
	global_idle_proc();
}

// LP: AlexJLS's Nav Services code, somewhat modified
//Function stolen from MPFileCopy
static OSStatus ExtractSingleItem(const NavReplyRecord *reply, FSSpec *item)
	// This item extracts a single FSRef from a NavReplyRecord.
	// Nav makes it really easy to support 'odoc', but a real pain
	// to support other things.  *sigh*
{
	OSStatus err;
	FSSpec fss, fss2;
	AEKeyword junkKeyword;
	DescType junkType;
	Size junkSize;

	obj_clear(fss);
	obj_clear(fss2);
	
	err = AEGetNthPtr(&reply->selection, 1, typeFSS, &junkKeyword, &junkType, &fss, sizeof(fss), &junkSize);
	if (err == noErr) {
		//MoreAssertQ(junkType == typeFSS);
		//MoreAssertQ(junkSize == sizeof(FSSpec));
		
		// We call FSMakeFSSpec because sometimes Nav is braindead
		// and gives us an invalid FSSpec (where the name is empty).
		// While FSpMakeFSRef seems to handle that (and the file system
		// engineers assure me that that will keep working (at least
		// on traditional Mac OS) because of the potential for breaking
		// existing applications), I'm still wary of doing this so
		// I regularise the FSSpec by feeding it through FSMakeFSSpec.
		
		FSMakeFSSpec(fss.vRefNum,fss.parID,fss.name,item);
		/*if (err == noErr) {
			err = FSpMakeFSRef(&fss, item);
		}*/
	}
	return err;
}

// These calls are for creating dialog boxes to set the filespec
// A null pointer means an empty string

bool FileSpecifier::ReadDialog(Typecode Type, char *Prompt)
{
	// For those who use return as the action key, queued returns can cause unwanted saves
	FlushEvents(everyEvent,0);
	
	if (machine_has_nav_services())
	{
	
	// LP: AlexJLS's Nav Services code, somewhat modified
	NavTypeListHandle list= NULL;
	if (InRange(Type))
	{
		list= (NavTypeListHandle)NewHandleClear(sizeof(NavTypeList));
		HLock((Handle)list);
		(**list).componentSignature = get_typecode(_typecode_creator);
		(**list).osTypeCount = 1;
		(**list).osType[0] = get_typecode(Type);
	}
	
	NavDialogOptions opts;
	NavGetDefaultDialogOptions(&opts);
	if (Prompt)
		CString_ToFilename(Prompt,opts.message);
	opts.dialogOptionFlags = kNavNoTypePopup | kNavAllowPreviews;
	
	NavReplyRecord reply;
	NavEventUPP evUPP= NewNavEventUPP(NavIdler);
	NavGetFile(NULL,&reply,&opts,evUPP,NULL,NULL,list,NULL);
	DisposeNavEventUPP(evUPP);
	if (list) DisposeHandle((Handle)list);
		
	if (!reply.validRecord) return false;
	
	FSSpec temp;
	obj_clear(temp);
	ExtractSingleItem(&reply,&temp);
	SetSpec(temp);
	
	} else {
	
//#if defined(SUPPRESS_MACOS_CLASSIC)
	// No Standard File under MacOS X
	assert(0);
/*
#else
	StandardFileReply Reply;
	short NumTypes;
	SFTypeList TypeList;
	if (InRange(Type))
	{
		NumTypes = 1;
		TypeList[0] = get_typecode(Type);
	}
	else
		NumTypes = -1;
	
	StandardGetFile(NULL, NumTypes, TypeList, &Reply);
	if (!Reply.sfGood) return false;
	
	SetSpec(Reply.sfFile);
	
#endif
*/
	}
	
	return true;
}

bool FileSpecifier::WriteDialog(Typecode Type, char *Prompt, char *DefaultName)
{
	// For those who use return as the action key, queued returns can cause unwanted saves
	FlushEvents(everyEvent,0);
	
	if (machine_has_nav_services())
	{
	
	// LP: AlexJLS's Nav Services code, somewhat modified
	NavDialogOptions opts;
	NavGetDefaultDialogOptions(&opts);
	if (Prompt)
		CString_ToFilename(Prompt,opts.message);
	if (DefaultName)
		CString_ToFilename(DefaultName,opts.savedFileName);
	opts.dialogOptionFlags &= ~kNavAllowStationery;
	
	NavReplyRecord reply;
	NavEventUPP evUPP= NewNavEventUPP(NavIdler);
	NavPutFile(NULL,&reply,&opts,evUPP,get_typecode(Type),get_typecode(_typecode_creator),NULL);
	DisposeNavEventUPP(evUPP);
	
	if (!reply.validRecord) return false;
	
	FSSpec temp;
	obj_clear(temp);
	ExtractSingleItem(&reply,&temp);
	SetSpec(temp);
	
	} else {
	
//#if defined(SUPPRESS_MACOS_CLASSIC)
	assert(0);
/*
#else
	Str31 PasPrompt, PasDefaultName;
	StandardFileReply Reply;
	
	if (Prompt)
		CString_ToFilename(Prompt,PasPrompt);
	else
		PasPrompt[0] = 0;
	if (DefaultName)
		CString_ToFilename(DefaultName,PasDefaultName);
	else
		PasDefaultName[0] = 0;
	
	StandardPutFile(PasPrompt, PasDefaultName, &Reply);
	if (!Reply.sfGood) return false;
	
	SetSpec(Reply.sfFile);
#endif	
*/	
	}
	
	return true;
}

#if 0
// Stuff for the asynchronous save dialog for savegames...
#define kFolderBit 0x10
#define strSAVE_LEVEL_NAME 128
#define dlogMY_REPLACE 131

#define REPLACE_H_OFFSET 52
#define REPLACE_V_OFFSET 52

/* Doesn't actually use it as an FSSpec, just easier */
/* Return true if we want to pass it through, otherwise return false. */
static bool confirm_save_choice(
	FSSpec *file)
{
	OSErr err;
	HFileParam pb;
	bool pass_through= true;
	DialogPtr dialog;
	short item_hit;
	Rect frame;

	/* Clear! */
	obj_clear(pb);
	pb.ioNamePtr= file->name;
	pb.ioVRefNum= file->vRefNum;
	pb.ioDirID= file->parID;
	err= PBHGetFInfo((HParmBlkPtr) &pb, false); 

	if(!err)
	{
		/* IF we aren't a folder.. */
		if(!(pb.ioFlAttrib & kFolderBit))
		{
			if(pb.ioFlFndrInfo.fdType==get_typecode(_typecode_savegame))
			{
				/* Get the default dialog's frame.. */
				get_window_frame(FrontWindow(), &frame);

				/* Slam the ParamText */
				ParamText(file->name, "\p", "\p", "\p");

				/* Load in the dialog.. */
				dialog= myGetNewDialog(dlogMY_REPLACE, NULL, (WindowPtr) -1, 0);
				assert(dialog);
				
				/* Move the window to the proper location.. */
//#if defined(USE_CARBON_ACCESSORS)                                
				MoveWindow(GetDialogWindow(dialog), frame.left+REPLACE_H_OFFSET, 
					frame.top+REPLACE_V_OFFSET, false);
/*
#else
				MoveWindow((WindowPtr) dialog, frame.left+REPLACE_H_OFFSET, 
					frame.top+REPLACE_V_OFFSET, false);
#endif
*/

				/* Show the window. */
//#if defined(USE_CARBON_ACCESSORS)
				ShowWindow(GetDialogWindow(dialog));			
/*
#else
				ShowWindow((WindowPtr) dialog);			
#endif
*/
				do {
					ModalDialog(get_general_filter_upp(), &item_hit);
				} while(item_hit > iCANCEL);

				/* Restore and cleanup.. */				
				ParamText("\p", "\p", "\p", "\p");
				DisposeDialog(dialog);
				
				if(item_hit==iOK) /* replace.. */
				{
					/* Want to delete it... */
					// err= FSpDelete(file);
					/* Pass it on through.. they won't bring up the replace now. */
				} else {
					/* They cancelled.. */
					pass_through= false;
				}
			}
		}
	}
	
	return pass_through;
}
#endif

/*
#if !defined(SUPPRESS_MACOS_CLASSIC)
*/
/* load_file_reply is valid.. */
/*
static pascal short custom_put_hook(
	short item, 
	DialogPtr theDialog,
	void *user_data)
{
	FSSpec *selected_file= (FSSpec *) user_data;

	global_idle_proc();

	if(GetWRefCon((WindowPtr) theDialog)==sfMainDialogRefCon)
	{
		if(item==sfItemOpenButton)
		{
			if(!confirm_save_choice(selected_file))
			{
				item= sfHookNullEvent;
			}
		}
	}
	
	return item;
}
#endif
*/

bool FileSpecifier::WriteDialogAsync(Typecode Type, char *Prompt, char *DefaultName)
{
	// For those who use return as the action key, queued returns can cause unwanted saves
	FlushEvents(everyEvent,0);
	
	if (machine_has_nav_services())
	{
	
	// LP: AlexJLS's Nav Services code, somewhat modified
	NavDialogOptions opts;
	NavGetDefaultDialogOptions(&opts);
	if (Prompt)
		CString_ToFilename(Prompt,opts.message);
	if (DefaultName)
		CString_ToFilename(DefaultName,opts.savedFileName);
	opts.dialogOptionFlags &= ~kNavAllowStationery;
	
	NavReplyRecord reply;
	NavEventUPP evUPP= NewNavEventUPP(NavIdler);
	NavPutFile(NULL,&reply,&opts,evUPP,get_typecode(Type),get_typecode(_typecode_creator),NULL);
	DisposeNavEventUPP(evUPP);
	
	if (!reply.validRecord) return false;
	
	FSSpec temp;
	obj_clear(temp);
	ExtractSingleItem(&reply,&temp);
	SetSpec(temp);
	
	} else {
//#if defined(SUPPRESS_MACOS_CLASSIC)
	assert(0);
/*
#else
	
	Str31 PasPrompt, PasDefaultName;
	StandardFileReply Reply;
	if (Prompt)
		CString_ToFilename(Prompt,PasPrompt);
	else
		PasPrompt[0] = 0;
	if (DefaultName)
		CString_ToFilename(DefaultName,PasDefaultName);
	else
		PasDefaultName[0] = 0;
	
	DlgHookYDUPP dlgHook;
	Point top_left= {-1, -1}; *//* auto center *//*
	
	*//* Create the UPP's *//*
	dlgHook= NewDlgHookYDProc(custom_put_hook);
	assert(dlgHook);
	
	*//* The drawback of this method-> I don't get a New Folder button. *//*
	*//* If this is terribly annoying, I will add the Sys7 only code. *//*
	CustomPutFile(PasPrompt, 
		PasDefaultName, &Reply, 0, top_left, dlgHook, NULL, NULL, NULL, &Reply.sfFile);

	*//* Free them... *//*
	DisposeRoutineDescriptor((UniversalProcPtr) dlgHook);
	
	if (!Reply.sfGood) return false;
	
	SetSpec(Reply.sfFile);
#endif
*/
	}
	
	return true;
}

bool FileSpecifier::Exists()
{
	// Test for the existence of a file by re-creating its FSSpec
	Err = FSMakeFSSpec(Spec.vRefNum, Spec.parID, Spec.name, &Spec);
	
	return (Err == noErr);
}

// Code from get_file_modification_date() in preferences.c
TimeType FileSpecifier::GetDate()
{
	CInfoPBRec pb;
	OSErr error;
	TimeType modification_date= 0;
	
	obj_clear(pb);
	pb.hFileInfo.ioVRefNum= Spec.vRefNum;
	pb.hFileInfo.ioNamePtr= Spec.name;
	pb.hFileInfo.ioDirID= Spec.parID;
	pb.hFileInfo.ioFDirIndex= 0;
	error= PBGetCatInfoSync(&pb);
	if(!error)
	{
		modification_date= pb.hFileInfo.ioFlMdDat;
	}
	
	return modification_date;
}

// Returns _typecode_unknown if the type could not be identified
Typecode FileSpecifier::GetType()
{
	FInfo FileInfo;
	Err = FSpGetFInfo(&Spec,&FileInfo);
	if (Err != noErr) return _typecode_unknown;
	
	OSType MacType = FileInfo.fdType;

        return get_typecode_for_file_type(MacType);
}
	
// How many bytes are free in the disk that the file lives in?
bool FileSpecifier::GetFreeSpace(unsigned long& FreeSpace)
{
	// Code cribbed from vbl_macintosh.c
	HParamBlockRec  parms;

	obj_clear(parms);	
	parms.volumeParam.ioCompletion = NULL;
	parms.volumeParam.ioVolIndex = 0;
	parms.volumeParam.ioNamePtr = ptemporary;
	parms.volumeParam.ioVRefNum = Spec.vRefNum;
	
	Err = PBHGetVInfo(&parms, false);
	if (Err == noErr)
		FreeSpace = 
			((unsigned long) parms.volumeParam.ioVAlBlkSiz) *
				((unsigned long) parms.volumeParam.ioVFrBlk);
	
	return (Err==noErr);
}

bool FileSpecifier::CopyContents(FileSpecifier& File)
{	
	// Copied out of vbl_macintosh.c;
	// this code only copies the data fork
	const int COPY_BUFFER_SIZE = (3*1024);
	FSSpec *source = &File.Spec;
	FSSpec *ult_dest = &Spec;
	
	// Do a safe copy; keep the original if the copying had failed
	FileSpecifier TempFile;
	DirectorySpecifier TempFileDir;
	File.ToDirectory(TempFileDir);
	TempFile.FromDirectory(TempFileDir);
	TempFile.SetName("savetemp.dat",NONE);
	FSSpec *destination = &TempFile.GetSpec();
	
	FInfo info;
	
	Err= FSpGetFInfo(source, &info);
	if(Err==noErr)
	{
		Err= FSpCreate(destination, info.fdCreator, info.fdType, smSystemScript);
		if(Err==noErr)
		{
			short dest_refnum, source_refnum;
		
			Err= FSpOpenDF(destination, fsWrPerm, &dest_refnum);
			if(Err==noErr)
			{
				Err= FSpOpenDF(source, fsRdPerm, &source_refnum);
				if(Err==noErr)
				{
					/* Everything is opened. Do the deed.. */
					Ptr data;
					long total_length;
					
					SetFPos(source_refnum, fsFromLEOF, 0l);
					GetFPos(source_refnum, &total_length);
					SetFPos(source_refnum, fsFromStart, 0l);
					
					data= new char[COPY_BUFFER_SIZE];
					if(data)
					{
						long running_length= total_length;
						
						while(running_length && Err==noErr)
						{
							long count= MIN(COPY_BUFFER_SIZE, running_length);
						
							Err= FSRead(source_refnum, &count, data);
							if(Err==noErr)
							{
								Err= FSWrite(dest_refnum, &count, data);
							}
							running_length -= count;
						}
					
						delete []data;
					} else {
						Err= MemError();
					}
					
					FSClose(source_refnum);
				}

				FSClose(dest_refnum);
			}

			/* Delete it on an error */
			if(Err != noErr) FSpDelete(destination);
		}
	}
	
	// Create a placeholder file if the ultimate-destination file does not exist
	if (Err == noErr)
	{
		if (!Exists()) Err = FSpCreate(ult_dest, info.fdCreator, info.fdType, smSystemScript);
	}
	
	if (Err == noErr)
	{
		Exchange(TempFile);
		TempFile.Delete();
	}
	
	return (Err == noErr);
}


bool FileSpecifier::Exchange(FileSpecifier& File)
{
	Err = FSpExchangeFiles(&Spec,&File.GetSpec());
		
	return (Err == noErr);
}


bool FileSpecifier::Delete()
{
	Err = FSpDelete(&Spec);
	
	return (Err == noErr);
}

	
// Is this file specifier the same as some other one?
bool FileSpecifier::operator==(const FileSpecifier& F)
{
	// Copied out of find_files.c
	bool equal= false;
	
	if(Spec.vRefNum==F.Spec.vRefNum && Spec.parID==F.Spec.parID && 
		EqualString(Spec.name, F.Spec.name, false, false))
	{
		equal= true;
	}
	
	return equal;

}


bool ParsePath_MacOS(const char *NameWithPath, FSSpec &Spec, bool WantDirectory)
{
	// String setup
	int NNPChars = strlen(NameWithPath);
	int StrPos = 0;
	
	// File-info block and name of file found
	CInfoPBRec PB;
	Str31 FoundFileName;
	
	// Maintained separately, since updating this ought not to affect the FSSpec,
	// which will be left unchanged in case of failure.
	long ParentDir = Spec.parID;
	
	// Member of a path (either directory or file)
	Str31 PathMember;
	
	// Directory-recursion loop;
	while(StrPos < NNPChars)
	{
		// Pull characters off the input string until
		// either the directory separator or the string end is reached.
		// WantDirectory means that the last name in sequence ought to be a directory
		// instead of a file.
		bool IsDir = WantDirectory;
		
		int PathMemberLen = 0;
		while(StrPos < NNPChars)
		{
			// One character at a time
			char c = NameWithPath[StrPos++];
			
			// Hit a directory separator
			if (c == '/')
			{
				IsDir = true;
				break;
			}
			
			// Put it in
			if (PathMemberLen < 31)
			{
				if (c == ':') c = '/';	// Translate for the MacOS filesystem (separator is : instead of /)
				PathMember[++PathMemberLen] = c;
				PathMember[0] = PathMemberLen;
			}
		}
		
		// Empty directory: ignore; empty file: bomb out
		if (PathMember[0] == 0)
		{
			if (IsDir) continue;
			else return false;
		}
		
		// Resetting for each directory walk
		obj_clear(PB);
		PB.hFileInfo.ioVRefNum = Spec.vRefNum;
		PB.hFileInfo.ioNamePtr = FoundFileName;
		
		// Walk the directory, searching for a file with a matching name
		int FileIndex = 1;
		bool FileFound = false;
		while(true)
		{
			// Resetting to look for next file in directory
			PB.hFileInfo.ioDirID = ParentDir;
			PB.hFileInfo.ioFDirIndex = FileIndex;
			
			// Quit if ran out of files
			OSErr Err = PBGetCatInfo(&PB, false);
			if (Err != noErr) break;
			
			// Compare names
			bool NamesEqual = true;
			for (int i=0; i<=PathMember[0]; i++)
			{
				if (FoundFileName[i] != PathMember[i])
				{
					NamesEqual = false;
					break;
				}
			}
						
			if (NamesEqual)
			{
				FileFound = true;
				// Do the path and the PB agree on whether it's a directory or a file?
				if (PB.hFileInfo.ioFlAttrib & 0x10)
				{
					if (IsDir)
					{
						// Grab the new parent-directory ID
						ParentDir = PB.dirInfo.ioDrDirID;
						break;
					}
					else return false;
				}
				else
				{
					if (!IsDir)
					{
						// Found the file! Now set the FSSpec and exit triumphantly
						Spec.parID = ParentDir;
						memset(Spec.name,0,32);
						memcpy(Spec.name,FoundFileName,int(FoundFileName[0])+1);
						return true;
					}
					else return false;
				}
			}
			// Otherwise, continue to the next file
			FileIndex++;
		}
		if (!FileFound) return false;
	}
	
	// Arriving here is appropriate if a directory had been wanted,
	// though not if a file had been wanted; be sure to update "Spec" as appropriate
	if (WantDirectory)
	{
		Spec.parID = ParentDir;
		return true;
	}
	else
		return false;
}

#endif