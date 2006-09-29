/*
 *  dumprsrcmap.cpp - Dump Macintosh resource map
 *
 *  Written in 2001 by Christian Bauer, Public Domain
 */

#include "resource_manager.cpp"
#include "FileHandler_SDL.cpp"
#include "csalerts_sdl.cpp"
#include "Logging.cpp"
#include "XML_ElementParser.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>

// Dummy declarations to avoid link errors
void set_game_error(short a, short b) {}
DirectorySpecifier local_data_dir, preferences_dir, saved_games_dir, recordings_dir;
vector<DirectorySpecifier> data_search_path;

char *csprintf(char *buffer, const char *format, ...)
{
	va_list list;

	va_start(list, format);
	vsprintf(buffer, format, list);
	va_end(list); 
	return buffer;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s FILE\nDump Macintosh resource map\n", argv[0]);
		exit(1);
	}

	// Open file
	FileSpecifier file = argv[1];
	SDL_RWops *rw = open_res_file(file);
	if (rw == NULL) {
		fprintf(stderr, "Can't open resource file\n");
		exit(1);
	}

	// Dump resource map
	res_file_t *r = *cur_res_file_t;
	res_file_t::type_map_t::const_iterator tit = r->types.begin(), titend = r->types.end();
	while (tit != titend) {
		uint32 type = tit->first;
		int num = r->count_resources(type);
		printf("'%c%c%c%c' (0x%08x), %d resources:\n", type >> 24, type >> 16, type >> 8, type, type, num);

		vector<int> ids;
		get_resource_id_list(type, ids);
		vector<int>::const_iterator it = ids.begin(), itend = ids.end();
		while (it != itend) {
			LoadedResource rsrc;
			get_resource(type, *it, rsrc);
			printf("  id %d, size %d\n", *it, rsrc.GetLength());
			rsrc.Unload();
			it++;
		}

		tit++;
	}

	return 0;
}
