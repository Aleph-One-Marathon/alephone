/*
	
	Implementations of Macintosh versions of file-handler class
	by Loren Petrich,
	August 11, 2000

[From macintosh_files.c, the source of much of this code]
	Tuesday, August 29, 1995 3:18:54 PM- rdm created.

Feb 3, 2000 (Loren Petrich):
	Suppressed "dprintf" in open_file_for_reading() and open_file_for_writing(),
		since file-opening errors are handled without any need for
		debug interrupts.
*/

#include <string.h>
#include <Aliases.h>
#include <Folders.h>
#include "cseries.h"
#include "game_errors.h"
#include "shell.h"
#include "FileHandler.h"


/*
	Abstraction for opened files; it does reading, writing, and closing of such files,
	without doing anything to the files' specifications
*/
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
	Abstraction for opened resource files:
	it does opening, setting, and closing of such files;
	also getting "LoadedResource" objects that return pointers
*/

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


bool OpenedResourceFile::Check(OSType Type, short ID)
{
	Push();
	
	SetResLoad(FALSE);
	Handle RsrcHandle = Get1Resource(Type,ID);
	Err = ResError();
	
	bool RsrcPresent = (RsrcHandle != NULL);
	if (RsrcPresent)
	{
		ReleaseResource(RsrcHandle);
		RsrcHandle = NULL;
		RsrcPresent = (Err == noErr);
	}
	
	SetResLoad(TRUE);
	
	Pop();
	return RsrcPresent;
}


bool OpenedResourceFile::Get(OSType Type, short ID, LoadedResource& Rsrc)
{
	Rsrc.Unload();
	
	Push();
	
	SetResLoad(TRUE);
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

bool OpenedResourceFile::Close()
{
	if (!IsOpen()) return true;
	
	CloseResFile(RefNum);
	Err = ResError();
	RefNum = RefNum_Closed;
	
	return (Err == noErr);
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


	
// The name as a C string:
// assumes enough space to hold it if getting (max. 256 bytes)

inline void CString_ToFilename(char *Source, unsigned char *Destination)
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
inline void MacFilename_To_CString(unsigned char *Source, char *Destination)
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


void FileSpecifier::GetName(char *Name)
{
	MacFilename_To_CString(Spec.name,Name);
}
void FileSpecifier::SetName(char *Name, int Type)
{
	CString_ToFilename(Name,Spec.name);
}

// Filespec management:

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

bool FileSpecifier::SetParentToPreferences()
{
	DirectorySpecifier D;
	
	D.SetToPreferencesParent();
	FromDirectory(D);
	
	Err = D.GetError();
	
	return (Err == noErr);
}

// Much of the content taken from portable_files.c:

bool FileSpecifier::Create(int Type)
{
	OSType TypeCode = InRange(Type) ? get_typecode(Type) : '????';
	OSType CreatorCode = get_typecode(_typecode_creator);

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
	Boolean IsFolder = FALSE, WasAliased = FALSE;
	ResolveAliasFile(&Spec, TRUE, &IsFolder, &WasAliased);
	return (!IsFolder);
}

// Sets the MacOS permission value (either read/write or plain read)
inline int WhatPermission(bool Writable) {return Writable ? fsRdWrPerm : fsRdPerm;}


// The main functions:

bool FileSpecifier::Open(OpenedFile& OFile, bool Writable)
{
	OFile.Close();
	
	if (!ResolveFile(Spec)) return false;
	
	Boolean IsFolder = FALSE, WasAliased = FALSE;
	ResolveAliasFile(&Spec, TRUE, &IsFolder, &WasAliased);
	if (IsFolder) return false;

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


// These calls are for creating dialog boxes to set the filespec
// A null pointer means an empty string

bool FileSpecifier::ReadDialog(int Type, char *Prompt)
{
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
	return true;
}

bool FileSpecifier::WriteDialog(int Type, char *Prompt, char *DefaultName)
{
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
	
	if (Reply.sfReplacing)
		FSpDelete(&Spec);
	
	return true;
}

// Stuff for the asynchronous save dialog for savegames...
#define kFolderBit 0x10
#define strSAVE_LEVEL_NAME 128
#define dlogMY_REPLACE 131

#define REPLACE_H_OFFSET 52
#define REPLACE_V_OFFSET 52

/* Doesn't actually use it as an FSSpec, just easier */
/* Return TRUE if we want to pass it through, otherwise return FALSE. */
static boolean confirm_save_choice(
	FSSpec *file)
{
	OSErr err;
	HFileParam pb;
	boolean pass_through= TRUE;
	DialogPtr dialog;
	short item_hit;
	Rect frame;

	/* Clear! */
	obj_clear(pb);
	pb.ioNamePtr= file->name;
	pb.ioVRefNum= file->vRefNum;
	pb.ioDirID= file->parID;
	err= PBHGetFInfo((HParmBlkPtr) &pb, FALSE); 

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
				MoveWindow((WindowPtr) dialog, frame.left+REPLACE_H_OFFSET, 
					frame.top+REPLACE_V_OFFSET, FALSE);

				/* Show the window. */
				ShowWindow((WindowPtr) dialog);			
				do {
					ModalDialog(get_general_filter_upp(), &item_hit);
				} while(item_hit > iCANCEL);

				/* Restore and cleanup.. */				
				ParamText("\p", "\p", "\p", "\p");
				DisposeDialog(dialog);
				
				if(item_hit==iOK) /* replace.. */
				{
					/* Want to delete it... */
					err= FSpDelete(file);
					/* Pass it on through.. they won't bring up the replace now. */
				} else {
					/* They cancelled.. */
					pass_through= FALSE;
				}
			}
		}
	}
	
	return pass_through;
}

