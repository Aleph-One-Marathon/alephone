/*
 *  XML_Loader_SDL.h - Parser for XML files, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef _XML_LOADER_SDL_
#define _XML_LOADER_SDL_

#include "XML_Configure.h"

class FileSpecifier;


class XML_Loader_SDL : public XML_Configure {
public:
	XML_Loader_SDL() : data(NULL) {}
	~XML_Loader_SDL() {delete[] data; data = NULL;}

	bool ParseDirectory(FileSpecifier &dir);

protected:
	virtual bool GetData();
	virtual void ReportReadError();
	virtual void ReportParseError(const char *ErrorString, int LineNumber);
	virtual void ReportInterpretError(const char *ErrorString);
	virtual bool RequestAbort();

private:
	char *data;
	long data_size;
};

#endif
