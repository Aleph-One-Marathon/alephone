#ifndef __CRC_H
#define __CRC_H

/*
	crc.h
	Sunday, March 5, 1995 6:25:36 PM
	
	Calculate the 32 bit CRC for a given file.

Aug 15, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

class FileSpecifier;
class OpenedFile;

uint32 calculate_crc_for_file(FileSpecifier& File);
uint32 calculate_crc_for_opened_file(OpenedFile& OFile);
uint32 calculate_data_crc(unsigned char *buffer, long length);

#endif
