/*
	find_files.c
	Wednesday, January 11, 1995 6:44:55 PM

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Aug 25, 2000 (Loren Petrich):
	Abstracting the file handling

Aug 28, 2000 (Loren Petrich):
	Put these functions into the FileFinder object; eradicated the "param_block->" code, etc.
	and did other things to abstract it.
*/

#include "macintosh_cseries.h"
#include <string.h> /* For memset */
#include <stdlib.h>

#include "find_files.h"

#ifdef env68k
	#pragma segment file_io
#endif

/* --------- local prototypes */
// static int alphabetical_names(void const *a, void const *b);
// static OSErr enumerate_files(struct find_file_pb *param_block, long directory_id);

// Code:

void FileFinder::Clear()
{
	memset(this,0,sizeof(FileFinder));
}


/* ---------------- Parameter Block Version */
// OSErr find_files(
//	struct find_file_pb *param_block)
bool FileFinder::Find()
{
	// Translate the type info:
	type_to_find = (Type != WILDCARD_TYPE) ? get_typecode(Type) : 0;
	
	// Eliminated buffer creation as unused
	
	/* Assert that we got it. */
	assert(search_type==_callback_only || buffer);
	assert(version==0);

	/* Set the variables */
	count= 0;

	Enumerate(BaseDir);
	
	// Eliminated alphabetical order as unused
		
	return (Err == noErr);
}

/*
Boolean equal_fsspecs(
	FSSpec *a, 
	FSSpec *b)
{
	Boolean equal= false;
	
	if(a->vRefNum==b->vRefNum && a->parID==b->parID && 
		EqualString(a->name, b->name, false, false))
	{
		equal= true;
	}
	
	return equal;
}
*/

/* --------- Local Code --------- */
// static OSErr enumerate_files(
//	struct find_file_pb *param_block, 
//	long directory_id) /* Because it is recursive.. */
bool FileFinder::Enumerate(DirectorySpecifier& Dir)
{
	// Set up the temporary file
	TempFile.FromDirectory(Dir);

	// Kludge to make the FSSpec always available
	FSSpec& temp_file = TempFile.GetSpec();
	
	// Local variable
	short index;

	memset(&pb, 0, sizeof(CInfoPBRec));
	
	pb.hFileInfo.ioVRefNum= Dir.Get_vRefNum();
	pb.hFileInfo.ioNamePtr= temp_file.name;
			
	for(Err= noErr, index=1; count<max && Err==noErr; index++) 
	{
		pb.hFileInfo.ioDirID= Dir.Get_parID();
		pb.hFileInfo.ioFDirIndex= index;

		Err= PBGetCatInfo( &pb, false);
		if(Err == noErr)
		{
			if ((pb.hFileInfo.ioFlAttrib & 16) && (flags & _ff_recurse))
			{
				/* Recurse, if you really want to... */
				
				// Set up volume info
				DirectorySpecifier SubDir = Dir;
				// Change the directory
				SubDir.Set_parID(pb.dirInfo.ioDrDirID);
				// Go!
				Enumerate(SubDir);
				// Reset the temporary file's directory to what's appropriate here
				// (it's global to this object)
				TempFile.FromDirectory(Dir);
				
			} else {
				/* Add.. */
				if(Type==WILDCARD_TYPE || 
					pb.hFileInfo.ioFlFndrInfo.fdType==type_to_find)
				{
					/* Only add if there isn't a callback or it returns TRUE */
					switch(search_type)
					{
						case _fill_buffer:
							if(!callback || callback(TempFile, user_data))
							// if(!callback || callback(&temp_file, user_data))
							{
								/* Copy it in.. */
								buffer[count++] = TempFile;
							}
							break;
							
						case _callback_only:
							assert(callback);
							if(flags & _ff_callback_with_catinfo)
							{
								callback(TempFile, &pb);
								// callback(&temp_file, &pb);
							} else {
								callback(TempFile, user_data);
								// callback(&temp_file, user_data);
							}
							break;
							
						default:
							// LP change:
							assert(false);
							// halt();
							break;
					}
				}
			}
		} else {
			/* We ran out of files.. */
		}
	}

	/* If we got a fnfErr, it was because we indexed too far. */	
	///return (Err==fnfErr) ? (noErr) : Err;
	if (Err == fnfErr) Err = noErr;
	
	return (Err == noErr);
}
/*
static int alphabetical_names(
	void const *a, 
	void const *b)
{
	return (IUCompString(((FSSpec const *)a)->name, ((FSSpec const *)b)->name)); 
}
*/

