/*
 *  wad_sdl.cpp - Additional map file handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "wad.h"
#include "tags.h"
#include "game_errors.h"

#include "find_files.h"


/*
 *  Find map file with specified checksum in path
 */

boolean find_wad_file_that_has_checksum(FileSpecifier &matching_file, unsigned long file_type, short path_resource_id, unsigned long checksum)
{
printf("*** find_wad_file_that_has_checksum(%08x), type %c%c%c%c, path %d\n", checksum, file_type >> 24, file_type >> 16, file_type >> 8, file_type, path_resource_id);
	//!!
	return false;
}


/*
 *  Find file with specified modification date in path
 */

boolean find_file_with_modification_date(FileSpecifier &matching_file, unsigned long file_type, short path_resource_id, unsigned long modification_date)
{
printf("*** find_file_with_modification_date(%d), path %d\n", modification_date, path_resource_id);
	//!!
	return false;
}
