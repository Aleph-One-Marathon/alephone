# Aleph One → Meta Quest (2/3) VR Port — Architecture & Plan

**Status:** Analysis pass 1 (feasibility + plan). No engine code changed yet.
**Reference port:** QuestZDoom (`C:\Projects\QuestZDoom`), DrBeef's GZDoom VR fork.
**Date:** 2026-06-13

---

## 1. Verdict

**Viable, but it's a real project — estimate ~6–12 months of focused solo work.** Aleph One is far
more portable than most engines of its age (SDL2 throughout, an existing shader render path), but
the single dominant cost is that its "shader" renderer is not actually a modern programmable-pipeline
renderer — it is **GLSL shaders bolted onto the fixed-function pipeline**, which does not exist in
OpenGL ES. Modernizing that renderer to clean GL ES 3.x is the heart of the port; everything else is
comparatively well-trodden.

The good news that makes it tractable:

- **SDL2 end to end.** SDL2 has a first-class Android backend (JNI bridge, GL context, input, audio).
  This is the biggest single portability win.
- **An existing shader path** (`Rasterizer_Shader`, `RenderRasterize_Shader`, `OGL_Shader`) separate
  from the legacy fixed-function and software renderers — we have something to modernize rather than
  write from scratch.
- **Shaders are GLSL 1.10 dialect** (`attribute` / `varying` / `gl_FragColor`), which maps closely to
  GLSL ES 1.00 / 3.00 once the fixed-function dependencies are removed.
- **QuestZDoom's TBXR harness is engine-agnostic by design** and reusable, and its `SupportLibs` already
  ships Android builds of several libraries Aleph One also depends on (OpenAL, libsndfile, libvpx, etc.).

---

## 2. Architecture findings (from the current codebase)

### 2.1 Rendering — the critical path

Aleph One has **three** renderers:

| Renderer | Files | VR-relevant? |
|---|---|---|
| Software | `Rasterizer_SW.h`, `scottish_textures.cpp` | No (CPU; no stereo/3D) |
| Fixed-function GL | `OGL_Render.cpp` (59 FF calls) | No — dead end for ES |
| **Shader GL** | `RenderRasterize_Shader.cpp` (50 FF calls), `Rasterizer_Shader.cpp`, `OGL_Shader.cpp` | **Yes — this is the one we port** |

The catch, confirmed by reading `RenderRasterize_Shader.cpp` in full: the shader renderer still relies
on the fixed-function pipeline for nearly everything structural. Concretely, these are used and **none
of them exist in OpenGL ES 2/3**:

- **Matrix stack:** `glPushMatrix`, `glPopMatrix`, `glTranslatef`, `glRotatef`, `glScalef`,
  `glMatrixMode`, `glLoadIdentity`, `glLoadMatrixd` — used for object/sprite placement *and* projection
  (`Screen_2_Clip`). The shaders read `gl_ModelViewMatrix` / `gl_ProjectionMatrix` / `gl_TextureMatrix`.
- **Immediate per-vertex/per-draw state:** `glColor4f` (shaders read `gl_Color`), `glNormal3f`,
  `glMultiTexCoord4fARB` (tangent).
- **Fixed-function client arrays:** `glVertexPointer`, `glTexCoordPointer`, `glNormalPointer`,
  `glClientActiveTextureARB`, `glEnableClientState` — instead of generic vertex attributes + VBOs.
- **Primitive types that don't exist in ES:** `glDrawArrays(GL_POLYGON, …)` and `GL_QUADS`. Must become
  triangle fans/strips or indexed triangles.
- **User clip planes:** `glClipPlane` / `GL_CLIP_PLANE0..5` (used for window clipping and media
  boundaries). ES has no fixed clip planes — must move to `gl_ClipDistance` (ES 3.2 / `EXT_clip_cull_distance`)
  or fragment-shader `discard`.
- **Alpha test:** `glAlphaFunc` / `GL_ALPHA_TEST` — gone in ES; must become shader `discard`.
- **sRGB framebuffer:** `GL_FRAMEBUFFER_SRGB_EXT` — handled differently on ES.

