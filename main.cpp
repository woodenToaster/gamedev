#include "SDL.h"
#include "stdio.h"

int main(int argc, char** argv) {
    const int windowWidth = 640;
    const int windowHeight = 480;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window;
    window = SDL_CreateWindow("gamedev",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              windowWidth,
                              windowHeight,
                              0);

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    Uint32 rmask;
    Uint32 gmask;
    Uint32 bmask;
    Uint32 amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_Surface* surface;
    surface = SDL_CreateRGBSurface(0,
                                   windowWidth,
                                   windowHeight,
                                   32,
                                   rmask,
                                   gmask,
                                   bmask,
                                   amask);

    if (surface == NULL) {
        SDL_Log("SDL_CreateRGBSurface() failed: %s", SDL_GetError());
        exit(1);
    }

    getchar();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}