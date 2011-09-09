/*
	preprocess_map_shared.cpp

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

Feb. 3, 2003 (Woody Zenfell):
        Created.  Support for saving in netgames.
*/

#include "cseries.h"
#include "FileHandler.h"
#include "game_wad.h"
#include "interface.h"
#include "TextStrings.h"	// TS_GetCString()
#include "map.h"	// static_world->level_name
#include "shell.h"	// screen_printf()

#include <ctype.h>	// I get the impression this is part of Standard C


// Hmm maybe some of this stuff should be in the File code instead.

// Yuck.  But we don't seem to have this available anywhere else.
enum {
    kMaxFilenameChars = 
#ifdef mac
    31
#else
    255
#endif
};

// This is a strncpy that leaves out all characters not alphanumeric or ' ' (space).
// Returns the strlen() of the finished output string.
static int32
strncpy_filename_friendly(char* outString, const char* inString, int32 inOutputStringBufferSize) {
        assert(inOutputStringBufferSize > 0);
        assert(outString != NULL);
        assert(inString != NULL);
        
        char* theOutputChar = outString;
        char* theOutNull = outString + inOutputStringBufferSize - 1;
        while(theOutputChar < theOutNull && *inString != '\0') {
                if(isalnum(*inString) || *inString == ' ')
                        *theOutputChar++ = *inString;
                inString++;
        }

        memset(theOutputChar, 0, (theOutNull - theOutputChar) + 1);

        return static_cast<int>(theOutputChar - outString);
}



// This tries to find a filename (in the same directory as inBaseName) based on 'inBaseName' but that is not yet used.
// The current logic tries inBaseName first, then 'inBaseName 2', then 'inBaseName 3', and so on.
// The resulting name will have no more than inMaxNameLength actual characters.
// Returns whether it was successful (the current logic either returns true or loops forever; fortunately, the
// probability of looping forever is really, REALLY tiny; the directory would have to contain every possible variant).
static bool
make_nonconflicting_filename_variant(/*const*/ FileSpecifier& inBaseName, FileSpecifier& outNonconflictingName, size_t inMaxNameLength) {
	FileSpecifier theNameToTry;
	DirectorySpecifier theDirectory;
	inBaseName.ToDirectory(theDirectory);
        
        char	theBaseNameString[256]; // XXX I don't like this, but the SDL code already imposes this maxlen throughout.

        inBaseName.GetName(theBaseNameString);
        size_t	theBaseNameLength = strlen(theBaseNameString);

        char	theNameToTryString[256];
        char	theVariantSuffix[32]; // way more than enough

        unsigned int	theVariant = 0;
        size_t	theSuffixLength;
        size_t	theBaseNameLengthToUse;
        
        bool	theVariantIsAcceptable = false;
        
        while(!theVariantIsAcceptable) {
                theVariant++;
                if(theVariant == 1) {
                        theVariantSuffix[0] = '\0';
                        theSuffixLength = 0;
                }
                else {
                        theSuffixLength = sprintf(theVariantSuffix, " %d", theVariant);
                }
                
                assert(theSuffixLength <= inMaxNameLength);
                
                if(theSuffixLength + theBaseNameLength > inMaxNameLength)
                        theBaseNameLengthToUse = inMaxNameLength - theSuffixLength;
                else
                        theBaseNameLengthToUse = theBaseNameLength;
                
                sprintf(theNameToTryString, "%.*s%s", (int) theBaseNameLengthToUse, theBaseNameString, theVariantSuffix);
        
                theNameToTry.FromDirectory(theDirectory);
#if defined(mac) || defined(SDL_RFORK_HACK)
                // Note: SetName() currently ignores the 'type' argument, so I feel
                // little compulsion to try to figure it out.
                theNameToTry.SetName(theNameToTryString,NONE);
#else
                theNameToTry.AddPart(theNameToTryString);
#endif
                if(!theNameToTry.Exists())
                        theVariantIsAcceptable = true;
        }
        
        if(theVariantIsAcceptable) {
                outNonconflictingName = theNameToTry;
        }
        
        return theVariantIsAcceptable;
}



// Analogous to 'save_game()', but does not present any dialog to the user, and reports the results
// using screen_printf().  If inOverwriteRecent is set, it will save over the most recently saved
// or restored game (if possible).  If inOverwriteRecent is not set, or if there is no recent game
// to save over, it will pick a new, nonconflicting filename and save to it.
// Returns whether save was successful.
bool
save_game_full_auto(bool inOverwriteRecent) {
        bool createNewFile = !inOverwriteRecent;

        FileSpecifier theRecentSavedGame;
        get_current_saved_game_name(theRecentSavedGame);

        char theSavedGameName[256]; // XXX

        // If we're supposed to overwrite, change our minds if we seem to have no 'existing file'.
        if(!createNewFile) {
                theRecentSavedGame.GetName(theSavedGameName);
			char theDefaultGameName[256];
			getcstr(theDefaultGameName, strFILENAMES, filenameDEFAULT_SAVE_GAME);
                if(strcmp(theSavedGameName, theDefaultGameName) == 0)
                        createNewFile = true;
        }
        
        // Make up a filename (currently based on level name)
        if(createNewFile) {
                if(strncpy_filename_friendly(theSavedGameName, static_world->level_name, kMaxFilenameChars) <= 0)
                        strcpy(theSavedGameName, "Automatic Save");

                DirectorySpecifier theDirectory;
                theRecentSavedGame.ToDirectory(theDirectory);
                theRecentSavedGame.FromDirectory(theDirectory);
#if defined(mac) || defined(SDL_RFORK_HACK)
                // Note: SetName() currently ignores the 'type' argument, so I feel
                // little compulsion to try to figure it out.
                theRecentSavedGame.SetName(theSavedGameName,NONE);
#else
                theRecentSavedGame.AddPart(theSavedGameName);
#endif
        }
                
        
        FileSpecifier theOutputFile;
        
        if(createNewFile)
                make_nonconflicting_filename_variant(theRecentSavedGame, theOutputFile, kMaxFilenameChars);
        else
                theOutputFile = theRecentSavedGame;
        
        bool successfulSave = save_game_file(theOutputFile);

        theOutputFile.GetName(theSavedGameName);
        
        if(successfulSave) {
                screen_printf("%s saved game '%s'", createNewFile ? "Created new" : "Replaced existing", theSavedGameName);
                // play a sound?
        }
        else {
                screen_printf("Unable to save game as '%s'", theSavedGameName);
                // play a sound?
        }

        return successfulSave;
}
