/*
 *  XML_Resources_SDL.h - Parser for XML-containing resource files
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef _XML_RESOURCES_SDL_
#define _XML_RESOURCES_SDL_

#include "XML_Configure.h"
#include "FileHandler.h"

class XML_Resources_SDL : public XML_Configure
{
public:
	XML_Resources_SDL() {}
	virtual ~XML_Resources_SDL() {rsrc.Unload();}

	bool ParseResourceSet(uint32 Type);

protected:
	virtual bool GetData();
	virtual void ReportReadError();
	virtual void ReportParseError(const char *ErrorString, int LineNumber);
	virtual void ReportInterpretError(const char *ErrorString);
	virtual bool RequestAbort();

private:
	LoadedResource rsrc;
};

#endif
