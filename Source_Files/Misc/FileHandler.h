#ifndef _FILE_HANDLER_
#define _FILE_HANDLER_
/*
	File-handler classes
	by Loren Petrich,
	August 11, 2000

	These are designed to provide some abstract interfaces to file and directory objects.
	
	Most of these routines return whether they had succeeded;
	more detailed error codes are API-specific.
	Attempted to support stdio I/O directly, but on my Macintosh, at least,
	the performance was much poorer. This is possibly due to "fseek" having to
	actually read the file or something.
	
	Merged all the Macintosh-specific code into these base classes, so that
	it will be selected with a preprocessor statement when more than one file-I/O
	API is supported.
*/

// Symbolic constant for a closed file's reference number (refnum)
const short RefNum_Closed = -1;

/*
	Abstraction for opened files; it does reading, writing, and closing of such files,
	without doing anything to the files' specifications
*/
class OpenedFile
{
	// This class will need to set the refnum and error value appropriately 
	friend class FileSpecifier;
	
	// MacOS-specific variables:
	short RefNum;	// File reference number
	OSErr Err;		// Error code
	
public:
	
	// MacOS-specific
	short GetRefNum() {return RefNum;}
	OSErr GetError() {return Err;}
	
	bool IsOpen() {return (RefNum != RefNum_Closed);}	
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
	OpenedFile(): RefNum(RefNum_Closed), Err(noErr) {}
	
	// Auto-close when destroying
	~OpenedFile() {Close();}
};

/*
	Abstraction for loaded resources;
	this object will release that resource when it finishes.
*/
class LoadedResource
{
	// This class grabs a resource
	friend class OpenedResourceFile;
	
	// MacOS-specific:
	Handle RsrcHandle;
	
	// Detaches an allocated handle from this object
	// (keep private to avoid memory leaks)
	void Detach()
		{if (RsrcHandle)
			{HUnlock(RsrcHandle); RsrcHandle = NULL;}}
public:
	
	// Handle loaded?
	bool IsLoaded() {return (RsrcHandle != NULL);}
	
	// Unloads an allocated handle
	void Unload()
		{if (RsrcHandle)
			{HUnlock(RsrcHandle); ReleaseResource(RsrcHandle); RsrcHandle = NULL;}}
	
	void *GetPointer(bool DoDetach = false)
		{
		if (RsrcHandle)
			{return *RsrcHandle; if (DoDetach) Detach();}
		else
			return NULL;
		}
	Handle GetHandle(bool DoDetach = false)
		{return RsrcHandle; if (DoDetach) Detach();}

	LoadedResource(): RsrcHandle(NULL) {}
	~LoadedResource() {Unload();}
};


/*
	Abstraction for opened resource files:
	it does opening, setting, and closing of such files;
	also getting "LoadedResource" objects that return pointers
*/
class OpenedResourceFile
{
	// This class will need to set the refnum and error value appropriately 
	friend class FileSpecifier;
	
	// MacOS-specific variables:
	short RefNum;	// File reference number
	OSErr Err;		// Error code
	
	short SavedRefNum;
public:
	
	// MacOS-specific
	short GetRefNum() {return RefNum;}
	OSErr GetError() {return Err;}

	// Pushing and popping the current file -- necessary in the MacOS version,
	// since resource forks are globally open with one of them the current top one.
	// Push() saves the earlier top one makes the current one the top one,
	// while Pop() restores the earlier top one.
	bool Push();
	bool Pop();

	// Pushing and popping are unnecessary for the MacOS version of Get()
	bool Get(OSType Type, short ID, LoadedResource& Rsrc);

	bool IsOpen() {return (RefNum != RefNum_Closed);}
	bool Close();
	
	// Set the file to initially closed
	OpenedResourceFile(): RefNum(RefNum_Closed), Err(noErr), SavedRefNum(RefNum_Closed) {}
	
	// Auto-close when destroying
	~OpenedResourceFile() {Close();}
};


struct FileSpecifier
{	
	FSSpec Spec;
	OSErr Err;

public:

	// Possible typecodes:
	// NONE means ignore typecode info
	enum
	{
		C_Creator,	// Creator code
		C_Map,		// For map/scenario file
		C_Save,		// For savegames
		C_Film,		// For films
		C_Phys,		// For physics
		C_Shape,	// For shapes
		C_Sound,	// For sounds
		C_Patch,	// For patches
		C_Images,	// For images
		C_Prefs,	// For preferences
		C_NUMBER_OF_TYPECODES
	};

	
	// Filespec management
	void SetSpec(FSSpec& _Spec);
	FSSpec& GetSpec() {return Spec;}

	// The error:
	OSErr GetError() {return Err;}
	
	// The name as a C string:
	// assumes enough space to hold it if getting (max. 256 bytes)
	// The typecode is for automatically adding a suffix;
	// NONE means add none
	
	void GetName(char *Name);
	void SetName(char *Name, int Type);
	
	// Parent directories:
		
	bool SetFileToApp();
	bool SetParentToPreferences();
	
	// Partially inspired by portable_files.h:
	
	// These functions take an appropriate one of the typecodes used earlier;
	// this is to try to cover the cases of both typecode attributes
	// and typecode suffixes.
	bool Create(int Type);
	// bool Open(bool Writable = false);
	
	// Opens a file:
	bool Open(OpenedFile& OFile, bool Writable=false);
	
	// Opens either a MacOS resource fork or some imitation of it:
	bool Open(OpenedResourceFile& OFile, bool Writable=false);
	
	// These calls are for creating dialog boxes to set the filespec
	// A null pointer means an empty string
	bool ReadDialog(int Type, char *Prompt=NULL);
	bool WriteDialog(int Type, char *Prompt=NULL, char *DefaultName=NULL);
	
	// Check on whether a file exists, and its type
	bool Exists();
	
	// Returns NONE if the type could not be identified
	int GetType();
	
	// How many bytes are free in the disk that the file lives in?
	bool GetFreeSpace(unsigned long& FreeSpace);
	
	// Copying: either copy the filespec or copy the whole file
	// into the current file object
	bool CopySpec(FileSpecifier& File);
	bool CopyContents(FileSpecifier& File);
	
	bool Delete();
	
	FileSpecifier(): Err(noErr) {}
};


#endif
