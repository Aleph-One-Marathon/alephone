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
	See D3D11_Context.h.
*/

#include "D3D11_Context.h"

#ifdef HAVE_D3D11

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "cseries.h"
#include "Logging.h"

#ifdef _MSC_VER
// The HLSL compiler is loaded at run time, so only these need linking.
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#endif

namespace {

// Minimal owning pointer for COM interfaces, so every early return
// releases what it created. (Avoids a dependency on WRL.)
template <class T>
class ComRef {
public:
	ComRef() : p(nullptr) {}
	~ComRef() { reset(); }
	ComRef(const ComRef&) = delete;
	ComRef& operator=(const ComRef&) = delete;

	void reset() { if (p) { p->Release(); p = nullptr; } }
	void attach(T *raw) { reset(); p = raw; }	// takes ownership of one reference
	T *get() const { return p; }
	T **put() { reset(); return &p; }
	T *operator->() const { return p; }
	explicit operator bool() const { return p != nullptr; }

	template <class U>
	HRESULT as(ComRef<U> &other) const {
		return p ? p->QueryInterface(__uuidof(U), reinterpret_cast<void **>(other.put())) : E_POINTER;
	}

private:
	T *p;
};

// Fullscreen triangle with no vertex buffer; the pixel shader samples
// the uploaded frame. Shader model 4.0 so it runs at feature level 10_0.
const char kPresentShaderSource[] = R"(
Texture2D frameTexture : register(t0);
SamplerState frameSampler : register(s0);

struct VSOut {
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
};

VSOut vs_main(uint id : SV_VertexID)
{
	VSOut o;
	float2 uv = float2((id << 1) & 2, id & 2);
	o.position = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
	o.uv = uv;
	return o;
}

float4 ps_main(VSOut i) : SV_Target
{
	// The game's 32-bit surfaces have no alpha channel.
	return float4(frameTexture.Sample(frameSampler, i.uv).rgb, 1.0);
}
)";

struct State {
	HWND hwnd = nullptr;
	bool active = false;
	bool vsync = false;
	bool device_lost = false;

	ComRef<ID3D11Device> device;
	ComRef<ID3D11DeviceContext> context;
	ComRef<IDXGIFactory1> factory;
	ComRef<IDXGISwapChain> swap_chain;
	ComRef<ID3D11RenderTargetView> back_buffer_rtv;
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_10_0;
	UINT swap_chain_flags = 0;
	bool flip_model = false;
	bool allow_tearing = false;
	int buffer_width = 0;
	int buffer_height = 0;

	ComRef<ID3D11VertexShader> present_vs;
	ComRef<ID3D11PixelShader> present_ps;
	ComRef<ID3D11SamplerState> linear_sampler;
	ComRef<ID3D11RasterizerState> rasterizer;
	ComRef<ID3D11BlendState> opaque_blend;
	ComRef<ID3D11DepthStencilState> no_depth;

	ComRef<ID3D11Texture2D> frame_texture;
	ComRef<ID3D11ShaderResourceView> frame_srv;
	int frame_width = 0;
	int frame_height = 0;
	SDL_Surface *converted = nullptr;	// scratch for non-ARGB8888 surfaces

	std::string description;
	std::string adapter_choice;	// as requested in Initialize
	std::string adapter_name;	// the card actually in use
};

State g;

// d3dcompiler_47.dll ships with Windows 8.1 and later and is
// redistributable for Windows 7. Loading it at run time means a missing
// DLL makes the D3D11 renderer unavailable instead of stopping the game
// from starting.
pD3DCompile load_d3dcompile()
{
	static pD3DCompile fn = nullptr;
	static bool tried = false;
	if (tried)
		return fn;
	tried = true;

	const wchar_t *names[] = { L"d3dcompiler_47.dll", L"d3dcompiler_46.dll", L"d3dcompiler_43.dll" };
	for (const wchar_t *name : names) {
		HMODULE lib = LoadLibraryW(name);
		if (lib) {
			fn = reinterpret_cast<pD3DCompile>(reinterpret_cast<void *>(GetProcAddress(lib, "D3DCompile")));
			if (fn)
				break;
			FreeLibrary(lib);
		}
	}
	if (!fn)
		logError("Direct3D 11: no HLSL compiler (d3dcompiler_47.dll) found");
	return fn;
}

