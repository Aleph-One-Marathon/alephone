# Aleph One — Android native build (Phase 0). See docs/ANDROID_BUILD.md
#
# Produces libmain.so, loaded by SDLActivity, which calls SDL_main (== Aleph One main()).
# This is a STARTING SKELETON: the source globs and link set are correct in shape, but
# exclusions and a couple of deps will need iteration against a live NDK build.

cmake_minimum_required(VERSION 3.22)
project(AlephOne LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Repo root is two levels up from android/jni/
set(A1_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(SRC ${A1_ROOT}/Source_Files)

# Toolchain validation toggle. While ON, build a tiny SDL+GLES smoke app as libmain.so
# instead of the engine (the engine can't link until the Phase 1 GLES renderer port).
# Flip OFF (in app/build.gradle cmake arguments) to build the real engine.
option(A1_SMOKE_TEST "Build minimal SDL+GLES smoke app instead of the engine" ON)

# SDL2 is needed for both the smoke app and the engine.
find_package(SDL2 CONFIG REQUIRED)

if(A1_SMOKE_TEST)
    # --- Minimal toolchain smoke test (throwaway) ---
    add_library(main SHARED ${CMAKE_CURRENT_SOURCE_DIR}/smoke_main.cpp)
    target_link_libraries(main PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        GLESv3 EGL android log)
    target_link_options(main PRIVATE "-Wl,-u,JNI_OnLoad")
    return()
endif()

# ===================== Full engine build (Phase 1+) =====================
# Enable the hand-written android/config.h (defines HAVE_OPENGL, HAVE_SDL_IMAGE, etc.)
add_compile_definitions(HAVE_CONFIG_H=1)

# ---- Dependencies (vcpkg arm64-android) ----
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
# asio is header-only (find via vcpkg include dirs)

# ---- Engine sources ----
# Directory set mirrors configure.ac's AC_CONFIG_FILES.
# (RenderMain/gl_es_compat.cpp -- the GLES fixed-function shim -- is picked up by the glob below.)
# Root Source_Files/*.cpp (shell.cpp, main.cpp, ...) globbed explicitly: an empty "" element in
# a CMake list is silently dropped by foreach, which previously skipped the whole root directory.
file(GLOB A1_SOURCES ${SRC}/*.cpp ${SRC}/*.c)
set(A1_SUBDIRS
    CSeries Files GameWorld Input Lua Misc ModelView
    Network Network/Metaserver RenderMain RenderOther
    Sound TCPMess Videos XML
)
foreach(d ${A1_SUBDIRS})
    file(GLOB _s ${SRC}/${d}/*.cpp ${SRC}/${d}/*.c)
    list(APPEND A1_SOURCES ${_s})
endforeach()

# Sources to exclude from the glob (platform-specific or competing entry points):
#  - Network/StandaloneHub/standalone_hub_main.cpp  (second main())
#  - any *_MacOSX.cpp / *_Win32.cpp / Carbon files
#  - Steam/* unless HAVE_STEAM
# TODO: enumerate exact exclusions on first compile pass.
list(FILTER A1_SOURCES EXCLUDE REGEX "StandaloneHub")
# Platform-specific implementations (case varies). Android keeps the POSIX variants.
list(FILTER A1_SOURCES EXCLUDE REGEX "_(MacOSX|Win32|Carbon|cocoa|macosx|win32)\\.")
list(FILTER A1_SOURCES EXCLUDE REGEX "thread_priority_sdl_(win32|macosx|dummy)")
# zziplib is disabled for Android (HAVE_ZZIP undefined); its RWops shim needs zzip headers.
list(FILTER A1_SOURCES EXCLUDE REGEX "SDL_rwops_zzip")
# network_dummy.cpp is the stub used when networking is off; we build the real network.cpp.
list(FILTER A1_SOURCES EXCLUDE REGEX "network_dummy")

add_library(main SHARED ${A1_SOURCES})

# ---- OpenXR (Phase 2 VR) ----
# Current official Khronos OpenXR loader for Android, delivered as a Prefab module by the
# org.khronos.openxr:openxr_loader_for_android AAR (see app/build.gradle). Supplies the loader .so
# (auto-packaged) + headers via the imported target OpenXR::openxr_loader, and supports the current
# Horizon OS runtime broker (the stale bundled loader did not). The VR backend is
# Source_Files/RenderMain/vr_openxr.cpp.
find_package(OpenXR REQUIRED CONFIG)   # prefab package is "OpenXR"; target OpenXR::openxr_loader

target_include_directories(main PRIVATE
    ${SRC} ${SRC}/CSeries ${SRC}/Files ${SRC}/GameWorld ${SRC}/Input
    ${SRC}/Lua ${SRC}/Misc ${SRC}/ModelView ${SRC}/Network
    ${SRC}/Network/Metaserver ${SRC}/RenderMain ${SRC}/RenderOther
    ${SRC}/Sound ${SRC}/TCPMess ${SRC}/Videos ${SRC}/XML
    ${A1_ROOT}/android            # for a generated Android config.h
)

# Static triplet: prefer the -static targets, fall back to the alias if present.
target_link_libraries(main PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    Boost::filesystem Boost::system
    SndFile::sndfile OpenAL::OpenAL ZLIB::ZLIB
    OpenXR::openxr_loader
    GLESv3 EGL android log
)

# SDL2 is static: its native methods are registered from JNI_OnLoad (SDL_android.c),
# which nothing in our C++ references, so the linker would drop it. Force-keep it so
# System.loadLibrary("main") triggers SDL's RegisterNatives. (SDL_main is defined in
# our own main.cpp, so it survives without help and stays default-visible for dlsym.)
target_link_options(main PRIVATE "-Wl,-u,JNI_OnLoad")
