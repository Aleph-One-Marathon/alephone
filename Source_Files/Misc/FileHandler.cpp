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
#include "tags.h"
#include "FileHandler.h"
#include "MoreFilesExtract.h"


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
	

bool OpenedResourceFile::Get(OSType Type, short ID, LoadedResource& Rsrc)
{
	Rsrc.Unload();
	
	Push();
	
	Handle RsrcHandle = Get1Resource(Type,ID);
	Err = ResError();
	if (!RsrcHandle) return false;
	if (Err != noErr)
	{
		DisposeHandle(RsrcHandle);
		return false;
	}
	Rsrc.RsrcHandle = RsrcHandle;
	
	Pop();
	return (Err == noErr);
}

bool OpenedResourceFile::Close()
{
	if (!IsOpen()) return true;
	
	CloseResFile(RefNum);
	Err = ResError();
	RefNum = RefNum_Closed;
	
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
inline bool InRange(int Type) {return (Type >= 0 && Type < FileSpecifier::C_NUMBER_OF_TYPECODES);}


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
	memcpy(&Spec,&_Spec,sizeof(FSSpec));
}

// Parent directories:

bool FileSpecifier::SetFileToApp()
{
	get_my_fsspec(&Spec);
	
	return (Err == noErr);
}

bool FileSpecifier::SetParentToPreferences()
{
	Err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		&Spec.vRefNum, &Spec.parID);

	return (Err == noErr);
}

// Much of the content taken from portable_files.c:

bool FileSpecifier::Create(int Type)
{
	OSType TypeCode = InRange(Type) ? get_typecode(Type) : '????';
	OSType CreatorCode = get_typecode(C_Creator);

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

bool FileSpecifier::Exists()
{
	// Test for the existence of a file by re-creating its FSSpec
	Err = FSMakeFSSpec(Spec.vRefNum, Spec.parID, Spec.name, &Spec);
	
	return (Err == noErr);
}

// Returns NONE if the type could not be identified
int FileSpecifier::GetType()
{
	FInfo FileInfo;
	Err = FSpGetFInfo(&Spec,&FileInfo);
	if (Err != noErr) return NONE;
	
	OSType MacType = FileInfo.fdType;
	
	for (int k=0; k<C_NUMBER_OF_TYPECODES; k++)
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

	memset(&parms, 0, sizeof(HParamBlockRec));	
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

bool FileSpecifier::CopySpec(FileSpecifier& File)
{
	// Cast to the Macintosh version; how does RTTI work?
	FileSpecifier *FPtr = &File;
	FileSpecifier& MacFile = *((FileSpecifier *)FPtr);
	
	SetSpec(MacFile.Spec);
	return true;
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