bool compile_shader(const char *source, size_t length, const char *name,
					const char *entry, const char *target, ComRef<ID3DBlob> &out)
{
	pD3DCompile compile = load_d3dcompile();
	if (!compile)
		return false;

	ComRef<ID3DBlob> errors;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
	HRESULT hr = compile(source, length, name, nullptr, nullptr, entry, target,
						 flags, 0, out.put(), errors.put());
	if (FAILED(hr)) {
		logError("Direct3D 11: shader %s:%s failed to compile (0x%08lx): %s", name, entry,
				 static_cast<unsigned long>(hr),
				 errors ? static_cast<const char *>(errors->GetBufferPointer()) : "no message");
		return false;
	}
	return true;
}

const char *feature_level_name(D3D_FEATURE_LEVEL level)
{
	switch (level) {
		case D3D_FEATURE_LEVEL_11_1: return "11_1";
		case D3D_FEATURE_LEVEL_11_0: return "11_0";
		case D3D_FEATURE_LEVEL_10_1: return "10_1";
		case D3D_FEATURE_LEVEL_10_0: return "10_0";
		default: return "unknown";
	}
}

void client_size(HWND hwnd, int &width, int &height)
{
	RECT rc = {};
	GetClientRect(hwnd, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
}

std::string narrow(const wchar_t *wide)
{
	char buffer[256] = {};
	WideCharToMultiByte(CP_UTF8, 0, wide, -1, buffer, sizeof(buffer) - 1, nullptr, nullptr);
	return buffer;
}

bool is_software_adapter(const DXGI_ADAPTER_DESC1 &desc)
{
	// Microsoft Basic Render Driver (WARP) reports the software flag; older
	// systems only identify it by its vendor/device IDs.
	return (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) ||
		(desc.VendorId == 0x1414 && desc.DeviceId == 0x8c);
}

// Lists hardware adapters in Windows' order. If `keep` is given, it
// receives one owned reference per listed adapter, parallel to `infos`.
void list_adapters(IDXGIFactory1 *factory, std::vector<D3D11::AdapterInfo> &infos,
				   std::vector<IDXGIAdapter1 *> *keep)
{
	for (UINT i = 0; ; ++i) {
		IDXGIAdapter1 *adapter = nullptr;
		HRESULT hr = factory->EnumAdapters1(i, &adapter);
		if (hr == DXGI_ERROR_NOT_FOUND)
			break;
		if (FAILED(hr) || !adapter)
			continue;

		DXGI_ADAPTER_DESC1 desc = {};
		adapter->GetDesc1(&desc);
		if (is_software_adapter(desc)) {
			adapter->Release();
			continue;
		}

		char base[48];
		std::snprintf(base, sizeof(base), "pci:%04x:%04x:%08x",
					  static_cast<unsigned>(desc.VendorId), static_cast<unsigned>(desc.DeviceId),
					  static_cast<unsigned>(desc.SubSysId));
		int ordinal = 0;
		for (const auto &other : infos)
			if (other.id.compare(0, std::strlen(base), base) == 0)
				++ordinal;

		D3D11::AdapterInfo info;
		info.id = std::string(base) + ":" + std::to_string(ordinal);
		info.name = narrow(desc.Description);
		info.dedicated_video_memory = desc.DedicatedVideoMemory;
		info.is_default = (i == 0);
		infos.push_back(info);

		if (keep)
			keep->push_back(adapter);
		else
			adapter->Release();
	}
}

// Resolves an adapter choice to an adapter. Leaves `out` empty for the
// Windows default (or when the choice can't be honoured, after logging why).
void choose_adapter(IDXGIFactory1 *factory, const std::string &choice, ComRef<IDXGIAdapter1> &out)
{
	out.reset();
	if (choice.empty())
		return;

	if (choice == D3D11::kAdapterHighPerformance || choice == D3D11::kAdapterMinimumPower) {
		ComRef<IDXGIFactory6> factory6;
		if (FAILED(factory->QueryInterface(__uuidof(IDXGIFactory6), reinterpret_cast<void **>(factory6.put())))) {
			logNote("Direct3D 11: GPU preference needs Windows 10 (1803) or later; using the default adapter");
			return;
		}
		DXGI_GPU_PREFERENCE preference = (choice == D3D11::kAdapterHighPerformance)
			? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_MINIMUM_POWER;
		if (SUCCEEDED(factory6->EnumAdapterByGpuPreference(0, preference, __uuidof(IDXGIAdapter1),
														   reinterpret_cast<void **>(out.put())))) {
			DXGI_ADAPTER_DESC1 desc = {};
			out->GetDesc1(&desc);
			if (is_software_adapter(desc))
				out.reset();
		}
		return;
	}

	std::vector<D3D11::AdapterInfo> infos;
	std::vector<IDXGIAdapter1 *> adapters;
	list_adapters(factory, infos, &adapters);
	for (size_t i = 0; i < infos.size(); ++i) {
		if (infos[i].id == choice) {
			out.attach(adapters[i]);
			adapters[i] = nullptr;
		}
	}
	for (IDXGIAdapter1 *adapter : adapters)
		if (adapter)
			adapter->Release();
	if (!out)
		logWarning("Direct3D 11: the selected graphics card (%s) was not found; using the default adapter",
				   choice.c_str());
}

HRESULT create_device_on(IDXGIAdapter *adapter, UINT flags)
{
	const D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	const UINT count = sizeof(levels) / sizeof(levels[0]);
	// An explicit adapter requires D3D_DRIVER_TYPE_UNKNOWN.
	D3D_DRIVER_TYPE type = adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;

	HRESULT hr = D3D11CreateDevice(adapter, type, nullptr, flags, levels, count, D3D11_SDK_VERSION,
								   g.device.put(), &g.feature_level, g.context.put());
	// Windows 7 without the platform update rejects 11_1 outright.
	if (hr == E_INVALIDARG)
		hr = D3D11CreateDevice(adapter, type, nullptr, flags, levels + 1, count - 1, D3D11_SDK_VERSION,
							   g.device.put(), &g.feature_level, g.context.put());
	return hr;
}

bool create_device()
{
	UINT flags = 0;
	const char *debug = std::getenv("ALEPHONE_D3D11_DEBUG");
	if (debug && *debug && *debug != '0')
		flags |= D3D11_CREATE_DEVICE_DEBUG;

	ComRef<IDXGIFactory1> enum_factory;
	ComRef<IDXGIAdapter1> adapter;
	if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(enum_factory.put()))))
		choose_adapter(enum_factory.get(), g.adapter_choice, adapter);

	HRESULT hr = create_device_on(adapter.get(), flags);
	if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG)) {
		// Debug layer requested but the SDK layers aren't installed.
		logWarning("Direct3D 11: debug layer unavailable, continuing without it");
		flags &= ~D3D11_CREATE_DEVICE_DEBUG;
		hr = create_device_on(adapter.get(), flags);
	}
	if (FAILED(hr) && adapter) {
		logWarning("Direct3D 11: the selected graphics card can't run Direct3D 11 (0x%08lx); using the default adapter",
				   static_cast<unsigned long>(hr));
		hr = create_device_on(nullptr, flags);
	}
	if (FAILED(hr)) {
		logWarning("Direct3D 11: no hardware device at feature level 10_0 or better (0x%08lx)",
				   static_cast<unsigned long>(hr));
		return false;
	}

	// The swap chain must come from the factory that owns the device's
	// adapter, which may not be the default one.
	ComRef<IDXGIDevice> dxgi_device;
	ComRef<IDXGIAdapter> device_adapter;
	if (FAILED(g.device.as(dxgi_device)) ||
		FAILED(dxgi_device->GetAdapter(device_adapter.put())) ||
		FAILED(device_adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(g.factory.put())))) {
		logWarning("Direct3D 11: could not reach the DXGI factory");
		return false;
	}

	DXGI_ADAPTER_DESC desc = {};
	device_adapter->GetDesc(&desc);
	g.adapter_name = narrow(desc.Description);
	g.description = g.adapter_name + " (feature level " + feature_level_name(g.feature_level) + ")";
	return true;
}