/* load_file_reply is valid.. */
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

bool FileSpecifier::WriteDialogAsync(int Type, char *Prompt, char *DefaultName)
{
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
	Point top_left= {-1, -1}; /* auto center */
	
	/* Create the UPP's */
	dlgHook= NewDlgHookYDProc(custom_put_hook);
	assert(dlgHook);

	/* The drawback of this method-> I don't get a New Folder button. */
	/* If this is terribly annoying, I will add the Sys7 only code. */
	CustomPutFile(PasPrompt, 
		PasDefaultName, &Reply, 0, top_left, dlgHook, NULL, NULL, NULL, &Reply.sfFile);

	/* Free them... */
	DisposeRoutineDescriptor((UniversalProcPtr) dlgHook);
	
	if (!Reply.sfGood) return false;
	
	SetSpec(Reply.sfFile);
	
	if (Reply.sfReplacing)
		FSpDelete(&Spec);
	
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

// Returns NONE if the type could not be identified
int FileSpecifier::GetType()
{
	FInfo FileInfo;
	Err = FSpGetFInfo(&Spec,&FileInfo);
	if (Err != noErr) return NONE;
	
	OSType MacType = FileInfo.fdType;
	
	for (int k=0; k<NUMBER_OF_TYPECODES; k++)
	{
		if (MacType == get_typecode(k)) return k;
	}
	return NONE;
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
	
	Err = PBHGetVInfo(&parms, FALSE);
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
	FSSpec *destination = &Spec;
	
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
	return (Err == noErr);
}

bool FileSpecifier::Delete()
{
	Err = FSpDelete(&Spec);
	
	return (Err == noErr);
}

	
// Is this file specifier the same as some other one?
bool FileSpecifier::operator==(FileSpecifier& F)
{
	// Copied out of find_files.c
	Boolean equal= false;
	
	if(Spec.vRefNum==F.Spec.vRefNum && Spec.parID==F.Spec.parID && 
		EqualString(Spec.name, F.Spec.name, false, false))
	{
		equal= true;
	}
	
	return equal;

}

