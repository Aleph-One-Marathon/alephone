// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_ALERTS_
#define _CSERIES_ALERTS_

enum {
	infoError,
	fatalError
};

extern void alert_user(
	short severity,
	short resid,
	short item,
	OSErr error);

extern void pause_debug(void);
extern void vpause(
	char *message);

extern void halt(void);
extern void vhalt(
	char *message);

extern void _alephone_assert(
	char *file,
	long line,
	char *what);
extern void _alephone_warn(
	char *file,
	long line,
	char *what);

#undef assert
#ifdef DEBUG
#define assert(what) ((what) ? (void)0 : _alephone_assert(__FILE__,__LINE__,"Assertion failed: " #what))
#define vassert(what,message) ((what) ? (void)0 : _alephone_assert(__FILE__,__LINE__,(message)))
#define warn(what) ((what) ? (void)0 : _alephone_warn(__FILE__,__LINE__,"Assertion failed: " #what))
#define vwarn(what,message) ((what) ? (void)0 : _alephone_warn(__FILE__,__LINE__,(message)))
#else
#define assert(what) ((void)(what))
#define vassert(what,message) ((void)(what))
#define warn(what) ((void)(what))
#define vwarn(what,message) ((void)(what))
#endif

#endif
