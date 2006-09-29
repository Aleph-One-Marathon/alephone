// This does opening and saving in the MacOS with MacOS dialog boxes

#include <Carbon.h>
#include <algorithm.h>
#include <string.h>
#include "OpenSave_Interface.h"
#include "GetFullPathObject.h"
#include "MiscUtils.h"


typedef unsigned char byte;

template<class T> void obj_clear(T& x) {memset(&x,0,sizeof(x));}

static OSStatus ExtractSingleItem(const NavReplyRecord *reply, FSSpec *item);


char *OpenFile(struct OpenParameters *OpenParms) {

	// LP: AlexJLS's Nav Services code, somewhat modified
	NavTypeListHandle list= NULL;
	
	if (OpenParms->NumTypes > 0)
	{
		list= (NavTypeListHandle)NewHandleClear(sizeof(NavTypeList) + (OpenParms->NumTypes-1)*sizeof(OSType));
		HLock((Handle)list);
		(**list).componentSignature = NULL;
		(**list).osTypeCount = OpenParms->NumTypes;
		for (int k=0; k<OpenParms->NumTypes; k++)
			(**list).osType[k] = OpenParms->TypeList[k];
	}
	
	NavDialogOptions opts;
	NavGetDefaultDialogOptions(&opts);
	if (OpenParms->Prompt)
		CopyCStringToPascal(OpenParms->Prompt,opts.message);
	opts.dialogOptionFlags = kNavNoTypePopup | kNavAllowPreviews;
	
	NavReplyRecord reply;
	NavGetFile(NULL,&reply,&opts,NULL,NULL,NULL,list,NULL);
	if (list) DisposeHandle((Handle)list);
		
	if (!reply.validRecord) return 0;
	
	FSSpec temp;
	obj_clear(temp);
	ExtractSingleItem(&reply,&temp);
	
	GetFullPathObject Pathfinder;
	Pathfinder.GetFullPath(&temp);
	
	if (Pathfinder.Err != noErr) return 0;
	int nbytes = Pathfinder.FullPath.get_len();
	if (nbytes <= 0) return 0;
	
	// Concession to plain-C programmers
	char *FileSpec = (char *)malloc(nbytes);
	if (FileSpec == 0) return 0;
	
	memcpy(FileSpec,Pathfinder.FullPath.begin(),nbytes);
	return FileSpec;
}

char *SaveFile(struct SaveParameters *SaveParms) {
	
	// LP: AlexJLS's Nav Services code, somewhat modified
	NavDialogOptions opts;
	NavGetDefaultDialogOptions(&opts);
	if (SaveParms->Prompt)
		CopyCStringToPascal(SaveParms->Prompt,opts.message);
	if (SaveParms->DefaultName)
		CopyCStringToPascal(SaveParms->DefaultName,opts.savedFileName);
	opts.dialogOptionFlags &= ~kNavAllowStationery;
	
	NavReplyRecord reply;
	NavPutFile(NULL,&reply,&opts,NULL,SaveParms->TypeCode,SaveParms->CreatorCode,NULL);
	
	if (!reply.validRecord) return false;
	
	FSSpec temp;
	obj_clear(temp);
	ExtractSingleItem(&reply,&temp);

	// Create the file before getting its path	
	if (reply.replacing)
		FSpDelete(&temp);
	if (FSpCreate(&temp,SaveParms->CreatorCode,SaveParms->TypeCode,smSystemScript) != 0)
		return 0;

	GetFullPathObject Pathfinder;
	Pathfinder.GetFullPath(&temp);
	
	if (Pathfinder.Err != noErr) return 0;
	int nbytes = Pathfinder.FullPath.get_len();
	if (nbytes <= 0) return 0;
	
	// Concession to plain-C programmers
	char *FileSpec = (char *)malloc(nbytes);
	if (FileSpec == 0) return 0;
	
	memcpy(FileSpec,Pathfinder.FullPath.begin(),nbytes);
	return FileSpec;
}


// LP: AlexJLS's Nav Services code, somewhat modified
//Function stolen from MPFileCopy
OSStatus ExtractSingleItem(const NavReplyRecord *reply, FSSpec *item)
	// This item extracts a single FSRef from a NavReplyRecord.
	// Nav makes it really easy to support 'odoc', but a real pain
	// to support other things.  *sigh*
{
	OSStatus err;
	SInt32 itemCount;
	FSSpec fss, fss2;
	AEKeyword junkKeyword;
	DescType junkType;
	Size junkSize;

	obj_clear(fss);
	obj_clear(fss2);
	
	//MoreAssertQ((AECountItems(&reply->selection, &itemCount) == noErr) && (itemCount == 1));
	
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
