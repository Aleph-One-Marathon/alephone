#ifndef _FILE_HANDLER_MACINTOSH_
#define _FILE_HANDLER_MACINTOSH_
/*
	
	Macintosh versions of file-handler class
	by Loren Petrich,
	August 11, 2000
*/

#include "FileHandler.h"

/*
	Abstraction for opened files; it does reading, writing, and closing of such files,
	without doing anything to the files' specifications
*/
class OpenedFile_Mac: public OpenedFile
{
	enum {RefNumClosed = -1};

	// This class will need to set the refnum and error value appropriately 
	friend class FileObject_Mac;
	
	short RefNum;
	short Err;
public:

	short GetError() {return Err;}
	
	bool IsOpen() {return (RefNum != RefNumClosed);}
	bool Close();
	
	bool GetPosition(long& Position);
	bool SetPosition(long Position);
	
	bool GetLength(long& Length);
	bool SetLength(long Length);
	
	bool Read(long Count, void *Buffer);
	bool Write(long Count, void *Buffer);
	
	// Smart macros for more easy reading of structured objects
	
	template<class T> bool ReadObject(T& Object)
		{return Read(sizeof(T),&Object);}
	
	template<class T> bool ReadObjectList(int NumObjects, T* ObjectList)
		{return Read(NumObjects*sizeof(T),ObjectList);}
	
	template<class T> bool WriteObject(T& Object)
		{return Write(sizeof(T),&Object);}
	
	template<class T> bool WriteObjectList(int NumObjects, T* ObjectList)
		{return Write(NumObjects*sizeof(T),ObjectList);}
	
	// Set the file to initially closed
	OpenedFile_Mac(): RefNum(RefNumClosed), Err(noErr) {}
};


class FileObject_Mac: public FileObject
{
public:
	// Moved this out here for some stuff that wants the refnum
	enum {RefNumClosed = -1};
	short RefNum;
	
	// Will leave this part exposed
	FSSpec Spec;
	OSErr Err;

	// The name as a C string:
	// assumes enough space to hold it if getting (max. 256 bytes)
	// The typecode is for automatically adding a suffix;
	// NONE means add none
		
	void GetName(char *Name);
	void SetName(char *Name, int Type);
	
	// Filespec management
	
	void SetSpec(FSSpec& _Spec);
	
	// Parent directories:
		
	bool SetFileToApp();
	bool SetParentToPreferences();
	
	// Partially inspired by portable_files.h:
	
	// These functions take an appropriate one of the typecodes used earlier;
	// this is to try to cover the cases of both typecode attributes
	// and typecode suffixes.
	bool Create(int Type);
	bool Open(bool Writable = false);
	
	// Added here: methods for opening Macintosh data and resource forks
	bool Open(OpenedFile& OFile, bool Writable=false);
	bool Open_MacData(short& refnum, bool Writable=false);
	bool Open_MacRsrc(short& refnum, bool Writable=false);
	
	// These calls are for creating dialog boxes to set the filespec
	// A null pointer means an empty string
	bool ReadDialog(int Type, char *Prompt=NULL);
	bool WriteDialog(int Type, char *Prompt=NULL, char *DefaultName=NULL);
	
	// Check on whether a file exists, is open, and its type
	bool Exists();
	bool IsOpen() {return (RefNum != RefNumClosed);}
	
	// Returns NONE if the type could not be identified
	int GetType();
	
	bool Close();
	
	bool GetPosition(long& Position);
	bool SetPosition(long Position);
	
	bool GetLength(long& Length);
	bool SetLength(long Length);
	
	bool Read(long Count, void *Buffer);
	bool Write(long Count, void *Buffer);
	
	// How many bytes are free in the disk that the file lives in?
	bool GetFreeSpace(unsigned long& FreeSpace);
	
	// Copying: either copy the filespec or copy the whole file
	// into the current file object
	bool CopySpec(FileObject& File);
	bool CopyContents(FileObject& File);
	
	bool Delete();
	
	// Sets the "file not open" error if the file is not open
	bool IsOpenCheck() {if (IsOpen()) return true; Err = fnOpnErr; return false;}
	
	FileObject_Mac(): RefNum(RefNumClosed), Err(noErr) {}
};

// Extract a FSSpec from a base-class object
inline FSSpec& GetSpec(FileObject &F) {return ((FileObject_Mac *)(&F))->Spec;}

#endif
