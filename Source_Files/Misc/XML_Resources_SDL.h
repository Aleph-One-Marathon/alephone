/*
 *  XML_Resources_SDL.h - Parser for XML-containing resource files
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef _XML_RESOURCES_SDL_
#define _XML_RESOURCES_SDL_

#include "XML_Configure.h"

class XML_Resources_SDL : public XML_Configure
{
public:
	XML_Resources_SDL() : data(NULL), data_size(0) {}
	virtual ~XML_Resources_SDL() {if (data) free(data);}

	bool ParseResourceSet(uint32 Type);

protected:
	virtual bool GetData();
	virtual void ReportReadError();
	virtual void ReportParseError(const char *ErrorString, int LineNumber);
	virtual void ReportInterpretError(const char *ErrorString);
	virtual bool RequestAbort();

private:
	void *data;
	uint32 data_size;
};

#endif
