/*
 * This code by David Olsen was released into the public domain at:
 *
 *    https://sourceforge.net/projects/sdlresize/
 *
 * Unless otherwise stated, modifications to this source file by
 * Jeremiah Morris or other Aleph One developers are also released
 * into the public domain.
 */

/*resize++.cpp : 
  High quality image scaling with Lanczos interpolation method adapted 
  from the Lanczos filtering code available on wikipedia.org.
  -by David Olsen.
  
  There are two methods to use the library, the only difference is in
  whether the image should be scaled to a specific size in pixels,
  or be scaled proportionally by a certain percentage(1.0 = 100%).

  If scale_factor > 1.0 image will be enlarged, < 1.0 will shrink the image

  If no rescaling is needed, the original surface is returned. Otherwise,
  a new surface of the same format is created and returned.

  The option to free or not free the source image is given, so that if you 
  desire to make multiple copies of a source image at various sizes, 
  you do not need to reload the image for each operation.

  The filter variable determines the radius of the Lanczos filter.
  The following settings can be used:
    1 = Quickest filtering mode, but when used for enlarging, produces
        blocky results, similar to the nearest neighbor approach. 
        When used for reductions, it outputs very nice results.
    2 = Medium filtering mode - slightly faster than 3, but slower than 1.
        When enlarging, this mode produces high quality results similar
        to (but a bit better than) bilinear interpolation.
    3 = The highest quality mode, also the slowest, but only about 30%
        slower than mode 1. This mode will produce the best enlargements.
        There is no need for a filter radius greater than 3.
    The default filtering value passed to the function is 4. Any number
    greater than 3 will tell the library to automatically choose the quickest
    filtering mode that still maintains highest quality output. Basically,
    if you are shrinking an image greatly, it will choose radius of 1.
    If you shrink an image a little bit, it will choose radius of 2.
    For all other operations, it will choose a radius of 3.
    You may therefore specify which filter mode to use manually, if speed
    is a particular issue, or let it be automatically handled, 
    for highest quality results. 

    The typical usage pattern can be demonstrated as follows:
        SDL_Surface *image = SDL_LoadBMP("myimage.bmp");
        image = SDL_Resize(image,320,240); //scale image to 320x240
        //the original image is freed, and image points to the new scaled surface.

    or

        SDL_Surface *image = SDL_LoadBMP("myimage.bmp");
        SDL_Surface *small_image = SDL_Resize(image,0.5f,false); //shrink to 50% size
        SDL_Surface *big_image = SDL_Resize(image,2.0f,false); //enlarge to 200% size
        //image still points to the original loaded surface.

    If you have any questions or comments, feel free to contact me(David Olsen) at:
    jolynsbass@gmail.com

*/

#ifndef __RESIZEpp__
#define __RESIZEpp__

#include <SDL2/SDL.h>

SDL_Surface * SDL_Resize(SDL_Surface *src, float scale_factor,   bool free_src = true, int filter = 4);
SDL_Surface * SDL_Resize(SDL_Surface *src, int new_w, int new_h, bool free_src = true, int filter = 4);

#endif