// Original code, for reference purposes
#if 0
/* ---------------- Parameter Block Version */
OSErr find_files(
	struct find_file_pb *param_block)
{
	OSErr err;

	/* If we need to create the destination buffer */
	if(param_block->flags & _ff_create_buffer) 
	{
		assert(param_block->search_type==_fill_buffer);
		param_block->buffer= (FSSpec *) NewPtr(sizeof(FSSpec)*param_block->max);
	}

	/* Assert that we got it. */
	assert(param_block->search_type==_callback_only || param_block->buffer);
	assert(param_block->version==0);

	/* Set the variables */
	param_block->count= 0;

	err= enumerate_files(param_block, param_block->BaseDir.Get_parID());
	// err= enumerate_files(param_block, param_block->directory_id);

	/* Alphabetical */
	if(param_block->flags & _ff_alphabetical)
	{
		assert(param_block->search_type==_fill_buffer);
		qsort(param_block->buffer, param_block->count, 
			sizeof(FSSpec), alphabetical_names);
	}

	/* If we created the buffer, make it the exact right size */
	if(param_block->flags & _ff_create_buffer)
	{
		assert(param_block->search_type==_fill_buffer);
		SetPtrSize((Ptr) param_block->buffer, sizeof(FSSpec)*(param_block->count));
	}
	
	return err;
}


/* --------- Local Code --------- */
static OSErr enumerate_files(
	struct find_file_pb *param_block, 
	long directory_id) /* Because it is recursive.. */
{
	static CInfoPBRec pb; /* static to prevent stack overflow.. */
	// Kludge to make the FSSpec always available
	static FileSpecifier TempFile;
	static FSSpec& temp_file = TempFile.GetSpec();
	static OSErr err;
	short index;

	memset(&pb, 0, sizeof(CInfoPBRec));
	
	temp_file.vRefNum= param_block->BaseDir.Get_vRefNum();
	// temp_file.vRefNum= param_block->vRefNum;
	pb.hFileInfo.ioVRefNum= temp_file.vRefNum;
	pb.hFileInfo.ioNamePtr= temp_file.name;
			
	for(err= noErr, index=1; param_block->count<param_block->max && err==noErr; index++) 
	{
		pb.hFileInfo.ioDirID= directory_id;
		pb.hFileInfo.ioFDirIndex= index;

		err= PBGetCatInfo( &pb, false);
		if(!err)
		{
			if ((pb.hFileInfo.ioFlAttrib & 16) && (param_block->flags & _ff_recurse))
			{
				/* Recurse, if you really want to... */
				err= enumerate_files(param_block, pb.dirInfo.ioDrDirID);
			} else {
				/* Add.. */
				if(param_block->type_to_find==WILDCARD_TYPE || 
					pb.hFileInfo.ioFlFndrInfo.fdType==param_block->type_to_find)
				{
					temp_file.vRefNum= pb.hFileInfo.ioVRefNum;
					temp_file.parID= directory_id;
					
					/* Only add if there isn't a callback or it returns TRUE */
					switch(param_block->search_type)
					{
						case _fill_buffer:
							if(!param_block->callback || param_block->callback(TempFile, param_block->user_data))
							// if(!param_block->callback || param_block->callback(&temp_file, param_block->user_data))
							{
								/* Copy it in.. */
								BlockMove(&temp_file, &param_block->buffer[param_block->count++], 
									sizeof(FSSpec));
							}
							break;
							
						case _callback_only:
							assert(param_block->callback);
							if(param_block->flags & _ff_callback_with_catinfo)
							{
								param_block->callback(TempFile, &pb);
								// param_block->callback(&temp_file, &pb);
							} else {
								param_block->callback(TempFile, param_block->user_data);
								// param_block->callback(&temp_file, param_block->user_data);
							}
							break;
							
						default:
							// LP change:
							assert(false);
							// halt();
							break;
					}
				}
			}
		} else {
			/* We ran out of files.. */
		}
	}

	/* If we got a fnfErr, it was because we indexed too far. */	
	return (err==fnfErr) ? (noErr) : err;
}
#endif
