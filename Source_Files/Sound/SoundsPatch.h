#ifndef SOUND_PATCH_H
#define SOUND_PATCH_H

/*
SOUND_PATCH.H

	Copyright (C) 2026 by Gregory Smith
 
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

 */

#include <map>
#include <memory>

#include "BStream.h"
#include "SoundFile.h"

class FileHandler;
class SoundDefinitionPatch;

class SoundsPatches {
public:
	bool add(FileSpecifier& file_specifier);
	bool add(BIStreamBE& stream);

	SoundDefinition* get_definition(int source, int sound_index);
	std::shared_ptr<SoundData> get_sound_data(SoundDefinition* definition, int permutation);

	void clear();

private:
	std::map<std::pair<int, int>, SoundDefinitionPatch> definition_patches;
};

extern SoundsPatches sounds_patches;

#endif
