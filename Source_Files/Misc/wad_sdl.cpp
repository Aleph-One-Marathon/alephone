/*
 *  wad_sdl.cpp - Additional map file handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"


/*
 *  Find map file with specified checksum in path
 */

bool find_wad_file_that_has_checksum(FileSpecifier &matching_file, int file_type, short path_resource_id, uint32 checksum)
{
printf("*** find_wad_file_that_has_checksum(%08x), type %d, path %d\n", checksum, file_type, path_resource_id);
	//!!
	return false;
}


/*
 *  Find file with specified modification date in path
 */

bool find_file_with_modification_date(FileSpecifier &matching_file, int file_type, short path_resource_id, unsigned long modification_date)
{
printf("*** find_file_with_modification_date(%d), path %d\n", modification_date, path_resource_id);
	//!!
	return false;
}
