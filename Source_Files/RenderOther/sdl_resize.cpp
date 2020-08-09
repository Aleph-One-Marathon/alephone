/*
 * This code by David Olsen was released into the public domain at:
 *
 *    https://sourceforge.net/projects/sdlresize/
 *
 * Unless otherwise stated, modifications to this source file by
 * Jeremiah Morris or other Aleph One developers are also released
 * into the public domain.
 */

#if defined(_MSC_VER)
#define NOMINMAX
#include <algorithm>
#endif

#include <cmath>
#include <vector>

#include <SDL.h>

#include "sdl_resize.h"

//code adapted by David Olsen from Lanczos filtering article on wikipedia.org

#ifndef M_PI
#define M_PI    3.14159265359
#endif

static inline double Lanczos(double x, int Radius)
{
  if (x == 0.0) return 1.0;
  if (x <= -Radius || x >= Radius) return 0.0;
  double tmp = x * M_PI;
  return Radius * std::sin(tmp) * std::sin(tmp / Radius) / (tmp * tmp);
}

static inline void Resample(SDL_Surface * src, SDL_Surface * dst, int filter)
{
    const double blur = 1.0;
    double factor  = dst->w / (double)src->w;
    double scale   = std::min(factor, 1.0) / blur;
    int FilterRadius = filter;
    if (filter < 1 )
        FilterRadius = 1;
    if (filter > 3) //automatically determine fastest filter setting
    {
        FilterRadius = 3;
        if (scale < 0.67) FilterRadius = 2;
        if (scale <= 0.5) FilterRadius = 1;        
    }
    double support = FilterRadius / scale; 

    std::vector<double> contribution_x(std::min((size_t)src->w, 5+(size_t)(2*support)));    
    /* 5 = room for rounding up in calculations of start, stop and support */

    Uint32 ** temp = new Uint32 * [src->h]; //array of source->height * dest->width
    for (int i = 0 ; i < src->h; i++)
        temp[i] = new Uint32 [dst->w];

    if (support <= 0.5) { support = 0.5 + 1E-12; scale = 1.0; }

    for (int x = 0; x < dst->w; ++x)
    {
        double center = (x + 0.5) / factor;
        size_t start = (size_t)std::max(center - support + 0.5, (double)0);
        size_t stop  = (size_t)std::min(center + support + 0.5, (double)src->w);
        double density = 0.0;
        size_t nmax = stop - start;
        double s = start - center + 0.5;
        double point[4] = {0,0,0,0};
        Uint8 v;
        double diff;

        for (int y = 0; y < src->h; y++)
        {                        
            for (size_t n = 0; n < nmax; ++n)
            {
                if (y == 0)
                { //only come up with the contribution list once per column.
                    contribution_x[n] = Lanczos (s * scale, FilterRadius);                
                    density += contribution_x[n];
                    s++;
                }
                //it MUST be a 32-bit surface for following code to work correctly
                Uint8 * p = (Uint8 *)src->pixels + y * src->pitch + (start+n) * 4;
                for (int c = 0; c < 4; c++)
                    point[c] += p[c] * contribution_x[n];
            }
            /* Normalize. Truncate to Uint8 values. Place in temp array*/
            Uint8 * p = (Uint8 *)&temp[y][x];
            for (size_t c = 0; c < 4; c++)
            {
                if (density != 0.0 && density != 1.0)
                    point[c] /= density;
                if (point[c] < 0)
                    point[c] = 0;
                if (point[c] > 255)
                    point[c] = 255;
	            v = (Uint8) point[c];
	            diff = point[c] - (double)v;
	            if (diff < 0)
		            diff = -diff;
	            if (diff >= 0.5)
                    v++;
	            p[c] = v;
                point[c] = 0; //reset value for next loop
            }
        }
    }

    factor  = dst->h / (double)src->h;
    scale   = std::min(factor, 1.0) / blur;
    if (filter > 3) //automatically determine fastest filter setting
    {
        FilterRadius = 3;
        if (scale < 0.67) FilterRadius = 2;
        if (scale <= 0.5) FilterRadius = 1;
    }
    support = FilterRadius / scale;

    std::vector<double> contribution_y(std::min((size_t)src->h, 5+(size_t)(2*support)));

    if (support <= 0.5) { support = 0.5 + 1E-12; scale = 1.0; }

    for (int y = 0; y<dst->h; ++y)
    {
        double center = (y + 0.5) / factor;
        size_t start = (size_t)std::max(center - support + 0.5, (double)0);
        size_t stop  = (size_t)std::min(center + support + 0.5, (double)src->h);
        double density = 0.0;
        size_t nmax = stop-start;
        double s = start - center+0.5;
        double point[4] = {0,0,0,0};
        Uint8 v;
        double diff;
        
        for (int x=0; x<dst->w; x++)
        {    
            for (size_t n=0; n<nmax; ++n)
            {
                if (x == 0)
                {
                    contribution_y[n] = Lanczos(s * scale, FilterRadius);
                    density += contribution_y[n];
                    s++;
                }
                Uint8 * p = (Uint8 *)&temp[start+n][x];
                for (int c = 0; c < 4; c++)
                    point[c] += p[c] * contribution_y[n];
            }
            //destination must also be a 32 bit surface for this to work!
            Uint8 * p = (Uint8 *)dst->pixels + y * dst->pitch + x * 4;
            for (size_t c = 0; c < 4; c++)
            {
                if (density != 0.0 && density != 1.0)
                    point[c] /= density;
                if (point[c] < 0)
                    point[c] = 0;
                if (point[c] > 255)
                    point[c] = 255;
                v = (Uint8) point[c];
	            diff = point[c] - (double)v;
	            if (diff < 0)
		            diff = -diff;
	            if (diff >= 0.5)
			        v++;
	            p[c] = v;
                point[c] = 0;
            }
        }
    }

    //free the temp array, so we don't leak any memory
    for (int i = 0 ; i < src->h; i++)
        delete [] temp[i];
    delete [] temp;
}