bool create_back_buffer_view()
{
	ComRef<ID3D11Texture2D> back_buffer;
	HRESULT hr = g.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(back_buffer.put()));
	if (FAILED(hr))
		return false;
	hr = g.device->CreateRenderTargetView(back_buffer.get(), nullptr, g.back_buffer_rtv.put());
	if (FAILED(hr))
		return false;

	D3D11_TEXTURE2D_DESC desc;
	back_buffer->GetDesc(&desc);
	g.buffer_width = static_cast<int>(desc.Width);
	g.buffer_height = static_cast<int>(desc.Height);
	return true;
}

bool create_swap_chain()
{
	int width, height;
	client_size(g.hwnd, width, height);
	width = std::max(width, 1);
	height = std::max(height, 1);

	g.flip_model = false;
	g.allow_tearing = false;
	g.swap_chain_flags = 0;

	// Flip model (Windows 8+; FLIP_DISCARD needs Windows 10) gives lower
	// latency and lets the compositor skip a copy. Fall back to the
	// classic blt model on older systems.
	ComRef<IDXGIFactory2> factory2;
	if (SUCCEEDED(g.factory.as(factory2))) {
		ComRef<IDXGIFactory5> factory5;
		if (SUCCEEDED(g.factory.as(factory5))) {
			BOOL tearing = FALSE;
			if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing))))
				g.allow_tearing = tearing != FALSE;
		}

		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		const DXGI_SWAP_EFFECT effects[] = { DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL };
		for (DXGI_SWAP_EFFECT effect : effects) {
			desc.SwapEffect = effect;
			desc.Flags = g.allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
			ComRef<IDXGISwapChain1> chain1;
			if (SUCCEEDED(factory2->CreateSwapChainForHwnd(g.device.get(), g.hwnd, &desc, nullptr, nullptr, chain1.put())) &&
				SUCCEEDED(chain1.as(g.swap_chain))) {
				g.flip_model = true;
				g.swap_chain_flags = desc.Flags;
				break;
			}
			g.allow_tearing = false;	// only meaningful with flip-discard
		}
	}

	if (!g.swap_chain) {
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = width;
		desc.BufferDesc.Height = height;
		desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 1;
		desc.OutputWindow = g.hwnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		HRESULT hr = g.factory->CreateSwapChain(g.device.get(), &desc, g.swap_chain.put());
		if (FAILED(hr)) {
			logWarning("Direct3D 11: could not create a swap chain (0x%08lx)", static_cast<unsigned long>(hr));
			return false;
		}
	}

	// SDL owns fullscreen (borderless desktop windows); keep DXGI from
	// switching modes behind its back on Alt+Enter.
	g.factory->MakeWindowAssociation(g.hwnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

	return create_back_buffer_view();
}

