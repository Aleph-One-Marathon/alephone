/*
 *  dumpwad.cpp - Dump contents of Marathon wad file
 *
 *  Written in 2001 by Christian Bauer, Public Domain
 */

#include "wad.cpp"
#include "resource_manager.cpp"
#include "FileHandler_SDL.cpp"
#include "csalerts_sdl.cpp"
#include "crc.cpp"
#include "map.h"
#include "Packing.cpp"
#include "Logging.cpp"
#include "XML_ElementParser.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Dummy declarations to avoid link errors
void set_game_error(short a, short b) {}
void dprintf(const char *a, ...) {}
void alert_user(short a, short b, short c, OSErr d) {}
DirectorySpecifier local_data_dir, preferences_dir, saved_games_dir, recordings_dir;
vector<DirectorySpecifier> data_search_path;
char temporary[256];

char *csprintf(char *buffer, const char *format, ...)
{
	va_list list;

	va_start(list, format);
	vsprintf(buffer, format, list);
	va_end(list); 
	return buffer;
}

void *level_transition_malloc(size_t size)
{
	return malloc(size);
}

static void unpack_directory_data(uint8 *Stream, directory_data *Objects)
{
	uint8* S = Stream;
	directory_data* ObjPtr = Objects;

	StreamToValue(S,ObjPtr->mission_flags);
	StreamToValue(S,ObjPtr->environment_flags);
	StreamToValue(S,ObjPtr->entry_point_flags);
	StreamToBytes(S,ObjPtr->level_name,LEVEL_NAME_LENGTH);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s FILE\nDump contents of Marathon wad file\n", argv[0]);
		exit(1);
	}

	// Open file
	FileSpecifier name = argv[1];
	OpenedFile f;
	if (!open_wad_file_for_reading(name, f)) {
		fprintf(stderr, "Can't open wad file\n");
		exit(1);
	}

	// Read wad header
	wad_header h;
	if (!read_wad_header(f, &h)) {
		fprintf(stderr, "Error reading wad header\n");
		exit(1);
	}

	// Dump wad header
	printf("Header:\n");
	printf("  version %d, data version %d, file name '%s'\n", h.version, h.data_version, h.file_name);
	printf("  checksum 0x%08x, parent checksum 0x%08x, wad count %d\n", h.checksum, h.parent_checksum, h.wad_count);
	printf("  directory data size %d, entry header size %d, directory entry size %d\n", h.application_specific_directory_data_size, h.entry_header_size, h.directory_entry_base_size);

	// Read directory data
	void *dir_data = read_directory_data(f, &h);
	if (dir_data == NULL) {
		fprintf(stderr, "Error reading wad directory\n");
		exit(1);
	}

	int base_entry_size = get_directory_base_length(&h);
	if (base_entry_size > SIZEOF_directory_entry)
		base_entry_size = SIZEOF_directory_entry;

	// Dump directory
	for (int i=0; i<h.wad_count; i++) {
		printf("\nDirectory entry %d:\n", i);

		// Read and dump directory entry
		long offset = calculate_directory_offset(&h, i);
		uint8 buffer[MAX(SIZEOF_old_directory_entry, SIZEOF_directory_entry)];
		if (!read_from_file(f, offset, buffer, base_entry_size)) {
			fprintf(stderr, "Error reading directory entry %d\n", i);
			exit(1);
		}
		directory_entry e;
		switch (base_entry_size) {
			case SIZEOF_old_directory_entry:
				unpack_old_directory_entry(buffer, (old_directory_entry *)&e, 1);
				e.index = i;
				break;
			case SIZEOF_directory_entry:
				unpack_directory_entry(buffer, &e, 1);
				break;
			default:
				fprintf(stderr, "Unrecognized base entry length\n");
				exit(1);
		}
		printf("  data size %d, index %d\n", e.length, e.index);

		// Dump application directory data (level information)
		if (h.application_specific_directory_data_size == SIZEOF_directory_data) {
			uint8 *p = (uint8 *)get_indexed_directory_data(&h, i, dir_data);
			directory_data d;
			unpack_directory_data(p, &d);

			printf("  level name '%s'\n", d.level_name);
			printf("  mission flags 0x%08x:\n", d.mission_flags);
			if (d.mission_flags == 0)
				printf("    none\n");
			else {
				if (d.mission_flags & _mission_extermination)
					printf("    extermination\n");
				if (d.mission_flags & _mission_exploration)
					printf("    exploration\n");
				if (d.mission_flags & _mission_retrieval)
					printf("    retrieval\n");
				if (d.mission_flags & _mission_repair)
					printf("    repair\n");
				if (d.mission_flags & _mission_rescue)
					printf("    rescue\n");
			}
			printf("  environment flags 0x%08x:\n", d.environment_flags);
			if (d.environment_flags == 0)
				printf("    none\n");
			else {
				if (d.environment_flags & _environment_vacuum)
					printf("    vacuum\n");
				if (d.environment_flags & _environment_magnetic)
					printf("    magnetic\n");
				if (d.environment_flags & _environment_rebellion)
					printf("    rebellion\n");
				if (d.environment_flags & _environment_low_gravity)
					printf("    low gravity\n");
			}
			printf("  entry point flags 0x%08x:\n", d.entry_point_flags);
			if (d.entry_point_flags == 0)
				printf("    none\n");
			else {
				if (d.entry_point_flags & _single_player_entry_point)
					printf("    single player\n");
				if (d.entry_point_flags & _multiplayer_cooperative_entry_point)
					printf("    multiplayer cooperative\n");
				if (d.entry_point_flags & _multiplayer_carnage_entry_point)
					printf("    multiplayer carnage\n");
				if (d.entry_point_flags & _capture_the_flag_entry_point)
					printf("    capture the flag\n");
				if (d.entry_point_flags & _king_of_hill_entry_point)
					printf("    king of the hill\n");
				if (d.entry_point_flags & _defense_entry_point)
					printf("    defense\n");
				if (d.entry_point_flags & _rugby_entry_point)
					printf("    rugby\n");
			}
		}

		// Read tags
		wad_data *w = read_indexed_wad_from_file(f, &h, e.index, true);
		if (w == NULL) {
			fprintf(stderr, "Error reading wad %d\n", i);
			exit(1);
		}

		// Dump tags
		printf("\n  %d tags:\n", w->tag_count);
		for (int t=0; t<w->tag_count; t++) {
			WadDataType tag = w->tag_data[t].tag;
			printf("    '%c%c%c%c' (0x%08x), size %ld\n", tag >> 24, tag >> 16, tag >> 8, tag, tag, w->tag_data[t].length);
		}
		free_wad(w);
	}

	return 0;
}
