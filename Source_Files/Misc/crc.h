#ifndef __CRC_H
#define __CRC_H

/*
	crc.h
	Sunday, March 5, 1995 6:25:36 PM
	
	Calculate the 32 bit CRC for a given file.
*/

unsigned long calculate_crc_for_file(FileDesc *file);
unsigned long calculate_crc_for_opened_file(short refnum);
unsigned long calculate_data_crc(unsigned char *buffer, long length);

#endif
