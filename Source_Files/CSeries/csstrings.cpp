// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
// LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <Resources.h>
#include <TextUtils.h>

#include "csstrings.h"

// LP addition: use XML as source of strings (Apr 20, 2000)
#include "TextStrings.h"

char temporary[256];


short countstr(
	short resid)
{
	// LP change: look in the string sets
	return TS_CountStrings(resid);
	
	/*
	Handle res;

	res=GetResource('STR#',resid);
	if (!res)
		return 0;
	if (!*res)
		LoadResource(res);
	if (!*res)
		return 0;
	return *(short *)*res;
	*/
}

unsigned char *getpstr(
	unsigned char *string,
	short resid,
	short item)
{
	unsigned char *CollString = TS_GetString(resid,item);
	if (CollString)
	{
		memcpy(string,CollString,CollString[0]+1);
	}
	else
	{
		string[0] = 0;
	}
	return string;
	
/*
	Handle res;
	int i,cnt;
	unsigned char *src;

	res=GetResource('STR#',resid);
	if (!res)
		goto notfound;
	if (!*res)
		LoadResource(res);
	if (!*res)
		goto notfound;
	cnt=*(short *)*res;
	if (item<0 || item>=cnt)
		goto notfound;
	src=(unsigned char *)(*res+sizeof (short));
	for (i=0; i<item; i++) {
		src+=src[0]+1;
	}
	memcpy(string,src,src[0]+1);
	return string;
notfound:
	string[0]=0;
	return string;
*/
}

char *getcstr(
	char *string,
	short resid,
	short item)
{
	unsigned char *CollString = TS_GetString(resid,item);
	if (CollString)
	{
		// The C string is offset one from the Pascal string
		memcpy(string,CollString+1,CollString[0]+1);
	}
	else
	{
		string[0] = 0;
	}
	return string;
	
	/*
	Handle res;
	int i,cnt,len;
	unsigned char *src;

	res=GetResource('STR#',resid);
	if (!res)
		goto notfound;
	if (!*res)
		LoadResource(res);
	if (!*res)
		goto notfound;
	cnt=*(short *)*res;
	if (item<0 || item>=cnt)
		goto notfound;
	src=(unsigned char *)(*res+sizeof (short));
	for (i=0; i<item; i++) {
		src+=src[0]+1;
	}
	len=src[0];
	memcpy(string,src+1,len);
	string[len]=0;
	return string;
notfound:
	string[0]=0;
	return string;
	*/
}

unsigned char *pstrcpy(
	unsigned char *dst,
	unsigned char *src)
{
	memcpy(dst,src,src[0]+1);
	return dst;
}

char *csprintf(
	char *buffer,
	const char *format,
	...)
{
	va_list list;

	va_start(list,format);
	vsprintf(buffer,format,list);
	va_end(list);
	return buffer;
}

unsigned char *psprintf(
	unsigned char *buffer,
	const char *format,
	...)
{
	va_list list;

	va_start(list,format);
	vsprintf((char *)buffer,format,list);
	va_end(list);
	c2pstr((char *)buffer);
	return buffer;
}

void dprintf(
	const char *format,
	...)
{
	Str255 buffer;
	va_list list;

	va_start(list,format);
	vsprintf((char *)buffer,format,list);
	va_end(list);
	c2pstr((char *)buffer);
	DebugStr(buffer);
}

void fdprintf(
	const char *format,
	...)
{
	FILE *FD = fopen("AlephOneDebugLog.txt","a");
	va_list list;

	va_start(list,format);
	vfprintf(FD,format,list);
	va_end(list);
	fprintf(FD,"\n");
	fclose(FD);
}

