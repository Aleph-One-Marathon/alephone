// This implements my full-path routines


#include "MoreFilesExtract.h"
#include "GetFullPathObject.h"


bool GetFullPathObject::GetFullPath(FSSpec *Spec) {

	// Reset the variables
	Err = noErr;
	FullPath.reallocate(0);
	
	Handle PathHdl;
	short FullPathLen;
	OSErr Err = FSpGetFullPath(Spec, &FullPathLen, &PathHdl);
	if (Err != noErr) return false;
	
	// Allocate some space for the new string (has to be copied)
	FullPath.reallocate(FullPathLen+1);
	
	// Copy and get into C format
	HLock(PathHdl);
	BlockMove(*PathHdl,FullPath.begin(),FullPathLen);
	FullPath[FullPathLen] = 0;
	HUnlock(PathHdl);
	DisposeHandle(PathHdl);
	
	// Success
	return true;
}
