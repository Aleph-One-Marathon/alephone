/*
 *  single2forks.c - Extract data and resource forks from AppleSingle file
 *
 *  Written in 2000 by Christian Bauer, Public Domain
 */

#include <SDL.h>
#include <SDL_endian.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 0x10000
static unsigned char buffer[BUFFER_SIZE];

static void extract_fork(SDL_RWops *fin, int req_id, const char *file_name, const char *fork_name)
{
	long id, fork_start, fork_size;
	int num_entries, fork_found;
	SDL_RWops *fout;

	// Look for fork in source file
	SDL_RWseek(fin, 0x18, SEEK_SET);
	num_entries = SDL_ReadBE16(fin);
	while (num_entries--) {
		Uint32 id = SDL_ReadBE32(fin);
		Sint32 ofs = SDL_ReadBE32(fin);
		Sint32 len = SDL_ReadBE32(fin);
		if (id == req_id) {
			fork_found = 1;
			fork_start = ofs;
			fork_size = len;
		}
	}
	if (!fork_found) {
		fprintf(stderr, "Warning: source file doesn't contain a %s fork.\n", fork_name);
		return;
	}

	// Found, open destination file
	fout = SDL_RWFromFile(file_name, "wb");
	if (fout == NULL) {
		perror("Can't open destination file");
		exit(1);
	}

	// Copy fork
	SDL_RWseek(fin, fork_start, SEEK_SET);
	while (fork_size) {
		long length = fork_size > BUFFER_SIZE ? BUFFER_SIZE : fork_size;
		SDL_RWread(fin, buffer, 1, length);
		SDL_RWwrite(fout, buffer, 1, length);
		fork_size -= length;
	}
	SDL_FreeRW(fout);
}

int main(int argc, char **argv)
{
	char *data_name = NULL, *rsrc_name = NULL;
	SDL_RWops *fin;
	Uint32 id, version;

	if (argc < 2 || argc > 4) {
		printf("Usage: %s SOURCE [DATA [RESOURCE]]\nExtract data and resource forks from AppleSingle file.\n", argv[0]);
		exit(1);
	}

	// If the destination file names are not specified, the data fork
	// will go to <name>.data and the resource fork to <name>.rsrc
	if (argc > 2) {
		data_name = argv[2];
		if (argc > 3)
			rsrc_name = argv[3];
	}
	if (data_name == NULL) {
		data_name = malloc(strlen(argv[1]) + 6);
		strcpy(data_name, argv[1]);
		strcat(data_name, ".data");
	}
	if (rsrc_name == NULL) {
		rsrc_name = malloc(strlen(argv[1]) + 6);
		strcpy(rsrc_name, argv[1]);
		strcat(rsrc_name, ".rsrc");
	}

	// Open source file
	SDL_Init(0);
	fin = SDL_RWFromFile(argv[1], "rb");
	if (fin == NULL) {
		perror("Can't open source file");
		exit(1);
	}
	id = SDL_ReadBE32(fin);
	version = SDL_ReadBE32(fin);
	if (id != 0x00051600 || version != 0x00020000) {
		fprintf(stderr, "Source file is not a version 2 AppleSingle file.\n");
		exit(1);
	}

	// Extract data and resource forks
	extract_fork(fin, 1, data_name, "data");
	extract_fork(fin, 2, rsrc_name, "resource");
	return 0;
}
