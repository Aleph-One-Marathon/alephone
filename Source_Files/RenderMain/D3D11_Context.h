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

#include <cstdint>
#include <string>
#include <vector>

struct SDL_Window;
struct SDL_Surface;
struct ID3D11Device;
struct ID3D11DeviceContext;

namespace D3D11 {

// Which graphics card to render on. A choice is stored in preferences as
// one of these strings, or as a specific card's AdapterInfo::id.
extern const char *const kAdapterAutomatic;		// Windows' default adapter ("")
extern const char *const kAdapterHighPerformance;	// Windows' high-performance GPU
extern const char *const kAdapterMinimumPower;	// Windows' power-saving GPU

struct AdapterInfo {
	// Stable across reboots and driver updates: PCI vendor, device and
	// subsystem IDs, plus an ordinal to tell identical cards apart.
	// (Windows' own adapter LUIDs change on every boot.)
	std::string id;
	std::string name;				// e.g. "NVIDIA GeForce RTX 3060"
	uint64_t dedicated_video_memory;	// bytes; 0 for most integrated GPUs
	bool is_default;				// the adapter Windows uses by default
};

// Hardware graphics cards Windows reports, in Windows' order (the default
// adapter first). Microsoft's software renderer is left out. Safe to call
// whether or not Direct3D 11 is active.
std::vector<AdapterInfo> EnumerateAdapters();

// Creates the device on the chosen adapter and a swap chain for the
// window. If the chosen card isn't present any more, falls back to the
// Windows default. Returns false (and leaves nothing behind) if Direct3D
// 11 at feature level 10_0 or better is unavailable; the caller should
// fall back to software.
bool Initialize(SDL_Window *window, bool vsync, const std::string &adapter_choice = kAdapterAutomatic);

// The adapter choice passed to the last successful Initialize, so callers
// can tell whether the preference has changed since.
const std::string &ActiveAdapterChoice();

// Name of the card actually in use (empty when inactive).
const std::string &ActiveAdapterName();

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