bool create_present_pipeline()
{
	ComRef<ID3DBlob> vs_code, ps_code;
	if (!compile_shader(kPresentShaderSource, sizeof(kPresentShaderSource) - 1, "present",
						"vs_main", "vs_4_0", vs_code) ||
		!compile_shader(kPresentShaderSource, sizeof(kPresentShaderSource) - 1, "present",
						"ps_main", "ps_4_0", ps_code))
		return false;

	if (FAILED(g.device->CreateVertexShader(vs_code->GetBufferPointer(), vs_code->GetBufferSize(), nullptr, g.present_vs.put())) ||
		FAILED(g.device->CreatePixelShader(ps_code->GetBufferPointer(), ps_code->GetBufferSize(), nullptr, g.present_ps.put())))
		return false;

	D3D11_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(g.device->CreateSamplerState(&sampler, g.linear_sampler.put())))
		return false;

	D3D11_RASTERIZER_DESC raster = {};
	raster.FillMode = D3D11_FILL_SOLID;
	raster.CullMode = D3D11_CULL_NONE;
	raster.DepthClipEnable = TRUE;
	if (FAILED(g.device->CreateRasterizerState(&raster, g.rasterizer.put())))
		return false;

	D3D11_BLEND_DESC blend = {};
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(g.device->CreateBlendState(&blend, g.opaque_blend.put())))
		return false;

	D3D11_DEPTH_STENCIL_DESC depth = {};
	depth.DepthEnable = FALSE;
	depth.StencilEnable = FALSE;
	if (FAILED(g.device->CreateDepthStencilState(&depth, g.no_depth.put())))
		return false;

	return true;
}

void release_frame_texture()
{
	g.frame_srv.reset();
	g.frame_texture.reset();
	g.frame_width = g.frame_height = 0;
}

void release_all()
{
	if (g.context)
		g.context->ClearState();
	release_frame_texture();
	g.no_depth.reset();
	g.opaque_blend.reset();
	g.rasterizer.reset();
	g.linear_sampler.reset();
	g.present_ps.reset();
	g.present_vs.reset();
	g.back_buffer_rtv.reset();
	g.swap_chain.reset();
	g.factory.reset();
	if (g.context)
		g.context->Flush();
	g.context.reset();
	g.device.reset();
	g.buffer_width = g.buffer_height = 0;
}

bool create_all()
{
	if (create_device() && create_swap_chain() && create_present_pipeline())
		return true;
	release_all();
	return false;
}

