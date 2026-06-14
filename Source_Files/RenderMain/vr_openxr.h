/*
 *  vr_openxr.h  --  OpenXR (Meta Quest) VR backend for Aleph One. See vr_openxr.cpp.
 *
 *  Phase 2 of the Quest port. SDL-hosted: OpenXR binds to the EGL context SDL already created, and
 *  the engine keeps SDL for the main loop, input, audio and timing. The stereo frame loop replaces
 *  SDL_GL_SwapWindow. Public API is plain C; everything is a no-op off Android.
 */
#ifndef VR_OPENXR_H
#define VR_OPENXR_H

#ifdef __cplusplus
extern "C" {
#endif

// Create the OpenXR instance + system. Call once SDL video is initialised (the GL context need not
// exist yet -- the session is created lazily on the first frame). Returns true if VR is available.
bool VR_InitOpenXR(void);

// True once VR_InitOpenXR has succeeded (i.e. we are running as a VR app).
bool VR_IsActive(void);

// Create and make-current a headless (pbuffer) GLES3 EGL context that the engine renders with.
// Immersive VR apps get no SurfaceView Surface, so SDL can't make a window GL context; we own one
// instead (the QuestZDoom/TBXR approach). Call on the engine/render thread before any GL use.
bool VR_InitEGL(void);

// Eye (per-view) framebuffer resolution chosen by the runtime; valid after the first frame. Used as
// the engine's logical screen size in VR. Returns false until known.
bool VR_GetEyeResolution(int* w, int* h);

// Increment 1: render one head-tracked stereo test frame (a colored room) to the headset and
// submit it. Drives the OpenXR session lifecycle internally. Returns true if a VR frame was
// presented (the caller should then skip SDL_GL_SwapWindow). Returns false if VR isn't ready yet.
bool VR_RenderTestFrame(void);

#ifdef __cplusplus
}
#endif

#endif // VR_OPENXR_H
