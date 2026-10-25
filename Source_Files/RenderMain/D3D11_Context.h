#ifndef _D3D11_CONTEXT_H_
#define _D3D11_CONTEXT_H_
/*
	Copyright (C) 2026 and beyond by the "Aleph One" developers.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Direct3D 11 device, swap chain and frame presentation.

	This is the foundation of the Direct3D 11 renderer. It owns the
	device (feature level 10_0 or better, so it runs on any DirectX 10
	class GPU), the swap chain for the game window, and a presenter that
	puts a CPU-drawn frame on screen with GPU scaling.

	Windows only. On other platforms HAVE_D3D11 is not defined and none
	of this is compiled.
*/

#if defined(_WIN32) && !defined(DISABLE_D3D11)
#define HAVE_D3D11 1
#endif

#ifdef HAVE_D3D11

struct SDL_Window;
struct SDL_Surface;
struct ID3D11Device;
struct ID3D11DeviceContext;

namespace D3D11 {

// Creates the device and a swap chain for the window. Returns false
// (and leaves nothing behind) if Direct3D 11 at feature level 10_0 or
// better is unavailable; the caller should fall back to software.
bool Initialize(SDL_Window *window, bool vsync);

// Releases everything. Must be called before the window is destroyed.
void Shutdown();

bool IsActive();

void SetVSync(bool vsync);

// Size of the window's drawable area in pixels (after high-DPI scaling).
void GetDrawableSize(int &width, int &height);

// Uploads the surface (any SDL format; ARGB8888 is the fast path) and
// draws it scaled to fit the window, letterboxed to keep its aspect
// ratio, then presents. Handles window resizes and device loss.
void PresentSurface(SDL_Surface *surface);

// Clears the window to black and presents.
void PresentBlack();

// Human-readable adapter name and feature level, for logs.
const char *DeviceDescription();

// For the renderer code built on top of this context.
ID3D11Device *Device();
ID3D11DeviceContext *Context();

} // namespace D3D11

#endif // HAVE_D3D11

#endif