bool ensure_frame_texture(int width, int height)
{
	if (g.frame_texture && g.frame_width == width && g.frame_height == height)
		return true;
	release_frame_texture();

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	// SDL ARGB8888 in memory on little-endian
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(g.device->CreateTexture2D(&desc, nullptr, g.frame_texture.put())) ||
		FAILED(g.device->CreateShaderResourceView(g.frame_texture.get(), nullptr, g.frame_srv.put()))) {
		release_frame_texture();
		return false;
	}
	g.frame_width = width;
	g.frame_height = height;
	return true;
}

// Matches the swap chain to the window's client area. Returns false if
// the window has no area (minimized) and nothing should be drawn.
bool sync_back_buffer_size()
{
	int width, height;
	client_size(g.hwnd, width, height);
	if (width <= 0 || height <= 0 || IsIconic(g.hwnd))
		return false;
	if (width == g.buffer_width && height == g.buffer_height)
		return true;

	g.context->OMSetRenderTargets(0, nullptr, nullptr);
	g.back_buffer_rtv.reset();
	g.context->Flush();
	HRESULT hr = g.swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, g.swap_chain_flags);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		g.device_lost = true;
		return false;
	}
	if (FAILED(hr)) {
		logWarning("Direct3D 11: ResizeBuffers failed (0x%08lx)", static_cast<unsigned long>(hr));
		return false;
	}
	return create_back_buffer_view();
}

// Recreates everything after the GPU was removed or reset (driver update,
// TDR, adapter change). Returns false if we still can't draw this frame.
bool recover_if_lost()
{
	if (!g.device_lost)
		return true;
	logWarning("Direct3D 11: device lost, recreating");
	release_all();
	if (!create_all())
		return false;
	g.device_lost = false;
	logNote("Direct3D 11: recovered on %s", g.description.c_str());
	return true;
}

void present()
{
	UINT sync = g.vsync ? 1 : 0;
	UINT flags = (!g.vsync && g.allow_tearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	HRESULT hr = g.swap_chain->Present(sync, flags);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		HRESULT reason = g.device->GetDeviceRemovedReason();
		logWarning("Direct3D 11: device removed (0x%08lx)", static_cast<unsigned long>(reason));
		g.device_lost = true;
	}
}

bool begin_frame()
{
	if (!g.active || !recover_if_lost() || !sync_back_buffer_size())
		return false;

	ID3D11RenderTargetView *rtv = g.back_buffer_rtv.get();
	g.context->OMSetRenderTargets(1, &rtv, nullptr);
	const float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	g.context->ClearRenderTargetView(rtv, black);
	return true;
}

// Copies the surface into the dynamic frame texture, converting first if
// it isn't 32-bit ARGB.
bool upload_surface(SDL_Surface *surface)
{
	SDL_Surface *src = surface;
	if (surface->format->format != SDL_PIXELFORMAT_ARGB8888 &&
		surface->format->format != SDL_PIXELFORMAT_RGB888) {
		if (g.converted && (g.converted->w != surface->w || g.converted->h != surface->h)) {
			SDL_FreeSurface(g.converted);
			g.converted = nullptr;
		}
		if (!g.converted)
			g.converted = SDL_CreateRGBSurfaceWithFormat(0, surface->w, surface->h, 32, SDL_PIXELFORMAT_ARGB8888);
		if (!g.converted || SDL_BlitSurface(surface, nullptr, g.converted, nullptr) != 0)
			return false;
		src = g.converted;
	}

	if (!ensure_frame_texture(src->w, src->h))
		return false;

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (FAILED(g.context->Map(g.frame_texture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		return false;

	bool must_lock = SDL_MUSTLOCK(src);
	if (must_lock)
		SDL_LockSurface(src);
	const uint8_t *in = static_cast<const uint8_t *>(src->pixels);
	uint8_t *out = static_cast<uint8_t *>(mapped.pData);
	const size_t row_bytes = static_cast<size_t>(src->w) * 4;
	if (mapped.RowPitch == static_cast<UINT>(src->pitch) && row_bytes == static_cast<size_t>(src->pitch)) {
		std::memcpy(out, in, row_bytes * src->h);
	} else {
		for (int y = 0; y < src->h; ++y)
			std::memcpy(out + y * mapped.RowPitch, in + y * src->pitch, row_bytes);
	}
	if (must_lock)
		SDL_UnlockSurface(src);

	g.context->Unmap(g.frame_texture.get(), 0);
	return true;
}

// Fits the frame inside the back buffer, keeping its aspect ratio
// (same letterboxing SDL_RenderSetLogicalSize does for software mode).
D3D11_VIEWPORT letterbox_viewport(int frame_width, int frame_height)
{
	float scale = std::min(g.buffer_width / static_cast<float>(frame_width),
						   g.buffer_height / static_cast<float>(frame_height));
	float width = frame_width * scale;
	float height = frame_height * scale;

	D3D11_VIEWPORT vp;
	vp.TopLeftX = std::floor((g.buffer_width - width) * 0.5f);
	vp.TopLeftY = std::floor((g.buffer_height - height) * 0.5f);
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	return vp;
}

} // namespace

namespace D3D11 {

const char *const kAdapterAutomatic = "";
const char *const kAdapterHighPerformance = "high_performance";
const char *const kAdapterMinimumPower = "minimum_power";

std::vector<AdapterInfo> EnumerateAdapters()
{
	std::vector<AdapterInfo> infos;
	ComRef<IDXGIFactory1> factory;
	if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(factory.put()))))
		list_adapters(factory.get(), infos, nullptr);
	return infos;
}

