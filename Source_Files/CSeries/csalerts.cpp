#include <stdlib.h>
#include <string.h>

#include <Dialogs.h>
#include <TextUtils.h>

#include "csalerts.h"
#include "csstrings.h"

static Str255 alert_text;

void alert_user(
	short severity,
	short resid,
	short item,
	OSErr error)
{
	getpstr(alert_text,resid,item);
	ParamText(alert_text,NULL,"\p","\p");
	NumToString(error,alert_text);
	ParamText(NULL,alert_text,NULL,NULL);
	InitCursor();
	switch (severity) {
	case infoError:
		Alert(129,NULL);
		break;
	case fatalError:
	default:
		Alert(128,NULL);
		exit(1);
	}
}

void pause(void)
{
	Debugger();
}

void vpause(
	char *message)
{
	long len;

	len=strlen(message);
	if (len>255)
		len=255;
	alert_text[0]=len;
	memcpy(alert_text+1,message,len);
	ParamText(alert_text,"\p","\p","\p");
	InitCursor();
	Alert(129,NULL);
}

void halt(void)
{
	Debugger();
	exit(1);
}

void vhalt(
	char *message)
{
	long len;

	len=strlen(message);
	if (len>255)
		len=255;
	alert_text[0]=len;
	memcpy(alert_text+1,message,len);
	ParamText(alert_text,"\p","\p","\p");
	InitCursor();
	Alert(128,NULL);
	exit(1);
}

static char assert_text[256];

void _assert(
	char *file,
	long line,
	char *what)
{
	vhalt(csprintf(assert_text,"%s:%ld: %s",file,line,what));
}

void _warn(
	char *file,
	long line,
	char *what)
{
	vpause(csprintf(assert_text,"%s:%ld: %s",file,line,what));
}

