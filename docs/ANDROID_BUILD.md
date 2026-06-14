# Aleph One — Android build (Phase 0 of the VR port)

**Goal of Phase 0:** Aleph One compiles and runs as a flat (non-VR) `.apk` on a Quest / any Android
device, rendering mono to the screen. This retires the dependency-porting risk and gives a harness to
iterate the Phase 1 GLES renderer work against. See [VR_PORT_PLAN.md](VR_PORT_PLAN.md) for the full plan.

> **Key insight:** Phase 0 needs **no TBXR and no OpenXR.** SDL2's Android backend (`SDLActivity`)
> already provides the window, GLES context, input, audio, and timers. TBXR/OpenXR only enters at
> Phase 2, where OpenXR must own the GL context instead of SDL. So Phase 0 is a *standard SDL2-Android
> app* — which is exactly why it's the right first milestone.

---

## 0. Toolchain setup (one-time)

Nothing Android is installed yet. You need an Android SDK + NDK + CMake. The fastest path:

1. **Install Android Studio** — https://developer.android.com/studio. It bundles a JDK and the SDK
   manager and gives you device deployment + logcat for the Quest. (You don't have to use the IDE to
   build, but it's the easiest way to get and manage the pieces below.)
2. In Android Studio → **SDK Manager**, install:
   - SDK Platform **android-34**
   - **NDK (Side by side)** r26+ (e.g. 26.3.x)
   - **CMake** 3.22+
   - **Android SDK Build-Tools** and **Platform-Tools** (gives you `adb`)
3. Set environment variables (PowerShell, persists across sessions):
   ```powershell
   setx ANDROID_HOME "$env:LOCALAPPDATA\Android\Sdk"
   setx ANDROID_NDK_HOME "$env:LOCALAPPDATA\Android\Sdk\ndk\26.3.11579264"
   ```
4. **Build the dependencies for Android** (long first run — boost + SDL2 from source):
   ```powershell
   & "C:\Projects\vcpkg\vcpkg.exe" install --triplet arm64-android
   ```
   (vcpkg's `arm64-android` triplet reads `ANDROID_NDK_HOME`.)
5. **Build the app:**
   ```powershell
   cd C:\Projects\alephone\android
   .\gradlew assembleDebug
   ```

**Running it — do you need an emulator?** For a quick flat smoke test you *can* use an Android emulator,
but the real target is the headset:
- On the Quest: enable **Developer Mode** (Meta Quest mobile app → Headset → Developer Mode; needs a
  free Meta developer account + a "verified organization"), plug in USB-C, then `adb install`:
  ```powershell
  adb devices              # confirm the Quest shows up (accept the USB-debug prompt in-headset)
  adb install -r app\build\outputs\apk\debug\app-debug.apk
  adb logcat -s AlephOne SDL DEBUG   # watch logs
  ```
- Phase 0 renders flat (mono) — it'll show up as a 2D window inside the Quest's panel view, which is
  fine for validating that the engine boots, loads data, takes input, and plays audio before we touch
  the renderer.

> **Command-line-only alternative (no IDE):** install Android *command-line tools*, then
> `sdkmanager "platforms;android-34" "ndk;26.3.11579264" "cmake;3.22.1" "platform-tools" "build-tools;34.0.0"`.
> Works, but you lose the IDE's device/logcat conveniences — not recommended for a first Quest setup.

---

## 1. Build system decision: gradle + CMake (not ndk-build)

§6 of the plan left this open, "leaning ndk-build (A)" to match QuestZDoom. **After analysis I'm
flipping to gradle + CMake (B).** Reasoning:

- Aleph One's dependency graph is dominated by **Boost** (filesystem/system compiled + many header libs)
  and the **SDL2 family** (SDL2, SDL2_ttf, SDL2_image). Hand-authoring ndk-build `Android.mk` for Boost
  is genuinely painful; these libraries are *designed* to be consumed via CMake / vcpkg.
- **SDL2 has first-class Android CMake support** and ships the `SDLActivity` Java glue we need.
- TBXR's files (`TBXR_Common.cpp`, `OpenXrInput.cpp`, math libs) are **plain C/C++** — they shipped as
  `Android.mk` but compile fine as CMake sources when we add them in Phase 2. We are not locked out of
  reusing TBXR by choosing CMake.
- vcpkg already builds every one of these deps for the `arm64-android` triplet, so we can reuse the same
  manifest workflow that's already set up for the desktop Windows build.

**Target ABI:** `arm64-v8a` only (Quest 2/3). **minSdk:** 29 (Quest runs Android 10/12L). **STL:**
`c++_shared`.

---

## 2. Dependency triage (`vcpkg.json` → Android)

Status legend: ✅ keep (build for arm64-android) · 🔻 defer (disable for first boot, add later) ·
❌ drop (desktop-only, stub out). "SupportLibs" = already an Android build in
`C:\Projects\QuestZDoom\Projects\Android\jni\SupportLibs` we can crib from if vcpkg-android is fussy.

| Dependency | Required? | Phase 0 | Android source | Notes |
|---|---|---|---|---|
| **boost** (filesystem, system, algorithm, uuid, property-tree, iostreams, lockfree, dll) | **required** | ✅ | vcpkg `arm64-android` | filesystem/system compiled; rest header-only. The big one. `boost-dll` needs `dlopen` (fine on Android). |
| **asio** | required | ✅ | vcpkg (header-only) | trivial |
| **sdl2** | required | ✅ | vcpkg / SDL source | provides `SDLActivity`, GLES context, input, audio, timers |
| **sdl2-ttf** | required | ✅ | vcpkg (pulls freetype/harfbuzz) | menu/terminal fonts |
| **zlib** | required | ✅ | NDK ships it | |
| **libsndfile** | required | ✅ | vcpkg / SupportLibs `libsndfile-android` | sound file decoding |
| **openal-soft** (pinned 1.23.1#2) | required | ✅ | vcpkg / SupportLibs `openal` | audio output; has an Android backend |
| **sdl2-image** (+libjpeg-turbo) | on by default | 🔻→✅ | vcpkg (pulls jpeg/png/webp) | textures/launcher art; can boot without, add early |
| **glew** | windows-only | ❌ | — | desktop GL loader; GLES needs no loader. Already `platform:windows` in manifest |
| **curl** | windows-only in manifest | 🔻 | vcpkg | online play/updates; defer for first boot |
| **zziplib** | optional | 🔻 | needs own build | reads zipped scenarios/plugins; game boots from plain dirs without it |
| **miniupnpc** | optional | 🔻 | vcpkg / source | NAT punch for netplay; defer |
| **nativefiledialog-extended** | optional | ❌ | — | desktop file dialogs; stub with no-op / SAF later |
| **libyuv** | optional (film) | 🔻 | vcpkg / source | film export + video; defer |
| **matroska** + **libvpx** (+ebml/vorbis) | optional (film) | 🔻 | SupportLibs `libvpx`,`liboggvorbis` | film export needs all six; defer the whole group |
| **steamworks-sdk** | optional | ❌ | — | no Steam on Quest; stub |
| **catch2** | tests only | ❌ | — | not in the app build |

**Phase-0 minimal link set:** boost, asio, sdl2, sdl2-ttf, zlib, libsndfile, openal-soft (+ sdl2-image
very soon). Everything else compiles behind its `HAVE_*` guard turned **off** initially.

### What must be stubbed in engine code for the disabled deps
The `HAVE_*` macros already gate most of this (see `configure.ac`). For the Android build we define only
the macros we support. Spots that may need a small `__ANDROID__` no-op:
- `nativefiledialog` (file pickers) — replace with a no-op or a fixed data path for now.
- `steamworks` — already behind `HAVE_STEAM`; leave undefined.
- `miniupnpc` / `curl` — behind `HAVE_MINIUPNPC` / `HAVE_CURL`; leave undefined → netplay/online off.

---

## 3. Scaffold layout

```
android/
  settings.gradle
  build.gradle                 # root
  gradle.properties
  app/
    build.gradle               # android app module; externalNativeBuild → CMake; ABI arm64-v8a
    src/main/
      AndroidManifest.xml
      java/org/libsdl/app/SDLActivity.java        # from SDL (or via aar)
      java/org/alephone/AlephOneActivity.java     # SDLActivity subclass; lib name + args
      assets/                  # bundled data files (shapes/maps/sounds) for first boot
  jni/
    CMakeLists.txt             # builds libalephone.so: globs Source_Files, links deps
```

The CMake target compiles `Source_Files/**` (same dir set as `configure.ac`'s `AC_CONFIG_FILES`:
CSeries, Files, GameWorld, Input, Lua, Misc, ModelView, Network[/Metaserver], RenderMain, RenderOther,
Sound, TCPMess, Videos, XML + the `Source_Files` root) plus `Lua` vendored sources, and links the
Phase-0 dep set. `main.cpp` already `#include`s `<SDL2/SDL_main.h>`, so on Android SDL renames `main`
to `SDL_main` and `SDLActivity` calls it — no entry-point change needed.

---

## 4. Engine code changes needed for Phase 0

1. **`OGL_Headers.h`** — add an `__ANDROID__` branch that pulls `<GLES3/gl3.h>` / `<GLES3/gl3ext.h>`
   and drops GLU/GLEW. *(Done — see commit; first concrete porting edit.)*
2. **Renderer** — the software renderer is **2.5D** (column/span raycast with a y-shear fake for pitch)
   and cannot do VR, so it is *not* a target. The OpenGL/shader renderer is true 3D (real frustum + 3D
   pitch when `OGL_Flag_MimicSW` is off, `Rasterizer_Shader.cpp:88,120`) and is the only viable path —
   but it won't link on GLES until the Phase 1 modernization (plan §2.1). So the toolchain is validated
   first with a minimal SDL+GLES **smoke app** (`android/jni/smoke_main.cpp`, `A1_SMOKE_TEST` option),
   then we do the Phase 1 renderer port. No software-renderer milestone.
3. **Data/asset paths** — Android apps don't have a writable CWD; point preferences/saves at
   `SDL_AndroidGetInternalStoragePath()` and load game data from assets or external storage.
4. **`HAVE_*` config** — provide an Android `config.h` defining only the supported features
   (HAVE_OPENGL, HAVE_SDL_IMAGE, etc.) and leaving the deferred ones undefined.

---

## 5. Open items to resolve while standing this up
- vcpkg `arm64-android` triplet vs. SupportLibs prebuilts — try vcpkg first; fall back to SupportLibs
  for anything that won't build.
- Where Phase-2 swaps SDL's GL context for TBXR's OpenXR-owned EGL context — keep the renderer's
  context/FBO assumptions abstracted so that swap is localized.
- Game data packaging: Marathon scenario data licensing for bundling vs. user-provided files.
