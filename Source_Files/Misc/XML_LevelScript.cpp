/*
	Support for XML scripts in map files
	by Loren Petrich,
	April 16, 2000

	The reason for a separate object is that it will be necessary to execute certain commands
	only on certain levels.
*/

#include "cseries.h"
#include "game_wad.h"
#include "GrowableList.h"
#include "XML_DataBlock.h"
#include "XML_LevelScript.h"
#include "XML_ParseTreeRoot.h"
#include "scripting.h"


// The "command" is an instruction to process a file/resource in a certain sort of way
struct LevelScriptCommand
{
	// Types of commands
	enum {
		MML,
		Pfhortran,
		Music,
		Screen,
		Sound
	};
	int Type;
	
	enum {
		UnsetResource = 0
	};
	
	// Where to read the command data from:
	
	// This is a MacOS resource ID
	short RsrcID;
	
	bool RsrcPresent() {return RsrcID != UnsetResource;}
	
	// WIll need a filename to read
	
	LevelScriptCommand(): RsrcID(UnsetResource) {}
};

struct LevelScriptHeader
{
	// Special pseudo-levels: restoration of previous parameters and defaults for this map file
	enum {
		Restore = -2,
		Default = -1
	};
	int Level;
	
	GrowableList<LevelScriptCommand> Commands;
	
	// Copy over:
	LevelScriptHeader& operator=(LevelScriptHeader& LSHdr)
	{
		Level = LSHdr.Level;
		
		Commands.ResetLength();
		
		for (int k=0; k<LSHdr.Commands.GetLength(); k++)
			Commands.Add(LSHdr.Commands[k]);
		
		return *this;
	}
	
	LevelScriptHeader(): Level(Default) {}
	LevelScriptHeader(LevelScriptHeader& LSHdr) {*this = LSHdr;}
};

// Scripts for current map file
static GrowableList<LevelScriptHeader> LevelScripts;

// Current script for adding commands to
static LevelScriptHeader *CurrScriptPtr = NULL;


// The level-script parsers are separate from the main MML ones,
// because they operate on per-level data.

static XML_DataBlock LSXML_Loader;
static XML_ElementParser LSRootParser("");
static XML_ElementParser LevelScriptSetParser("marathon_levels");

// Get parsers for other stuff
static XML_ElementParser *LevelScript_GetParser();
static XML_ElementParser *DefaultLevelScript_GetParser();
static XML_ElementParser *RestoreLevelScript_GetParser();


// This is for searching for a script and running it -- works for pseudo-levels
static void GeneralRunScript(int LevelIndex);


static void SetupLSParseTree()
{
	// Don't set up more than once!
	static bool WasSetUp = false;
	if (WasSetUp)
		return;
	else
		WasSetUp = true;
	
	LSRootParser.AddChild(&LevelScriptSetParser);
	
	LevelScriptSetParser.AddChild(LevelScript_GetParser());
	LevelScriptSetParser.AddChild(DefaultLevelScript_GetParser());
	LevelScriptSetParser.AddChild(RestoreLevelScript_GetParser());
}


// Loads all those in resource 128 in a map file (or some appropriate equivalent)
void LoadLevelScripts(FileSpecifier& MapFile)
{
	// Get rid of the previous level script
	LevelScripts.ResetLength();
	
	// Lazy setup of XML parsing definitions
	SetupLSParseTree();
	LSXML_Loader.CurrentElement = &LSRootParser;

	OpenedResourceFile OFile;
	if (!MapFile.Open(OFile)) return;
	
	// The script is stored at a special resource ID;
	// simply quit if it could not be found
	LoadedResource ScriptRsrc;
	if (!OFile.Get('TEXT',128,ScriptRsrc)) return;
	
	// Load the script
	LSXML_Loader.ParseData((char *)ScriptRsrc.GetPointer(),ScriptRsrc.GetLength());
}


// Runs a script for some level; loads Pfhortran,
// runs level-specific MML...
void RunLevelScript(int LevelIndex)
{
	GeneralRunScript(LevelScriptHeader::Default);
	GeneralRunScript(LevelIndex);
}

// Intended for restoring old parameter values, because MML sets values at a variety
// of different places, and it may be easier to simply set stuff back to defaults
// by including those defaults in the script.
void RunRestorationScript()
{
	GeneralRunScript(LevelScriptHeader::Restore);
}

// Search for level script and then run it
void GeneralRunScript(int LevelIndex)
{
	CurrScriptPtr = NULL;
	for (LevelScriptHeader *ScriptIter = LevelScripts.Begin(); ScriptIter < LevelScripts.End(); ScriptIter++)
	{
		if (ScriptIter->Level == LevelIndex)
		{
			CurrScriptPtr = ScriptIter;
			break;
		}
	}
	if (!CurrScriptPtr) return;

	OpenedResourceFile OFile;
	FileSpecifier& MapFile = get_map_file();
	if (!MapFile.Open(OFile)) return;
	
	for (int k=0; k<CurrScriptPtr->Commands.GetLength(); k++)
	{
		LevelScriptCommand& Cmd = CurrScriptPtr->Commands[k];
		
		// Data to parse (either MML or Pfhortran)
		char *Data = NULL;
		int DataLen = 0;
		
		// First, try to load a resource (only for scripts)
		LoadedResource ScriptRsrc;
		switch(Cmd.Type)
		{
		case LevelScriptCommand::MML:
		case LevelScriptCommand::Pfhortran:
			if (Cmd.RsrcPresent() && OFile.Get('TEXT',Cmd.RsrcID,ScriptRsrc))
			{
				Data = (char *)ScriptRsrc.GetPointer();
				DataLen = ScriptRsrc.GetLength();
			}
		}
		
		switch(Cmd.Type)
		{
		case LevelScriptCommand::MML:
			{
				// Skip if not loaded
				if (Data == NULL || DataLen <= 0) break;
				
				// Set to the MML root parser
				LSXML_Loader.CurrentElement = &RootParser;
				LSXML_Loader.ParseData(Data,DataLen);
			}
			break;
		
		case LevelScriptCommand::Pfhortran:
			{
			}
			break;
		
		}
	}
}


