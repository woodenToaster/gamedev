#include "SDL.h"
#include "SDL_image.h"
#include "stdio.h"
#include "stdlib.h"


int main(int argc, char** argv) {
    const int window_width = 640;
    const int window_height = 480;

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window;
    window = SDL_CreateWindow(
        "gamedev",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    Uint32 frames = 0;
    Uint32 start = SDL_GetTicks();
    bool running = true;

    SDL_Surface *sprite_sheet = IMG_Load("sprites/dude.png");
    const int sprite_sheet_width = 256;
    const int sprite_sheet_height = 256;
    const int w_increment = sprite_sheet_width / 4;
    const int h_increment = sprite_sheet_height / 4;

    SDL_Rect sprite_rect;
    sprite_rect.x = 0;
    sprite_rect.y = 0;
    sprite_rect.w = w_increment;
    sprite_rect.h = h_increment;

    float sprite_speed = 10.0f;

    SDL_Rect dest_rect;
    dest_rect.x = 0;
    dest_rect.y = 0;
    dest_rect.w = w_increment;
    dest_rect.h = h_increment;

    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

    if (!sprite_sheet) {
        printf("Could not load sprite sheet: %s\n", SDL_GetError());
    }

    int tile_width = 80;
    int tile_height = 80;

    SDL_Rect tile_rect;
    tile_rect.w = tile_width;
    tile_rect.h = tile_height;

    const int map_width = 12;
    const int map_height = 10;

    Uint8 map[map_height][map_width] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    SDL_Rect current_tile;
    current_tile.x = sprite_rect.x;
    current_tile.y = sprite_rect.y;
    current_tile.w = tile_width;
    current_tile.h = tile_height;

    SDL_Rect camera;
    camera.x = 0;
    camera.y = 0;
    camera.w = window_width;
    camera.h = window_height;

    while(running) {

        if (SDL_GetTicks() > start + 1000) {
            printf("FPS: %d\n", frames);
            frames = 0;
            start = SDL_GetTicks();
        }

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYUP:
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        running = false;
                    }
                    break;
                case SDL_QUIT:
                    running = false;
                case SDL_KEYDOWN:
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                        dest_rect.x += sprite_speed;
                        sprite_rect.y = 2 * h_increment;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                        dest_rect.x -= sprite_speed;
                        sprite_rect.y = 1 * h_increment;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                        dest_rect.y -= sprite_speed;
                        sprite_rect.y = 3 * h_increment;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                        dest_rect.y += sprite_speed;
                        sprite_rect.y = 0 * h_increment;
                    }
                    current_tile.x = ((dest_rect.x + (dest_rect.w / 2)) / 80) * 80;
                    current_tile.y = ((dest_rect.y + (dest_rect.h / 2)) / 80) * 80;
            }
        }

        for (int i = 0; i < window_height / tile_height; ++i) {
            for (int j = 0; j < window_width / tile_width; ++j) {
                tile_rect.x = j * tile_width;
                tile_rect.y = i * tile_height;

                if (map[i][j]) {
                    SDL_FillRect(
                        windowSurface,
                        &tile_rect,
                        SDL_MapRGB(windowSurface->format, 0, 255, 0)
                    );
                }
                else {
                    SDL_FillRect(
                        windowSurface,
                        &tile_rect,
                        SDL_MapRGB(windowSurface->format, 0, 0, 255)
                    );
                }
                SDL_FillRect(
                    windowSurface,
                    &current_tile,
                    SDL_MapRGB(windowSurface->format, 235, 245, 65)
                );
            }
        }

        SDL_BlitSurface(sprite_sheet, &sprite_rect, windowSurface, &dest_rect);
        SDL_UpdateWindowSurface(window);

        SDL_Delay(33);
        frames++;
    }

    IMG_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}