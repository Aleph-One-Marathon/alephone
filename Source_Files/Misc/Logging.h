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

 July 13, 2003 (Woody Zenfell):
	Split out the repetitive bits to Logging_gruntwork.h; now generating that with a script
	Now offering logWarningNMT3() and co. for use in non-main threads, for added safety
	New Logging level 'summary'
*/

#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>

enum {
	logFatalLevel	= 0,	// program must exit
	logErrorLevel	= 10,	// can't do something significant
	logWarningLevel	= 20,	// can continue but results could be really screwy
	logAnomalyLevel	= 30,	// can continue, results could be off a little, but no big deal
	logNoteLevel	= 40,	// something worth mentioning
	logSummaryLevel	= 45,	// occasional dumps of aggregate statistics
	logTraceLevel	= 50,	// details of actions and logic
	logDumpLevel	= 60	// values of data etc.
};


class Logger {
public:
	virtual void pushLogContext(const char* inFile, int inLine, const char* inContext, ...);
	virtual void logMessage(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, ...);
	virtual void logMessageNMT(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, ...);

	virtual void pushLogContextV(const char* inFile, int inLine, const char* inContext, va_list inArgList) = 0;
	virtual void popLogContext() = 0;
	virtual void logMessageV(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, va_list inArgList) = 0;
	virtual void flush() = 0;

	virtual ~Logger();
};


// Primitive functions supporting the macros, classes, etc.
Logger* GetCurrentLogger();

// Other functions
void setLoggingThreshhold(const char* inDomain, short inThreshhold); // message appears if its level < inThreshhold
void setShowLoggingLocations(const char* inDomain, bool inShowLocations);	// show file and line?
void setFlushLoggingOutput(const char* inDomain, bool inFlushOutput);	// flush output file after every log message?


class InfoTree;
void parse_mml_logging(const InfoTree& root);
void reset_mml_logging();

// Log file name, for display in error messages
const char *loggingFileName();


// Catch-all domain for use when nobody overrides us
extern const char* logDomain;



// (COMMENTS FOR logError() FAMILY)
// Ease-of-use macros wrap around logMessage.
#define logFatal(...)    (GetCurrentLogger()->logMessage(logDomain, logFatalLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logError(...)    (GetCurrentLogger()->logMessage(logDomain, logErrorLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logWarning(...)  (GetCurrentLogger()->logMessage(logDomain, logWarningLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logAnomaly(...)  (GetCurrentLogger()->logMessage(logDomain, logAnomalyLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logNote(...)     (GetCurrentLogger()->logMessage(logDomain, logNoteLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logSummary(...)  (GetCurrentLogger()->logMessage(logDomain, logSummaryLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logTrace(...)    (GetCurrentLogger()->logMessage(logDomain, logTraceLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logDump(...)     (GetCurrentLogger()->logMessage(logDomain, logDumpLevel, __FILE__, __LINE__, __VA_ARGS__))

// NB! use logError() and co. only in the main thread.  Elsewhere, use logErrorNMT() and co.
// (the NMT versions may have more complex - or missing - implementations to make them safer)
#define logFatalNMT(...)    (GetCurrentLogger()->logMessageNMT(logDomain, logFatalLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logErrorNMT(...)    (GetCurrentLogger()->logMessageNMT(logDomain, logErrorLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logWarningNMT(...)  (GetCurrentLogger()->logMessageNMT(logDomain, logWarningLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logAnomalyNMT(...)  (GetCurrentLogger()->logMessageNMT(logDomain, logAnomalyLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logNoteNMT(...)     (GetCurrentLogger()->logMessageNMT(logDomain, logNoteLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logSummaryNMT(...)  (GetCurrentLogger()->logMessageNMT(logDomain, logSummaryLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logTraceNMT(...)    (GetCurrentLogger()->logMessageNMT(logDomain, logTraceLevel, __FILE__, __LINE__, __VA_ARGS__))
#define logDumpNMT(...)     (GetCurrentLogger()->logMessageNMT(logDomain, logDumpLevel, __FILE__, __LINE__, __VA_ARGS__))

// (COMMENTS FOR logContext() FAMILY)
// Enter a (runtime) logging subcontext; expires at end of block.  Use only within functions!!
// Use only as a statement, e.g. logContext("foo");, never in an expression.  Useless in code like
// if(a) logContext("a is true"); else logContext("b is true");.  For that sort of thing, use
// the class directly - declare an instance of it earlier and call enterContext() in your if().
// Contexts should be phrased like "starting a new game".
// We need to use makeUniqueIdentifier so that __LINE__ gets expanded before concatenation.
#define makeUniqueIdentifier(a, b) a ## b

#define logContext(...)	LogContext makeUniqueIdentifier(_theLogContext,__LINE__)(false, __FILE__, __LINE__, __VA_ARGS__)
#define logContextNMT(...)	LogContext makeUniqueIdentifier(_theLogContext,__LINE__)(true, __FILE__, __LINE__, __VA_ARGS__)


// Class intended for stack-based use for entering/leaving a logging subcontext
class LogContext {
public:
	LogContext(bool inNonMainThread) : contextSet(false), nonMainThread(inNonMainThread) {}

	LogContext(const char* inFile, int inLine, const char* inContext, ...) : contextSet(false), nonMainThread(false) {
		va_list theVarArgs;
		va_start(theVarArgs, inContext);
		enterContextV(inFile, inLine, inContext, theVarArgs);
		va_end(theVarArgs);
	}

	LogContext(bool inNonMainThread, const char* inFile, int inLine, const char* inContext, ...) : contextSet(false), nonMainThread(inNonMainThread) {
		va_list theVarArgs;
		va_start(theVarArgs, inContext);
		enterContextV(inFile, inLine, inContext, theVarArgs);
		va_end(theVarArgs);
	}

	void enterContext(const char* inFile, int inLine, const char* inContext, ...) {
		va_list theVarArgs;
		va_start(theVarArgs, inContext);
		enterContextV(inFile, inLine, inContext, theVarArgs);
		va_end(theVarArgs);
	}

	void enterContextV(const char* inFile, int inLine, const char* inContext, va_list inArgs) {
		if(contextSet)
			leaveContext();
		
			GetCurrentLogger()->pushLogContextV(inFile, inLine, inContext, inArgs);

		contextSet = true;
	}

	void leaveContext() {
			if(contextSet)
				GetCurrentLogger()->popLogContext();

		contextSet = false;
	}

	~LogContext() {
		leaveContext();
	}
	
protected:
	bool contextSet;
	bool nonMainThread;
};



// Not yet complete - idea is let some set of logging operations be "held",
// then whether to log them normally or otherwise (perhaps with a "visibility boost")
// can be decided later.  Intended for, like, if a sub-function tries a lot of different ways
// to succeed, its efforts can be logged in a SubLog.  Typically any small failures etc.
// would not be interesting.  But, if the function as a whole fails, it'd be nice to
// know (what was tried and what the results of each try were).  The idea then is,
// much of the SubLog can be included in a regular Log.
/*
class SubLogger : public Logger {
public:
	virtual void pushLogContext(const char* inFile, int inLine, const char* inContext, ...);
	virtual void popLogContext();
	virtual void logMessage(const char* inDomain, int inLevel, const char* inFile, int inLine, const char* inMessage, ...);
protected:
	struct LogAction {
		enum { ePushContext, ePopContext, eMessage } mType;
		string	mDomain;
		int	mLevel;
		string	mMessage;
		string	mFile;
		int	mLine;
	};
	
	vector<LogAction> mLogActions;
};
*/

#endif // LOGGING_H
