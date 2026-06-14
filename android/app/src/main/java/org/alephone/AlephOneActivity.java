package org.alephone;

import org.libsdl.app.SDLActivity;

/**
 * Aleph One entry activity for Phase 0 (flat / non-VR).
 *
 * SDLActivity loads the native libraries returned by {@link #getLibraries()} and then calls
 * {@code SDL_main} (which is Aleph One's {@code main()} in Source_Files/main.cpp, renamed by
 * SDL2/SDL_main.h on Android). No engine entry-point change is required.
 *
 * Phase 2 (VR) will replace this with an OpenXR-hosted activity where TBXR owns the EGL context;
 * see docs/VR_PORT_PLAN.md.
 */
public class AlephOneActivity extends SDLActivity {

    @Override
    protected String[] getLibraries() {
        // vcpkg's arm64-android triplet builds everything STATIC, so SDL2 (and
        // SDL2_image/ttf, OpenAL, etc.) are linked into libmain.so — there is no
        // separate libSDL2.so to load. SDL's JNI is wired via JNI_OnLoad, which the
        // CMake build force-keeps (-u JNI_OnLoad) so RegisterNatives runs on load.
        return new String[] { "main" };
    }

    @Override
    protected String[] getArguments() {
        // Command-line args passed to Aleph One's main(). Point at on-device data here later.
        return new String[] {};
    }

    /**
     * Immersive VR (Phase 2): the OpenXR compositor — not the 2D SurfaceView — owns the display, so
     * the activity window never holds stable focus. SDLActivity gates starting SDL_main (and
     * pauses) on {@code mHasFocus}, so without this the engine thread never launches. Force
     * "focused" so SDL starts and keeps running while OpenXR drives presentation.
     */
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(true);
    }
}
