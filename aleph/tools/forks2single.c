/*
 *  forks2single.c - Combine data and resource forks to AppleSingle file
 *
 *  Written in 2000 by Christian Bauer, Public Domain
 */

#include <SDL.h>
#include <SDL_endian.h>

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 0x10000
static unsigned char buffer[BUFFER_SIZE];

static void copy_fork(SDL_RWops *fin, SDL_RWops *fout, long size)
{
	while (size) {
		long length = size > BUFFER_SIZE ? BUFFER_SIZE : size;
		SDL_RWread(fin, buffer, 1, length);
		SDL_RWwrite(fout, buffer, 1, length);
		size -= length;
	}
}

int main(int argc, char **argv)
{
	SDL_RWops *fdata, *frsrc, *fout;
	long data_size, rsrc_size, offset;
	int i;

	if (argc < 4) {
		printf("Usage: %s DATA RESOURCE DESTINATION\nCombine data and resource forks to AppleSingle file.\n", argv[0]);
		exit(1);
	}

	// Open files and get fork sizes
	fdata = SDL_RWFromFile(argv[1], "rb");
	if (fdata == NULL) {
		perror("Can't open data file");
		exit(1);
	}
	SDL_RWseek(fdata, 0, SEEK_END);
	data_size = SDL_RWtell(fdata);
	SDL_RWseek(fdata, 0, SEEK_SET);

	frsrc = SDL_RWFromFile(argv[2], "rb");
	if (frsrc == NULL) {
		perror("Can't open resource file");
		exit(1);
	}
	SDL_RWseek(frsrc, 0, SEEK_END);
	rsrc_size = SDL_RWtell(frsrc);
	SDL_RWseek(frsrc, 0, SEEK_SET);

	fout = SDL_RWFromFile(argv[3], "wb");
	if (fout == NULL) {
		perror("Can't open destination file");
		exit(1);
	}

	// Write AppleSingle header
	SDL_WriteBE32(fout, 0x00051600);	// Magic number
	SDL_WriteBE32(fout, 0x00020000);	// Version number
	for (i=0; i<4; i++)
		SDL_WriteBE32(fout, 0);			// Filler
	SDL_WriteBE16(fout, 2);				// Number of entries
	offset = 26 + 12 * 2;
	SDL_WriteBE32(fout, 1);				// Data fork ID
	SDL_WriteBE32(fout, offset);		// Data fork offset
	SDL_WriteBE32(fout, data_size);		// Data fork size
	offset += data_size;
	SDL_WriteBE32(fout, 2);				// Resource fork ID
	SDL_WriteBE32(fout, offset);		// Resource fork offset
	SDL_WriteBE32(fout, rsrc_size);		// Resource fork size

	// Copy forks
	copy_fork(fdata, fout, data_size);
	copy_fork(frsrc, fout, rsrc_size);
	return 0;
}