**Implication:** the GLSL shaders are compatibility-profile shaders depending on FF built-in attributes
and uniforms. On GL ES (and core profile) *all* of those built-ins are gone. So the work is, for every
shader and every draw site:
1. Replace built-in attributes with explicit ones (`aPosition`, `aTexCoord0/1`, `aNormal`, `aColor`).
2. Replace built-in matrix uniforms with explicit ones (`uMVP`, `uTextureMatrix`, `uNormalMatrix`) the
   C++ computes and uploads.
3. Replace immediate-mode state (`glColor`, `glNormal`) with per-vertex attribute data in VBOs.
4. Convert `GL_QUADS`/`GL_POLYGON` to ES-legal primitives.
5. Replace clip planes and alpha test with shader logic.

This is mechanical but pervasive, and it spans not just `RenderRasterize_Shader.cpp` but the 2D/overlay
GL code too (see 2.2).

### 2.2 Secondary GL surfaces (HUD, menus, overlays)

Roughly ~100 more FF calls live in 2D/overlay code that also needs an ES rewrite, but it's simpler
(orthographic, textured quads):

- `HUDRenderer_OGL.cpp` (11), `HUDRenderer_Lua.cpp` (11) — in-game HUD.
- `OverheadMap_OGL.cpp` (13) — automap.
- `OGL_Faders.cpp` (9) — full-screen color fades.
- `OGL_FBO.cpp` (11) — framebuffer object wrapper (mostly already FBO-based; light touch).
- `screen.cpp` (12), `OGL_Blitter.cpp`, `Shape_Blitter.cpp`, `FontHandler.cpp`, `OGL_LoadScreen.cpp` —
  blitting, fonts, load screens.

For VR these overlays get reprojected onto a world-space panel rather than the physical screen (Phase 3).

### 2.3 GL header / context selection

`OGL_Headers.h` selects desktop GL only: GLEW on Windows, `SDL2/SDL_opengl.h` + GLU elsewhere. There is
**no ES path and no EGL path**. Needs an `__ANDROID__` branch pulling `<GLES3/gl3.h>` / `<GLES3/gl31.h>`
and dropping GLU (GLU does not exist on ES — check for `glu*` usage during the rewrite). SDL2 already
creates GLES contexts, and vcpkg pulled the GLES2/3 + EGL headers, so the plumbing exists.

### 2.4 Platform / main loop

- Entry: `main.cpp:8` `int main()` → `main_event_loop()` (`shell.cpp:712`), with `global_idle_proc()`
  pumped from several places (`images.cpp`, `sdl_dialogs.cpp`).
- This is a **blocking desktop loop**. VR inverts control: the OpenXR runtime drives the frame and the
  host calls into the engine per-frame/per-eye. So `main_event_loop` must be refactored into a
  re-entrant "advance one frame / render one eye" entry point the VR host can call. SDL2-Android's
  `SDL_main` + activity model is compatible with this, mirroring how TBXR drives GZDoom.

### 2.5 Dependencies (current `vcpkg.json`)

boost (algorithm/uuid/property-tree/iostreams/filesystem/lockfree/dll), SDL2 + SDL2_image + SDL2_ttf,
asio, curl, glew (Windows only — dropped on Android), zziplib, miniupnpc, nativefiledialog-extended
(desktop only — replace with native/no-op on Android), libsndfile, openal-soft, catch2, libyuv,
steamworks-sdk (drop on Android), matroska, libvpx.

Android availability: SDL2 family, boost, openal-soft, libsndfile, libvpx, curl, zziplib all build for
Android (and QuestZDoom's `SupportLibs` already has Android builds of OpenAL/libsndfile/libvpx/libpng/
libzip we can crib from). `nativefiledialog-extended`, `glew`, and `steamworks-sdk` get stubbed/dropped.

---

## 3. The QuestZDoom / TBXR pattern (what we reuse)

QuestZDoom's VR layer is **TBXR ("The Big XR")**, DrBeef's deliberately engine-agnostic framework reused
across Doom3Quest / QuestZDoom / JKQuest / etc. Layout (`Projects/Android/jni/`):

