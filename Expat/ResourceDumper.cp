// This is for dumping STR# and nrct resources from a Marathon app file

#include <Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


template<class T> void obj_clear(T& x) {memset(&x,0,sizeof(x));}

static OSStatus ExtractSingleItem(const NavReplyRecord *reply, FSSpec *item);


void CheckMacError(OSErr MacError)
{
	if (MacError != noErr)
	{
		printf("MacOS error = %d\nQuitting\n",MacError);
		exit(-1);
	}
}


int main()
{
	printf("Starting the resource dumper...\n"); 

	// LP: AlexJLS's Nav Services code, somewhat modified
	NavTypeListHandle list= NULL;
	
	NavDialogOptions opts;
	NavGetDefaultDialogOptions(&opts);
	CopyCStringToPascal("What to extract STR# and nrct from?",opts.message);
	opts.dialogOptionFlags = kNavNoTypePopup | kNavAllowPreviews;
	
	NavReplyRecord reply;
	NavGetFile(NULL,&reply,&opts,NULL,NULL,NULL,list,NULL);
	if (list) DisposeHandle((Handle)list);
		
	if (!reply.validRecord) return 0;
	
	FSSpec temp;
	obj_clear(temp);
	ExtractSingleItem(&reply,&temp);

	// Open its resource fork
	short RefNum = FSpOpenResFile(&temp, fsRdPerm);
	CheckMacError(ResError());
	
	// Push the current resource fork
	short PrevRefNum = CurResFile();
	UseResFile(RefNum);
	
	// Dump those resources
	printf("What dump file? ");
	char FileSpec[256];
	scanf("%s",FileSpec);
	FILE *fptr = fopen(FileSpec,"w");
	if (fptr == 0)
	{
		printf("File-opening error\n");
		return -1;
	}
	
	fprintf(fptr,"<marathon>\n\n");
	
	// First, dump the "STR#" resources
	
	short NumResources = Count1Resources('STR#');
	for (short ires=1; ires<=NumResources; ires++)
	{
		Handle ResHdl = Get1IndResource('STR#',ires);
		
		ResType Type;
		short ID;
		Str255 ResName;
		GetResInfo(ResHdl,&ID, &Type, ResName);
		
		fprintf(fptr,"<!-- STR# Resource: \"");
		for (int k=1; k<=ResName[0]; k++)
			fprintf(fptr,"%c",ResName[k]);
		fprintf(fptr,"\" -->\n");
		
		fprintf(fptr,"<stringset index=\"%d\">\n",ID);
		
		HLock(ResHdl);
		Ptr ResPtr = *ResHdl;
		
		// Grab the strings:
		short NumStrings = *((short *)ResPtr);
		Ptr CharPtr = ResPtr + sizeof(short);
		for (int istr=0; istr<NumStrings; istr++)
		{
			fprintf(fptr,"<string index=\"%d\">",istr);
			unsigned char StrLen = *(CharPtr++);
			// Put in an extra space after certain characters
			bool ExtraSpaceBeforeEnd = false;
			for (int k=0; k<StrLen; k++)
			{
				unsigned char Char = *(CharPtr++);
				ExtraSpaceBeforeEnd = false;
				switch(Char)
				{
				case '<':
					fprintf(fptr,"&lt;");
					break;
				case '>':
					fprintf(fptr,"&gt;");
					break;
				case '&':
					fprintf(fptr,"&amp;");
					break;
				case '\'':
					fprintf(fptr,"&apos;");
					break;
				case '"':
					fprintf(fptr,"&quot;");
					break;
				default:
					if (Char >= 0x80)
					{
						// UTF-8 encoding
						unsigned char Char1 = 0xc0 + (Char >> 6);
						unsigned char Char2 = 0x80 + (Char & 0x3f);
						fprintf(fptr,"%c%c",Char1,Char2);
					}
					else
						fprintf(fptr,"%c",Char);
				}
				// Upper-byte-set characters
				//if (Char & 0x80) ExtraSpaceBeforeEnd = true;
			}
			// Space put in if necessary
			//if (ExtraSpaceBeforeEnd)
			//	fprintf(fptr," ");
			fprintf(fptr,"</string>\n");
		}
		
		HUnlock(ResHdl);
		ReleaseResource(ResHdl);

		fprintf(fptr,"</stringset>\n\n");
	}
	
	NumResources = Count1Resources('nrct');
	for (short ires=1; ires<=NumResources; ires++)
	{
		Handle ResHdl = Get1IndResource('nrct',ires);
		
		ResType Type;
		short ID;
		Str255 ResName;
		GetResInfo(ResHdl,&ID, &Type, ResName);
		
		fprintf(fptr,"<!-- nrct Resource ID: %d -->\n",ID);
		
		// Grab the rectangles:
		HLock(ResHdl);
		Ptr ResPtr = *ResHdl;
		
		short *ResInts = (short *)ResPtr;

		fprintf(fptr,"<interface>\n");
		
		short NumRects = *(ResInts++);
		for (int irct=0; irct<NumRects; irct++)
		{
			short Top = *(ResInts++);
			short Left = *(ResInts++);
			short Bottom = *(ResInts++);
			short Right = *(ResInts++);
			fprintf(fptr,"<rect index=\"%d\" top=\"%d\" left=\"%d\" bottom=\"%d\" right=\"%d\"/>\n",irct,Top,Left,Bottom,Right);
		}
		
		fprintf(fptr,"</interface>\n");
		
		HUnlock(ResHdl);
		ReleaseResource(ResHdl);
	}
	
	// Pop it
	UseResFile(PrevRefNum);
	
	fprintf(fptr,"</marathon>\n");
	printf("All done!\n");
	
	return 0;
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
