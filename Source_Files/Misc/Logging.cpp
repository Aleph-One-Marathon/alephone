/*
 *  Logging.cpp - facilities for flexible logging

	Copyright (C) 2003 and beyond by Woody Zenfell, III
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html


	Jan. 16, 2003 (Woody Zenfell): Created.

	May 21, 2003 (Woody Zenfell): being a little more defensive about NULL file pointer.
*/

#include "Logging.h"
#include "cseries.h"
#include "shell.h"

#include <fstream>
#include <string>
#include <vector>
#include <time.h>	// apparently is in C std library, used here to print time/date log section started.
#include <stdio.h>
#include "FileHandler.h"
#include "InfoTree.h"

#ifndef NO_STD_NAMESPACE
using std::vector;
using std::string;
#endif

enum { kStringBufferSize = 1024 };

static Logger*	sCurrentLogger	= NULL;
static FILE*	sOutputFile	= NULL;
static int	sLoggingThreshhold = logNoteLevel;	// log messages at or above this level will be squelched
static bool	sShowLocations	= true;			// should filenames and line numbers be printed as well?
static bool	sFlushOutput	= false;		// flush output after every log-write?  (good if crash expected)
const char*	logDomain	= "global";


static void InitializeLogging();


Logger*
GetCurrentLogger() {
    if(sCurrentLogger == NULL)
        InitializeLogging();

    return sCurrentLogger;
}





void
Logger::pushLogContext(const char* inFile, int inLine, const char* inContext, ...) {
    va_list theVarArgs;
    va_start(theVarArgs, inContext);
    pushLogContextV(inFile, inLine, inContext, theVarArgs);
    va_end(theVarArgs);
}


void
Logger::logMessage(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, ...) {
    va_list theVarArgs;
    va_start(theVarArgs, inMessage);
    logMessageV(inDomain, inLevel, inFile, inLine, inMessage, theVarArgs);
    va_end(theVarArgs);
}


void
Logger::logMessageNMT(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, ...) {
	va_list theVarArgs;
	va_start(theVarArgs, inMessage);
	logMessageV(inDomain, inLevel, inFile, inLine, inMessage, theVarArgs);
	va_end(theVarArgs);
}

Logger::~Logger() {
}





class TopLevelLogger : public Logger {
public:
    TopLevelLogger() : mMostRecentCommonStackDepth(0), mMostRecentlyPrintedStackDepth(0) {}
    virtual void pushLogContextV(const char* inFile, int inLine, const char* inContext, va_list inArgs);
    virtual void popLogContext();
    virtual void logMessageV(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, va_list inArgs);
	void flush();
protected:
    vector<string>	mContextStack;
    size_t	mMostRecentCommonStackDepth;
    size_t	mMostRecentlyPrintedStackDepth;
};


void
TopLevelLogger::pushLogContextV(const char* inFile, int inLine, const char* inContext, va_list inArgs) {
        char 	stringBuffer[kStringBufferSize];
        vsnprintf(stringBuffer, kStringBufferSize, inContext, inArgs);
        string	theContextString(stringBuffer);
        if(sShowLocations) {
                // Strictly speaking, the choice of whether to include location info should be
                // deferred until the context actually gets logged, just in case the setting
                // changes in between entering the context and logging a message.
                snprintf(stringBuffer, kStringBufferSize, " (%s:%d)", inFile, inLine);
                theContextString += stringBuffer;
        }
        mContextStack.push_back(theContextString);
}


void
TopLevelLogger::popLogContext() {
    mContextStack.pop_back();
    if(mContextStack.size() < mMostRecentCommonStackDepth)
        mMostRecentCommonStackDepth = mContextStack.size();
}


