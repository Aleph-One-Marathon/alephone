// This does opening and saving in the MacOS with MacOS dialog boxes

#include <Script.h>
#include <StandardFile.h>
#include <stdlib.h>
#include <string.h>
#include "OpenSave_Interface.h"
#include "GetFullPathObject.h"
#include "MiscUtils.h"


typedef unsigned char byte;


char *OpenFile(struct OpenParameters *OpenParms) {

	// Probably better to use CustomGetFile,
	// and to use a DLOG resource that has a prompt string,
	// which would then be replaced by OpenParms->Prompt
	StandardFileReply Reply;
	StandardGetFile(0,OpenParms->NumTypes,OpenParms->TypeList,&Reply);
	if (!Reply.sfGood) return 0;

	GetFullPathObject Pathfinder;
	Pathfinder.GetFullPath(&Reply.sfFile);
	
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
	
	// Pascalize the prompt and default strings for the MacOS:
	Str31 PasPrompt, PasDefault;
	PasPrompt[0] = 0;
	PasDefault[0] = 0;
	if (SaveParms->Prompt != 0) C2Pas((byte *)SaveParms->Prompt,PasPrompt,31);
	if (SaveParms->DefaultName != 0) C2Pas((byte *)SaveParms->DefaultName,PasDefault,31);
	
	StandardFileReply Reply;
	StandardPutFile(PasPrompt,PasDefault,&Reply);
	if (!Reply.sfGood) return 0;

	// Create the file before getting its path	
	if (Reply.sfReplacing)
		FSpDelete(&Reply.sfFile);
	if (FSpCreate(&Reply.sfFile,SaveParms->CreatorCode,SaveParms->TypeCode,smSystemScript) != 0)
		return 0;

	GetFullPathObject Pathfinder;
	Pathfinder.GetFullPath(&Reply.sfFile);
	
	if (Pathfinder.Err != noErr) return 0;
	int nbytes = Pathfinder.FullPath.get_len();
	if (nbytes <= 0) return 0;
	
	// Concession to plain-C programmers
	char *FileSpec = (char *)malloc(nbytes);
	if (FileSpec == 0) return 0;
	
	memcpy(FileSpec,Pathfinder.FullPath.begin(),nbytes);
	return FileSpec;
}
