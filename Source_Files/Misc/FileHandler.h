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
*/


/*
	Abstraction for opened files; it does reading, writing, and closing of such files,
	without doing anything to the files' specifications
*/
struct OpenedFile
{
	virtual bool IsOpen()=0;
	virtual bool Close()=0;
	
	virtual bool GetPosition(long& Position)=0;
	virtual bool SetPosition(long Position)=0;
	
	virtual bool GetLength(long& Length)=0;
	virtual bool SetLength(long Length)=0;
	
	virtual bool Read(long Count, void *Buffer)=0;
	virtual bool Write(long Count, void *Buffer)=0;
	
	// Smart macros for more easy reading of structured objects
	
	template<class T> bool ReadObject(T& Object)
		{return Read(sizeof(T),&Object);}
	
	template<class T> bool ReadObjectList(int NumObjects, T* ObjectList)
		{return Read(NumObjects*sizeof(T),ObjectList);}
	
	template<class T> bool WriteObject(T& Object)
		{return Write(sizeof(T),&Object);}
	
	template<class T> bool WriteObjectList(int NumObjects, T* ObjectList)
		{return Write(NumObjects*sizeof(T),ObjectList);}
};


struct FileObject
{
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
		C_NUMBER_OF_TYPECODES
	};
	
	// The name as a C string:
	// assumes enough space to hold it if getting (max. 256 bytes)
	// The typecode is for automatically adding a suffix;
	// NONE means add none
	
	virtual void GetName(char *Name)=0;
	virtual void SetName(char *Name, int Type)=0;
	
	// Parent directories:
		
	virtual bool SetFileToApp()=0;
	virtual bool SetParentToPreferences()=0;
	
	// Partially inspired by portable_files.h:
	
	// The typecode is for adding it as a file attribute
	virtual bool Create(int Type)=0;
	virtual bool Open(bool Writable=false)=0;

	virtual bool Open(OpenedFile& OFile, bool Writable=false)=0;
	
	// These calls are for creating dialog boxes to set the filespec
	// A null pointer means an empty string
	// The typecode is for restricting the display
	virtual bool ReadDialog(int Type, char *Prompt=NULL)=0;
	virtual bool WriteDialog(int Type, char *Prompt=NULL, char *DefaultName=NULL)=0;
	
	// Check on whether a file exists, is open, and its type
	virtual bool Exists()=0;
	virtual bool IsOpen()=0;
	
	// Returns NONE if the type could not be identified
	virtual int GetType()=0;
	bool IsTypeCorrect(int Type) {return (Type == GetType());}
	
	virtual bool Close()=0;
	
	virtual bool GetPosition(long& Position)=0;
	virtual bool SetPosition(long Position)=0;
	
	virtual bool GetLength(long& Length)=0;
	virtual bool SetLength(long Length)=0;
	
	virtual bool Read(long Count, void *Buffer)=0;
	virtual bool Write(long Count, void *Buffer)=0;
	
	// How many bytes are free in the disk that the file lives in?
	virtual bool GetFreeSpace(unsigned long& FreeSpace)=0;
	
	// Copying: either copy the filespec or copy the whole file
	// into the current file object
	virtual bool CopySpec(FileObject& File)=0;
	virtual bool CopyContents(FileObject& File)=0;
	
	// Smart macros for more easy reading of structured objects
	
	template<class T> bool ReadObject(T& Object)
		{return Read(sizeof(T),&Object);}
	
	template<class T> bool ReadObjectList(int NumObjects, T* ObjectList)
		{return Read(NumObjects*sizeof(T),ObjectList);}
	
	template<class T> bool WriteObject(T& Object)
		{return Write(sizeof(T),&Object);}
	
	template<class T> bool WriteObjectList(int NumObjects, T* ObjectList)
		{return Write(NumObjects*sizeof(T),ObjectList);}
	
	virtual bool Delete()=0;
};


#endif
