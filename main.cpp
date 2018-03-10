#include "SDL.h"
#include "SDL_image.h"
#include "stdio.h"
#include "stdlib.h"

int screen_clamp_x(int val) {
    if (val < 0) {
        return 0;
    }
    else if (val > 8) {
        return 8;
    }
    else {
        return val;
    }
}

int screen_clamp_y(int val) {
    if (val < 0) {
        return 0;
    }
    else if (val > 6) {
        return 6;
    }
    else {
        return val;
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Window
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

    // Sprite sheet
    SDL_Surface *sprite_sheet = IMG_Load("sprites/dude.png");
    const int sprite_sheet_width = 256;
    const int sprite_sheet_height = 256;
    const int num_x_sprites = 4;
    const int num_y_sprites = 4;
    const int w_increment = sprite_sheet_width / num_x_sprites;
    const int h_increment = sprite_sheet_height / num_y_sprites;

    SDL_Rect sprite_rect;
    sprite_rect.x = 0;
    sprite_rect.y = 0;
    sprite_rect.w = w_increment;
    sprite_rect.h = h_increment;

    int sprite_speed = 10;

    SDL_Rect dest_rect;
    dest_rect.x = 0;
    dest_rect.y = 0;
    dest_rect.w = w_increment;
    dest_rect.h = h_increment;

    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

    if (!sprite_sheet) {
        printf("Could not load sprite sheet: %s\n", SDL_GetError());
    }

    // Tiles
    int tile_width = 80;
    int tile_height = 80;
    int x_tiles_per_screen = window_width / tile_width;
    int y_tiles_per_screen = window_height / tile_height;

    SDL_Rect tile_to_draw;
    tile_to_draw.w = tile_width;
    tile_to_draw.h = tile_height;

    struct TileCoord {
        int x;
        int y;
    };

    TileCoord map_tile_coord = {0, 0};

    SDL_Rect current_tile;
    current_tile.x = sprite_rect.x;
    current_tile.y = sprite_rect.y;
    current_tile.w = tile_width;
    current_tile.h = tile_height;

    // Map
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

    // Camera
    SDL_Rect camera;
    camera.x = 0;
    camera.y = 0;
    camera.w = window_width;
    camera.h = window_height;

    int y_movement_threshold = y_tiles_per_screen / 2;
    int x_movement_threshold = x_tiles_per_screen / 2;

    // Main loop
    while(running) {

        if (SDL_GetTicks() > start + 1000) {
            // printf("FPS: %d\n", frames);
            frames = 0;
            start = SDL_GetTicks();
        }

        // Input
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
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT ||
                        event.key.keysym.scancode == SDL_SCANCODE_L) {
                        dest_rect.x += sprite_speed;
                        sprite_rect.y = 2 * h_increment;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT ||
                        event.key.keysym.scancode == SDL_SCANCODE_H) {
                        dest_rect.x -= sprite_speed;
                        sprite_rect.y = 1 * h_increment;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP ||
                        event.key.keysym.scancode == SDL_SCANCODE_K) {
                        dest_rect.y -= sprite_speed;
                        sprite_rect.y = 3 * h_increment;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_DOWN ||
                        event.key.keysym.scancode == SDL_SCANCODE_J) {
                        dest_rect.y += sprite_speed;
                        sprite_rect.y = 0 * h_increment;
                    }
                    current_tile.x = ((dest_rect.x + (dest_rect.w / 2)) / 80) * 80;
                    current_tile.y = ((dest_rect.y + (dest_rect.h / 2)) / 80) * 80;
                    map_tile_coord.x = current_tile.x / tile_width;
                    map_tile_coord.y = current_tile.y / tile_height;
                    printf("Map tile coord: {%d, %d}\n", map_tile_coord.x, map_tile_coord.y);
            }
        }

        int first_tile_coordinate_x = current_tile.x / tile_width;
        int first_tile_coordinate_y = current_tile.y / tile_height;

        int screen_y = screen_clamp_y(map_tile_coord.y - y_movement_threshold);
        int screen_x = screen_clamp_x(map_tile_coord.x - x_movement_threshold);

        // Draw visible portion of map, tile by tile
        for (int i = 0; i < y_tiles_per_screen; ++i) {
            for (int j = 0; j < x_tiles_per_screen; ++j) {
                tile_to_draw.x = j * tile_width;
                tile_to_draw.y = i * tile_height;

                // See which tile needs to be draw here. Determined relative
                // to current tile in map coordinates.
                int map_x = screen_x + j;
                int map_y = screen_y + i;
                int tile = map[map_x][map_y];

                if (tile) {
                    SDL_FillRect(
                        windowSurface,
                        &tile_to_draw,
                        SDL_MapRGB(windowSurface->format, 0, 255, 0)
                    );
                }
                else {
                    SDL_FillRect(
                        windowSurface,
                        &tile_to_draw,
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

        // Convert map pixel coordinates of dest_rect to screen pixel coordinates.
        // if (dest_rect.y > screen_y) {
        //     dest_rect.y = screen_y;
        // }
        // if (dest_rect.x > screen_x) {
        //     dest_rect.x = screen_x;
        // }
        SDL_BlitSurface(sprite_sheet, &sprite_rect, windowSurface, &dest_rect);
        SDL_UpdateWindowSurface(window);

        SDL_Delay(33);
        frames++;
    }

    // Cleanup
    IMG_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