bool Initialize(SDL_Window *window, bool vsync, const std::string &adapter_choice)
{
	Shutdown();

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!window || !SDL_GetWindowWMInfo(window, &info) || info.subsystem != SDL_SYSWM_WINDOWS) {
		logWarning("Direct3D 11: could not get the native window handle (%s)", SDL_GetError());
		return false;
	}

	g.hwnd = info.info.win.window;
	g.vsync = vsync;
	g.device_lost = false;
	g.adapter_choice = adapter_choice;
	if (!create_all()) {
		g.hwnd = nullptr;
		g.adapter_choice.clear();
		return false;
	}

	g.active = true;
	// Print once per device, not on every window recreation.
	static std::string last_printed;
	if (last_printed != g.description) {
		printf("Direct3D 11: %s, %s swap chain\n", g.description.c_str(),
			   g.flip_model ? "flip-model" : "blt-model");
		last_printed = g.description;
	}
	logNote("Direct3D 11: %s", g.description.c_str());
	return true;
}

void Shutdown()
{
	release_all();
	if (g.converted) {
		SDL_FreeSurface(g.converted);
		g.converted = nullptr;
	}
	g.active = false;
	g.device_lost = false;
	g.hwnd = nullptr;
	g.adapter_choice.clear();
	g.adapter_name.clear();
}

const std::string &ActiveAdapterChoice()
{
	return g.adapter_choice;
}

const std::string &ActiveAdapterName()
{
	return g.adapter_name;
}

bool IsActive()
{
	return g.active;
}

void SetVSync(bool vsync)
{
	g.vsync = vsync;
}

void GetDrawableSize(int &width, int &height)
{
	width = height = 0;
	if (!g.hwnd)
		return;
	client_size(g.hwnd, width, height);
}

void PresentSurface(SDL_Surface *surface)
{
	if (!surface || surface->w <= 0 || surface->h <= 0)
		return;
	if (!begin_frame())
		return;
	if (!upload_surface(surface)) {
		present();
		return;
	}

	D3D11_VIEWPORT vp = letterbox_viewport(surface->w, surface->h);
	ID3D11DeviceContext *ctx = g.context.get();
	ctx->RSSetViewports(1, &vp);
	ctx->RSSetState(g.rasterizer.get());
	ctx->OMSetBlendState(g.opaque_blend.get(), nullptr, 0xffffffff);
	ctx->OMSetDepthStencilState(g.no_depth.get(), 0);
	ctx->IASetInputLayout(nullptr);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->VSSetShader(g.present_vs.get(), nullptr, 0);
	ctx->PSSetShader(g.present_ps.get(), nullptr, 0);
	ID3D11ShaderResourceView *srv = g.frame_srv.get();
	ctx->PSSetShaderResources(0, 1, &srv);
	ID3D11SamplerState *sampler = g.linear_sampler.get();
	ctx->PSSetSamplers(0, 1, &sampler);
	ctx->Draw(3, 0);

	// Unbind so the texture can be mapped again next frame without a hazard.
	ID3D11ShaderResourceView *null_srv = nullptr;
	ctx->PSSetShaderResources(0, 1, &null_srv);

	present();
}

void PresentBlack()
{
	if (!begin_frame())
		return;
	present();
}

const char *DeviceDescription()
{
	return g.description.c_str();
}

ID3D11Device *Device()
{
	return g.device.get();
}

ID3D11DeviceContext *Context()
{
	return g.context.get();
}

} // namespace D3D11

#endif // HAVE_D3D11
