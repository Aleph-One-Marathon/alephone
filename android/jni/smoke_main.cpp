// Minimal SDL2 + OpenGL ES 3 smoke test for the Aleph One Android/Quest toolchain.
//
// Purpose: prove the parts that are brand-new and hardest to debug, BEFORE the
// months-long GLES renderer port:
//   1. vcpkg's static arm64-android libs link into a single libmain.so
//   2. SDL's JNI_OnLoad survives static linking (-Wl,-u,JNI_OnLoad) and RegisterNatives runs
//   3. SDLActivity loads libmain.so and calls SDL_main
//   4. an OpenGL ES 3 context comes up on the device
// It clears the screen to a slowly cycling colour so success is obvious in-headset.
//
// Built instead of the engine when -DA1_SMOKE_TEST=ON (default for now). Throwaway.

#include <SDL.h>
#include <SDL_main.h>
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "AlephOneSmoke", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "AlephOneSmoke", __VA_ARGS__)

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    LOGI("smoke: entering SDL_main");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(
        "Aleph One (smoke)", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    if (!window) {
        LOGE("SDL_CreateWindow failed: %s", SDL_GetError());
        return 1;
    }

    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    if (!ctx) {
        LOGE("SDL_GL_CreateContext failed: %s", SDL_GetError());
        return 1;
    }

    LOGI("GL_VERSION  : %s", glGetString(GL_VERSION));
    LOGI("GL_RENDERER : %s", glGetString(GL_RENDERER));
    LOGI("GL_VENDOR   : %s", glGetString(GL_VENDOR));

    bool running = true;
    float t = 0.0f;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_AC_BACK) running = false;
        }

        t += 0.01f;
        glClearColor(0.5f + 0.5f * SDL_sinf(t),
                     0.5f + 0.5f * SDL_sinf(t + 2.094f),
                     0.5f + 0.5f * SDL_sinf(t + 4.188f), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