```
QzDoom/
  TBXR_Common.cpp / .h   ← engine-agnostic VR harness: OpenXR session, swapchains,
                            per-eye framebuffers, frame loop, head-pose → matrices (70KB)
  QzDoom_OpenXR.cpp      ← per-game glue: connects TBXR to the GZDoom engine
  OpenXrInput.cpp        ← OpenXR action sets / controller bindings
  VrInputDefault.cpp     ← maps controller state → game input/locomotion
  matrixlib.c / mathlib.c← matrix/vector math
OpenXR-SDK/              ← OpenXR loader + headers
SupportLibs/             ← Android builds of openal, libsndfile, libvpx, libpng, libzip, …
Android.mk / Application.mk ← ndk-build (NOT gradle-native CMake)
```

`AndroidPrebuilt/jni/libopenxr_loader.so` is the prebuilt OpenXR loader.

**Reuse plan:** lift `TBXR_Common.*`, `OpenXrInput.cpp`, the math libs, and the OpenXR-SDK wiring largely
intact. Write a new **`AlephOne_OpenXR.cpp`** analogous to `QzDoom_OpenXR.cpp` that:
- owns the inverted frame loop and calls Aleph One's per-frame/per-eye render entry,
- injects the per-eye view + projection matrices into the engine's camera (`view_data`),
- feeds controller input into Aleph One's input system.

### 3.1 The exact engine ↔ VR contract (extracted from TBXR_Common)

The boundary is two sets of plain C functions (no C++ coupling), declared in `TBXR_Common.h`:

**(a) Functions the game must implement** (TBXR calls these — these are our `AlephOne_OpenXR.cpp`):
```c
void  VR_FrameSetup();                                          // per-frame game hook (often empty)
bool  VR_UseScreenLayer();                                      // true → render 2D menu/HUD as a flat quad layer
float VR_GetScreenLayerDistance();                              // metres to that quad
bool  VR_GetVRProjection(int eye, float zNear, float zFar, float* projection);  // fill 4x4 per-eye proj
void  VR_HandleControllerInput();                               // poll + map controllers to game input
void  VR_SetHMDOrientation(float pitch, float yaw, float roll); // TBXR pushes head orientation in
void  VR_SetHMDPosition(float x, float y, float z);             // TBXR pushes head position in
void  VR_Shutdown();  /* + VR_Haptic* family */
```

