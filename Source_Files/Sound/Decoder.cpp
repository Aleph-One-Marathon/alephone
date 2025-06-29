/*

	Copyright (C) 2007 Gregory Smith
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

	Decodes music and external sounds

*/

#include "Decoder.h"
#include "SndfileDecoder.h"
#include <memory>

using std::unique_ptr;

unique_ptr<StreamDecoder> StreamDecoder::Get(FileSpecifier& File)
{
	unique_ptr<SndfileDecoder> sndfileDecoder(std::make_unique<SndfileDecoder>());
	if (sndfileDecoder->Open(File))
		return sndfileDecoder;

	return 0;
}

Decoder* Decoder::Get(FileSpecifier& File)
{
	unique_ptr<SndfileDecoder> sndfileDecoder(std::make_unique<SndfileDecoder>());
	if (sndfileDecoder->Open(File))
		return sndfileDecoder.release();

	return 0;
}
