/*
 *  single2rsrc.c - Extract resource fork from AppleSingle file
 *
 *  Written in 2000 by Christian Bauer, Public Domain
 */

#include <SDL/SDL.h>
#include <SDL/SDL_endian.h>

#define BUFFER_SIZE 0x10000
static unsigned char buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
	SDL_RWops *fin, *fout;
	long id, fork_start, fork_size;
	int ret = 1, num_entries;
	int resource_fork_found = 0;

	if (argc < 3) {
		printf("Usage: %s <source file> <destination file>\n", argv[0]);
		exit(1);
	}

	SDL_Init(0);
	fin = SDL_RWFromFile(argv[1], "rb");
	if (fin == NULL) {
		perror("Can't open source file");
		exit(1);
	}
	fout = SDL_RWFromFile(argv[2], "wb");
	if (fout == NULL) {
		perror("Can't open destination file");
		SDL_FreeRW(fin);
		exit(1);
	}

	id = SDL_ReadBE32(fin);
	if (id != 0x00051600) {
		printf("Source file is not an AppleSingle file.\n");
		goto quit;
	}

	SDL_RWseek(fin, 0x18, SEEK_SET);
	num_entries = SDL_ReadBE16(fin);
	while (num_entries--) {
		Uint32 id = SDL_ReadBE32(fin);
		Sint32 ofs = SDL_ReadBE32(fin);
		Sint32 len = SDL_ReadBE32(fin);
		if (id == 2) {
			resource_fork_found = 1;
			fork_start = ofs;
			fork_size = len;
		}
	}
	if (!resource_fork_found) {
		printf("Source file doesn't contain a resource fork.\n");
		goto quit;
	}

	SDL_RWseek(fin, fork_start, SEEK_SET);
	while (fork_size) {
		long length = fork_size > BUFFER_SIZE ? BUFFER_SIZE : fork_size;
		SDL_RWread(fin, buffer, 1, length);
		SDL_RWwrite(fout, buffer, 1, length);
		fork_size -= length;
	}

	ret = 0;

quit:
	SDL_FreeRW(fout);
	SDL_FreeRW(fin);
	SDL_Quit();
	return ret;
}