// domains are currently unused; idea is that eventually different logs can be routed to different
// files, different domains can have different levels of detail, etc.
// Something like network.h would declare extern const char* NetworkLoggingDomain;, and some
// .cpp would (obviously) provide it - then files that want to log in that domain would put
// static const char* logDomain = NetworkLoggingDomain; so that all logging calls (via the macros)
// would end up in the Network logging domain instead of the Global domain.  (Also this way creates
// an identifier that the compiler can spell-check etc., which you wouldn't get with
// static const char* logDomain = "Network"; or the like.
void
TopLevelLogger::logMessageV(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, va_list inArgs) {
    // Obviously eventually this will be settable more dynamically...
    // Also eventually some logged messages could be posted in a dialog in addition to appended to the file.
    if(sOutputFile != NULL && inLevel < sLoggingThreshhold) {
        char	stringBuffer[kStringBufferSize];
        size_t firstDepthToPrint = mMostRecentCommonStackDepth;
    /*
        // This was designed to give a little context when coming back from deep stacks, but it seems
        // rather annoying to me in practice.  (Maybe should be set to only kick in for bigger stack depth differences,
        // or after a certain number of entries at deeper depths, etc.)
        if(mMostRecentlyPrintedStackDepth != mMostRecentCommonStackDepth && firstDepthToPrint > 0)
            firstDepthToPrint--;
    */
        for(size_t depth = firstDepthToPrint; depth < mContextStack.size(); depth++) {
            string	theString(depth * 2, ' ');
    
            theString += "while ";
            theString += mContextStack[depth];
            
            fprintf(sOutputFile, "%s\n", theString.c_str());
			fprintf(stderr, "%s\n", theString.c_str());
        }
        
        vsnprintf(stringBuffer, kStringBufferSize, inMessage, inArgs);
    
        string	theString(mContextStack.size() * 2, ' ');
        
        theString += stringBuffer;
        
        if(sShowLocations) {
            snprintf(stringBuffer, kStringBufferSize, " (%s:%d)\n", inFile, inLine);
            theString += stringBuffer;
        }
        else
            theString += "\n";
        
        fprintf(sOutputFile, "%s", theString.c_str());
		fprintf(stderr, "%s", theString.c_str());
        
        if(sFlushOutput)
                fflush(sOutputFile);
        
        mMostRecentCommonStackDepth = mContextStack.size();
        mMostRecentlyPrintedStackDepth = mContextStack.size();
    }
}

void TopLevelLogger::flush()
{
	if (sOutputFile)
	{
		fflush(sOutputFile);
	}
}

#if defined(__unix__) || defined(__NetBSD__) || defined(__OpenBSD__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/types.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#endif

extern DirectorySpecifier log_dir;

char g_loggingFileName[256] = "";
const char *loggingFileName()
{
	if (!strlen(g_loggingFileName))
	{
		strncpy(g_loggingFileName, get_application_name().c_str(), 256);
		strncat(g_loggingFileName, " Log.txt", 256 - strlen(g_loggingFileName));
	}
	return g_loggingFileName;
}

static void
InitializeLogging() {
    assert(sOutputFile == NULL);
    FileSpecifier fs = log_dir;
    fs += loggingFileName();

#ifdef __WIN32__
    sOutputFile = _wfopen(utf8_to_wide(fs.GetPath()).c_str(), L"a");
#else
    sOutputFile = fopen(fs.GetPath(), "a");
#endif

    sCurrentLogger = new TopLevelLogger;
    if(sOutputFile != NULL)
    {
	    time_t theTime = time(NULL);
	    const char* theTimeString = ctime(&theTime);
	    fprintf(sOutputFile, "\n-------------------- %s\n\n", theTimeString == NULL ? "(timestamp unavailable)" : theTimeString);
    }
}


// Currently these ignore the domain since domains are effectively not implemented.
void
setLoggingThreshhold(const char* inDomain, int16 inThreshhold) {
        sLoggingThreshhold = inThreshhold;
}


void
setShowLoggingLocations(const char* inDomain, bool inShowLoggingLocations) {
        sShowLocations = inShowLoggingLocations;
}


void
setFlushLoggingOutput(const char* inDomain, bool inFlushOutput) {
        sFlushOutput = inFlushOutput;

        // Flush now for good measure
        if(sFlushOutput && sOutputFile != NULL)
                fflush(sOutputFile);
}


void reset_mml_logging()
{
	// no reset
}

void parse_mml_logging(const InfoTree& root)
{
	BOOST_FOREACH(InfoTree dtree, root.children_named("logging_domain"))
	{
		std::string domain;
		if (!dtree.read_attr("domain", domain) || !domain.size())
			continue;
		
		int16 threshhold;
		if (dtree.read_attr("threshhold", threshhold))
			setLoggingThreshhold(domain.c_str(), threshhold);
		bool locations;
		if (dtree.read_attr("show_locations", locations))
			setShowLoggingLocations(domain.c_str(), locations);
		bool flush;
		if (dtree.read_attr("flush", flush))
			setFlushLoggingOutput(domain.c_str(), flush);
	}
}