**(b) Functions TBXR provides** (the engine's loop calls these):
```c
TBXR_InitialiseOpenXR(); TBXR_EnterVR(); TBXR_InitRenderer(); TBXR_InitActions();  // startup
bool TBXR_FrameSetup();          // begin frame; INTERNALLY pushes head pose via VR_SetHMD*,
                                 //   then calls VR_FrameSetup() + VR_HandleControllerInput();
                                 //   returns FrameState.shouldRender
TBXR_updateProjections();        // xrLocateViews → per-eye pose + fov
TBXR_prepareEyeBuffer(int eye);  // acquire + bind + clear that eye's GLES FBO
TBXR_finishEyeBuffer(int eye);   // resolve + release that eye's FBO
TBXR_submitFrame();              // build projection layer + xrEndFrame
```

**The per-frame loop the engine must drive** (replaces Aleph One's desktop present in `main_event_loop`):
```c
if (TBXR_FrameSetup()) {              // also delivers head pose + input for this frame
    TBXR_updateProjections();
    for (int eye = 0; eye < 2; eye++) {
        TBXR_prepareEyeBuffer(eye);
        // ── ENGINE renders the world into the bound FBO, using: ──
        //   projection = VR_GetVRProjection(eye, zNear, zFar)
        //   modelview  = perEyeView(eye) × worldFromPlayer(hmdPosition, hmdorientation, player pos/yaw)
        TBXR_finishEyeBuffer(eye);
    }
}
TBXR_submitFrame();
```

**What this means for Aleph One specifically:**
- The eye loop and the scene draw are the **engine's** responsibility (TBXR only owns the FBOs and
  compositor). So we graft this loop into Aleph One's render path and, per eye, **override `view_data`'s
  projection and modelview** with the VR matrices instead of the engine's `Screen_2_Clip` / `glRotatef`
  camera. This is the same plumbing Phase 1 already makes explicit (uMVP uniforms) — so Phase 1 directly
  enables Phase 2; the VR matrices simply substitute for the computed ones.
- **Head pose arrives via callbacks**, not pull: `VR_SetHMDOrientation/Position` store globals
  (`hmdorientation`, `hmdPosition`) that the render + movement code read. Movement is fed back through a
  `VR_GetMove()`-style accessor into the player input.
- **`VR_UseScreenLayer()` is the menu/terminal/2D path.** When true, TBXR composites a single flat quad
  instead of stereo projection. Marathon is heavy on fullscreen 2D (terminals, automap, menus, overlay
  map) — these render through the screen-layer path using the Phase 1 overlay rewrite, not full stereo.
- Everything here binds **GLES3 FBOs** and expects the engine to draw with GLES — which is why Phase 1
  (renderer modernization) is a hard prerequisite for Phase 2.

> Note: QuestZDoom's `gzdoom-g3.3mgw_mobile` is an unpopulated submodule locally, so the engine *side*
> of this wiring (where GZDoom's stereo3d renderer calls the eye-buffer functions) isn't on disk — but
> the contract above is fully determined by `TBXR_Common.cpp` + `QzDoom_OpenXR.cpp`, which are present.

---

## 4. Phased plan

### Phase 0 — Flat Android build (no VR)
**Goal:** Aleph One compiles and runs as a plain `.apk`, rendering mono to the device screen.
- Stand up the NDK build (build-system decision in §6) with SDL2-Android `SDL_main` + activity.
- Port/stub dependencies; drop glew/steamworks/nativefiledialog; wire OpenAL + libsndfile + libvpx from
  TBXR `SupportLibs` or vcpkg-android.
- Add the `__ANDROID__` / GLES3 / EGL branch to `OGL_Headers.h`; get a context created.
- Initially can run the **software renderer** to prove input/audio/asset loading before tackling GL.
- **Deliverable:** title screen + a level rendering (even SW) on a Quest in flat mode.
- **Estimate:** 2–4 weeks (mostly dependency wrangling).

### Phase 1 — GL ES renderer modernization  ← the hard part
**Goal:** the shader render path runs on OpenGL ES 3.x.
- Rewrite all shaders in `OGL_Shader.cpp` to GLSL ES (explicit attributes/uniforms, `precision`, version
  pragma, `discard` for alpha test, optional `gl_ClipDistance`).
- Rewrite `RenderRasterize_Shader.cpp` draw sites: explicit MVP/texture/normal matrices computed in C++,
  VBOs + generic vertex attributes, per-vertex color/normal/tangent, `GL_QUADS`/`GL_POLYGON` → triangles.
- Replace `glClipPlane` window/media clipping with shader clip distances or fragment discard.
- Port the 2D/overlay GL (HUD, automap, faders, blitters, fonts) to the same ES path.
- Validate against the desktop GL output frame-by-frame to catch regressions.
- **Deliverable:** full-fidelity mono GL ES rendering on Quest in flat mode.
- **Estimate:** 2–4 months.

### Phase 2 — Stereo VR rendering via TBXR
**Goal:** the world renders in stereo with head tracking.
- Integrate `TBXR_Common` + OpenXR-SDK; create per-eye swapchain framebuffers.
- Invert the main loop into a TBXR-driven per-eye render entry (`AlephOne_OpenXR.cpp`).
- Drive Aleph One's camera from the head pose: per-eye projection (FOV from OpenXR), per-eye view matrix,
  IPD. Replace the engine's screen-space projection (`Screen_2_Clip`, `glFrustum`-equivalent) with the
  matrices TBXR provides.
- Decouple world scale → real-world units (define how 1 WU maps to metres for comfortable scale).
- **Deliverable:** stand inside a Marathon level in stereo, look around with the headset.
- **Estimate:** 1–2 months.

### Phase 3 — VR input, locomotion, and UX
**Goal:** actually playable in VR.
- OpenXR action sets → Aleph One input: thumbstick locomotion + snap/smooth turn, jump/run/action.
- 6DOF controller as weapon: aim from the controller pose rather than the head; map triggers to fire.
- Reproject the 2D HUD into a world-space panel (uses the Phase 1 overlay rewrite); same for menus,
  terminals, and the automap.
- Comfort options (vignette, snap turn, height calibration).
- **Deliverable:** end-to-end playable VR build; iterate on feel.
- **Estimate:** 1–2 months + ongoing polish.

---

## 5. Risks

1. **`RenderRasterize_Shader.cpp` modernization (Phase 1).** This is the make-or-break. It's not hard
   in any single spot, but it's pervasive and easy to introduce subtle rendering regressions. If this
   goes smoothly, the rest is assembly.
2. **Marathon's screen-space rendering assumptions.** The engine computes a screen projection and does
   weapon/HUD sprites directly in screen clip space (`Screen_2_Clip`, `render_viewer_sprite_layer`).
   VR needs true per-eye world projection; the weapon-in-hand model especially must move from a 2D
   screen sprite to a world-space object (or a 3D weapon model) to feel right.
3. **Clip-plane removal.** Window clipping is core to Marathon's portal/visibility rendering. Moving it
   off `glClipPlane` without breaking the portal renderer needs care.
4. **Performance on Quest.** Stereo doubles draw cost; the renderer was never tuned for tiled mobile
   GPUs. Multiview (`OVR_multiview`) is the standard mitigation but adds shader constraints.
5. **Dependency Android builds.** Manageable, but each non-trivial dep (libsndfile, libvpx, matroska,
   miniupnpc) is a potential time sink.

---

## 6. Open decision: build system (deferred, decide after analysis)

Aleph One's non-Windows build is **autotools** (`configure.ac` / `Makefile.am`). QuestZDoom uses
**ndk-build** (`Android.mk` / `Application.mk`) wrapped by gradle. Options:

- **(A) New ndk-build `Android.mk`** matching the TBXR harness's expectations — cleanest integration with
  the reused VR code, but a third build description to maintain.
- **(B) gradle + CMake (`externalNativeBuild`)** — modern NDK default; would mean authoring a CMakeLists
  for the Android target and adapting TBXR (which expects ndk-build) to it.
- **(C) Extend autotools under the NDK** — reuses upstream logic but fights the Android toolchain.

**DECIDED: (B) gradle + CMake** — flipped from the initial "leaning A" after the dependency analysis.
Aleph One's graph is dominated by Boost + the SDL2 family, which are designed for CMake/vcpkg and painful
to hand-author as ndk-build `Android.mk`; SDL2 has first-class Android CMake support; and TBXR's files are
plain C/C++ that compile fine as CMake sources (we are not locked out of reusing TBXR). Full rationale,
dependency triage, and the scaffold are in **[ANDROID_BUILD.md](ANDROID_BUILD.md)**.

---

## 7. Recommended next step

**Done:** the engine ↔ VR interface is now extracted (§3.1).

Next, kick off **Phase 0** — stand up the Android/NDK build with SDL2-Android and get *something* (title
screen / a level via the software renderer) running flat on a Quest. This retires the dependency-porting
risk early and gives a harness to iterate the Phase 1 renderer work against. Concretely:
1. Decide the build system (§6) — author the `Android.mk` (leaning A) or a CMake target.
2. Triage each `vcpkg.json` dependency to an Android source (reuse TBXR `SupportLibs` where possible;
   stub glew/steamworks/nativefiledialog).
3. Add the `__ANDROID__` / GLES3 / EGL branch to `OGL_Headers.h` and get an `SDL_main` activity launching.

In parallel, map the exact `view_data` injection points in Aleph One's render path (where projection and
modelview are set) so Phase 2's matrix substitution is a known, small surface when we reach it.