static inline Uint32 get_pixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp)
    {
        case 1: return *p;
        case 2: return *(Uint16 *)p;
        case 3: if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                    return p[0] << 16 | p[1] << 8 | p[2];
                else
                    return p[0] | p[1] << 8 | p[2] << 16;
        case 4: return *(Uint32 *)p;
        default: return 0;       /* shouldn't happen, but avoids warnings */
    }
}

static inline bool has_alpha(SDL_Surface * src)
{    
    Uint8 r, g, b, a;
    bool is_alpha = false;

    if SDL_MUSTLOCK(src) SDL_LockSurface(src);
       
    for (int x = 0; x < src->w; x++)
        for (int y = 0; y < src->h; y++)
        {	
            SDL_GetRGBA(get_pixel(src, x, y), src->format, &r, &g, &b, &a);
            if (a != SDL_ALPHA_OPAQUE) 
            {
                is_alpha = true; 
                x = src->w; 
                break;
            }
        }
    
    if SDL_MUSTLOCK(src) SDL_UnlockSurface(src);

    return is_alpha;
}

SDL_Surface* SDL_Resize(SDL_Surface *src, float factor, bool free, int filter)
{
    if (factor > 100.0f) factor = 100.0f; //let's be reasonable...
    int new_w = (int)(src->w * factor),
        new_h = (int)(src->h * factor);
    if (new_w < 1) new_w = 1;
    if (new_h < 1) new_h = 1;

    return SDL_Resize(src, new_w, new_h, free, filter);
}

SDL_Surface* SDL_Resize(SDL_Surface *src, int new_w, int new_h, bool free, int filter)
{
    SDL_Surface * dst;
    bool is_alpha = has_alpha(src);    

    if (src->w == new_w && src->h == new_h)
    {
        //No change in size.
        return src;
    }

    Uint32 rmask = 0x000000ff,
        gmask = 0x0000ff00,
        bmask = 0x00ff0000,
        amask = 0xff000000;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #endif

    dst = SDL_CreateRGBSurface(0, new_w, new_h, 32, rmask, gmask, bmask, amask);
    SDL_Surface * temp = SDL_ConvertSurface(src,dst->format,0);

    Resample(temp,dst,filter);
	
	SDL_FreeSurface(temp);
	temp = SDL_ConvertSurface(dst,src->format,0);
	SDL_FreeSurface(dst);
    if (is_alpha)
	{
		SDL_SetSurfaceBlendMode(temp, SDL_BLENDMODE_BLEND);
	}

	if (free)
		SDL_FreeSurface(src);
    return temp;
}
