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

