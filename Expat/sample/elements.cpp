/* This is simple demonstration of how to use expat. This program
reads an XML document from standard input and writes a line with the
name of each element to standard output indenting child elements by
one tab stop more than their parent element. */

#include <stdio.h>
#include "xmlparse.h"

// LP change, 3-8-00: added open-file dialog
#include <stdlib.h>
#include "OpenSave_Interface.h"



void startElement(void *userData, const char *name, const char **atts)
{
  int i;
  int *depthPtr = (int *)userData;
  for (i = 0; i < *depthPtr; i++)
    putchar('\t');
  puts(name);
  *depthPtr += 1;
  
  char **AttPtr = (char **)atts;
  while(*AttPtr != 0)
  {
  for (i = 0; i < *depthPtr; i++)
    putchar('\t');
    putchar('$');
    puts(*AttPtr);
  	AttPtr++;
  }
}

void endElement(void *userData, const char *name)
{
  int *depthPtr = (int *)userData;
  *depthPtr -= 1;
}

// LP: added this to see how it works
void CharacterDataHandler(void *userData,
					 const XML_Char *s,
					 int len)
{
  int i;
  int *depthPtr = (int *)userData;
  for (i = 0; i < *depthPtr; i++)
    putchar('\t');
  printf("<<<");
  for (i=0; i<len; i++)
   	putchar(s[i]);
  printf(">>>");
  putchar('\n');
}


int main()
{
	// LP: added open/save; changed "stdin" to filespec
	// Also added some Toolbox food.
	// All the non_XML printouts will have prefix "***"
	printf("*** Starting XML Tester\n");
	// Looking for plain old text
	OpenParameters OParms;
	OParms.NumTypes = 1;
	OParms.TypeList[0] = 'TEXT';
	OParms.Prompt = "What XML file to analyze?";
	char *FileName = OpenFile(&OParms);
	if (FileName == NULL)
	{
		printf("*** Will not open a file\n");
		return -1;
	}
	FILE *fptr = fopen(FileName,"r");
	if (fptr == NULL)
	{
		printf("*** Error in opening file \"%s\"\n",FileName);
		return -1;
	}
	free(FileName);
	
  char buf[BUFSIZ];
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;
  int depth = 0;
  XML_SetUserData(parser, &depth);
	XML_SetCharacterDataHandler(parser, CharacterDataHandler);
  XML_SetElementHandler(parser, startElement, endElement);
  do {
    size_t len = fread(buf, 1, sizeof(buf), fptr);
    done = len < sizeof(buf);
    if (!XML_Parse(parser, buf, len, done)) {
      fprintf(stderr,
	      "*** %s at line %d\n",
	      XML_ErrorString(XML_GetErrorCode(parser)),
	      XML_GetCurrentLineNumber(parser));
      return 1;
    }
  } while (!done);
  XML_ParserFree(parser);
  
  // To indicate that it's done
	printf("*** Done\n");
  return 0;
}
