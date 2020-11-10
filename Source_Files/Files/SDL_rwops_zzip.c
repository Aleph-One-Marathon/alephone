/*
 *      Copyright (c) 2001 Guido Draheim <guidod@gmx.de>
 *      Use freely under the restrictions of the ZLIB License
 *
 *      (this example uses errno which might not be multithreaded everywhere)
 */

#include "SDL_rwops_zzip.h"
#include <zzip/zzip.h>
#include <string.h> /* strchr */

/* MSVC can not take a casted variable as an lvalue ! */
#define SDL_RWOPS_ZZIP_DATA(_context) \
             ((_context)->hidden.unknown.data1)
#define SDL_RWOPS_ZZIP_FILE(_context)  (ZZIP_FILE*) \
             ((_context)->hidden.unknown.data1)

static Sint64 _zzip_size(SDL_RWops *context)
{
    return -1;
}

static Sint64 _zzip_seek(SDL_RWops *context, Sint64 offset, int whence)
{
    return zzip_seek(SDL_RWOPS_ZZIP_FILE(context), offset, whence);
}

static size_t _zzip_rwread(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
    if (!size) return 0;
    return zzip_read(SDL_RWOPS_ZZIP_FILE(context), ptr, size*maxnum) / size;
}

static size_t _zzip_rwwrite(SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
    return 0; /* ignored */
}

static int _zzip_close(SDL_RWops *context)
{
    if (! context) return 0; /* may be SDL_RWclose is called by atexit */

    zzip_close (SDL_RWOPS_ZZIP_FILE(context));
    SDL_FreeRW (context);
    return 0;
}

SDL_RWops *SDL_RWFromZZIP(const char* file, const zzip_plugin_io_handlers* custom_zzip_io)
{
    register SDL_RWops* rwops;
    register ZZIP_FILE* zzip_file;

#ifdef O_BINARY /*Microsoft extension*/
    const int o_binary = O_BINARY;
#else
    const int o_binary = 0;
#endif

    zzip_file = zzip_open_ext_io (file, O_RDONLY|o_binary, 0, NULL, custom_zzip_io);
    if (! zzip_file) return 0;

    rwops = SDL_AllocRW ();
    if (! rwops) { errno=ENOMEM; zzip_close (zzip_file); return 0; }

    SDL_RWOPS_ZZIP_DATA(rwops) = zzip_file;
    rwops->size = _zzip_size;
    rwops->read = _zzip_rwread;
    rwops->write = _zzip_rwwrite;
    rwops->seek = _zzip_seek;
    rwops->close = _zzip_close;
    return rwops;
}