// Generalized parser for level-script commands; they all use the same format,
// but will be distinguished by their names
class XML_LSCommandParser: public XML_ElementParser
{
	// Need to get an object ID (either resource ID or filename)
	bool ObjectWasFound;
	
	// New script command to compose	
	LevelScriptCommand Cmd;
	
public:
	bool Start() {ObjectWasFound = false; return true;}
	bool AttributesDone();

	// For grabbing the level ID
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_LSCommandParser(char *_Name, int _CmdType): XML_ElementParser(_Name) {Cmd.Type = _CmdType;}
};


bool XML_LSCommandParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"resource") == 0)
	{
		short RsrcID;
		if (ReadBoundedNumericalValue(Value,"%hd",RsrcID,short(0),short(SHRT_MAX)))
		{
			Cmd.RsrcID = RsrcID;
			ObjectWasFound = true;
			return true;
		}
		else
			return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_LSCommandParser::AttributesDone()
{
	if (!ObjectWasFound) return false;
	
	assert(CurrScriptPtr);
	CurrScriptPtr->Commands.Add(Cmd);
	
	return true;
}

static XML_LSCommandParser MMLParser("mml",LevelScriptCommand::MML);
static XML_LSCommandParser PfhortranParser("pfhortran",LevelScriptCommand::Pfhortran);

static void AddScriptCommands(XML_ElementParser& ElementParser)
{
	ElementParser.AddChild(&MMLParser);
	ElementParser.AddChild(&PfhortranParser);
}


// Generalized parser for level scripts; for also parsing default and restoration scripts

class XML_GeneralLevelScriptParser: public XML_ElementParser
{
protected:
	// Tell the level script and the parser for its contents what level we are currently doing
	void SetLevel(short Level);
	
public:
	
	XML_GeneralLevelScriptParser(const char *_Name): XML_ElementParser(_Name) {}
};

void XML_GeneralLevelScriptParser::SetLevel(short Level)
{
	// Scan for current level
	CurrScriptPtr = NULL;
	for (LevelScriptHeader *ScriptIter = LevelScripts.Begin(); ScriptIter < LevelScripts.End(); ScriptIter++)
	{
		if (ScriptIter->Level == Level)
		{
			CurrScriptPtr = ScriptIter;
			break;
		}
	}
	
	// Not found, so add it
	if (!CurrScriptPtr)
	{
		LevelScriptHeader NewHdr;
		NewHdr.Level = Level;
		LevelScripts.Add(NewHdr);
		CurrScriptPtr = LevelScripts.RevBegin();
	}
}


// For setting up scripting for levels in general
class XML_LevelScriptParser: public XML_GeneralLevelScriptParser
{
	// Need to get a level ID
	bool LevelWasFound;
	
public:
	bool Start() {LevelWasFound = false; return true;}
	bool AttributesDone() {return LevelWasFound;}

	// For grabbing the level ID
	bool HandleAttribute(const char *Tag, const char *Value);
	
	XML_LevelScriptParser(): XML_GeneralLevelScriptParser("level") {}
};


bool XML_LevelScriptParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"index") == 0)
	{
		short Level;
		if (ReadBoundedNumericalValue(Value,"%hd",Level,short(0),short(SHRT_MAX)))
		{
			SetLevel(Level);
			LevelWasFound = true;
			return true;
		}
		else
			return false;
	}
	UnrecognizedTag();
	return false;
}

static XML_LevelScriptParser LevelScriptParser;


// For setting up scripting for special pseudo-levels: the default and the restore
class XML_SpecialLevelScriptParser: public XML_GeneralLevelScriptParser
{
	short Level;
	
public:
	bool Start() {SetLevel(Level); return true;}
	
	XML_SpecialLevelScriptParser(char *_Name, short _Level): XML_GeneralLevelScriptParser(_Name), Level(_Level) {}
};

static XML_SpecialLevelScriptParser
	DefaultScriptParser("default",LevelScriptHeader::Default),
	RestoreScriptParser("restore",LevelScriptHeader::Restore);


// XML-parser support
XML_ElementParser *LevelScript_GetParser()
{
	AddScriptCommands(LevelScriptParser);

	return &LevelScriptParser;
}
XML_ElementParser *DefaultLevelScript_GetParser()
{
	AddScriptCommands(DefaultScriptParser);

	return &DefaultScriptParser;
}
XML_ElementParser *RestoreLevelScript_GetParser()
{
	AddScriptCommands(RestoreScriptParser);

	return &RestoreScriptParser;
}
