// This is for dumping STR# and nrct resources from a Marathon app file

#include <Script.h>
#include <Resources.h>
#include <StandardFile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void CheckMacError(OSErr MacError)
{
	if (MacError != noErr)
	{
		printf("MacOS error = %d\nQuitting\n",MacError);
		exit(-1);
	}
}


main()
{
	printf("Starting the resource dumper...\n"); 

	// Get the file to read
	StandardFileReply Reply;
	StandardGetFile(0,-1,0,&Reply);
	if (!Reply.sfGood)
	{
		printf("Quitting\n");
		return 0;
	}

	// Open its resource fork
	short RefNum = FSpOpenResFile(&Reply.sfFile, fsRdPerm);
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
				char Char = *(CharPtr++);
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
				case 'ª':
					fprintf(fptr,"[TM]");
					break;
				case '§':
					fprintf(fptr,"b");
					break;
				default:
					fprintf(fptr,"%c",Char);
				}
				// Upper-byte-set characters
				if (Char & 0x80) ExtraSpaceBeforeEnd = true;
			}
			// Space put in if necessary
			if (ExtraSpaceBeforeEnd)
				fprintf(fptr," ");
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