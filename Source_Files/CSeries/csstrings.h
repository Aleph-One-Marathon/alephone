// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
// LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt
#ifndef _CSERIES_STRINGS_
#define _CSERIES_STRINGS_

extern char temporary[256];
#define ptemporary (*(Str255 *)temporary)

extern short countstr(
	short resid);

extern unsigned char *getpstr(
	unsigned char *string,
	short resid,
	short item);

extern char *getcstr(
	char *string,
	short resid,
	short item);

extern unsigned char *pstrcpy(
	unsigned char *dst,
	unsigned char *src);

extern char *csprintf(
	char *buffer,
	const char *format,
	...);

extern unsigned char *psprintf(
	unsigned char *buffer,
	const char *format,
	...);

extern void dprintf(
	const char *format,
	...);

extern void fdprintf(
	const char *format,
	...);

#endif

