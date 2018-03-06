#include "SDL.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc, char** argv) {
    const int windowWidth = 640;
    const int windowHeight = 480;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window;
    window = SDL_CreateWindow(
        "gamedev",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    Uint32 frames = 0;
    Uint32 start = SDL_GetTicks();
    bool running = true;
    Uint8 r, g, b;

    while(running) {

        if (SDL_GetTicks() > start + 1000) {
            printf("FPS: %d\n", frames);
            frames = 0;
            start = SDL_GetTicks();
            r = rand() % 256;
            g = rand() % 256;
            b = rand() % 256;
        }

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYUP:
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        running = false;
                    }
                    break;
            }
        }

        SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
        SDL_FillRect(windowSurface, NULL, SDL_MapRGB(windowSurface->format, r, g, b));
	    SDL_UpdateWindowSurface(window);

        SDL_Delay(30);
        frames++;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}