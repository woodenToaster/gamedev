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

    // Frame stats
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

    // Colors
    Uint32 green = SDL_MapRGB(windowSurface->format, 0, 255, 0);
    Uint32 blue = SDL_MapRGB(windowSurface->format, 0, 0, 255);
    Uint32 yellow = SDL_MapRGB(windowSurface->format, 235, 245, 65);

    // Map
    // const int map_cols = 12;
    // const int map_rows = 10;

    // Uint8 map[map_rows][map_cols] = {
    //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    // };
    const int map_rows = 6;
    const int map_cols = 8;
    Uint8 map[map_rows][map_cols] = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
    };
    // Camera
    SDL_Rect viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.w = window_width;
    viewport.h = window_height;

    int y_movement_threshold = y_tiles_per_screen / 2;
    int x_movement_threshold = x_tiles_per_screen / 2;

    SDL_Rect center;
    center.x = window_width / 2;
    center.y = window_height / 2;
    center.w = sprite_rect.w;
    center.h = sprite_rect.h;

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
                        // dest_rect.x += sprite_speed;
                        // sprite_rect.y = 2 * h_increment;
                        current_tile.x += tile_width;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT ||
                        event.key.keysym.scancode == SDL_SCANCODE_H) {
                        // dest_rect.x -= sprite_speed;
                        // sprite_rect.y = 1 * h_increment;
                        current_tile.x -= tile_width;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP ||
                        event.key.keysym.scancode == SDL_SCANCODE_K) {
                        // dest_rect.y -= sprite_speed;
                        // sprite_rect.y = 3 * h_increment;
                        current_tile.y -= tile_height;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_DOWN ||
                        event.key.keysym.scancode == SDL_SCANCODE_J) {
                        // dest_rect.y += sprite_speed;
                        // sprite_rect.y = 0 * h_increment;
                        current_tile.y += tile_height;
                    }
                    map_tile_coord.x = current_tile.x / tile_width;
                    map_tile_coord.y = current_tile.y / tile_height;

                    // Clamp tile
                    if (current_tile.x < 0) {
                        current_tile.x = 0;
                        map_tile_coord.x = 0;
                    }
                    if (current_tile.x > map_cols * tile_width) {
                        current_tile.x = map_cols * tile_width;
                        map_tile_coord.x = map_cols;
                    }
                    if (current_tile.y < 0) {
                        current_tile.y = 0;
                        map_tile_coord.y = 0;
                    }
                    if (current_tile.y > map_rows * tile_height) {
                        current_tile.y = map_rows * tile_height;
                        map_tile_coord.y = map_rows;
                    }

                    // current_tile.x = ((dest_rect.x + (dest_rect.w / 2)) / 80) * 80;
                    // current_tile.y = ((dest_rect.y + (dest_rect.h / 2)) / 80) * 80;
                    printf("Map tile coord: {%d, %d}\n", map_tile_coord.x, map_tile_coord.y);
            }
        }

        // int first_tile_coordinate_x = current_tile.x / tile_width;
        // int first_tile_coordinate_y = current_tile.y / tile_height;

        int screen_y = screen_clamp_y(map_tile_coord.y - y_movement_threshold);
        int screen_x = screen_clamp_x(map_tile_coord.x - x_movement_threshold);


        // Draw visible portion of map, tile by tile
        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 8; ++col) {
                tile_to_draw.x = col * tile_width;
                tile_to_draw.y = row * tile_height;

                // See which tile needs to be drawn here. Determined relative
                // to current tile in map coordinates.
                // int map_x = screen_x + j;
                // int map_y = screen_y + i;
                // int tile = map[map_x][map_y];

                if (map[row][col]) {
                    SDL_FillRect(windowSurface, &tile_to_draw, green);
                }
                else {
                    SDL_FillRect(windowSurface, &tile_to_draw, blue);
                }
            }
        }
        SDL_Rect current_tile_dest = current_tile;

        if (current_tile.x > window_width / 2 &&
            current_tile.x < (map_cols * tile_width) - (x_movement_threshold * tile_width)) {
            current_tile_dest.x = center.x;
        }
        if (current_tile.y > window_height / 2 &&
            current_tile.y < (map_rows * tile_height) - (y_movement_threshold * tile_height)) {
            current_tile_dest.y = center.y;
        }
        SDL_FillRect(windowSurface, &current_tile_dest, yellow);

        SDL_Rect sprite_blit_rect = dest_rect;

        // if (dest_rect.x > x_movement_threshold * tile_width &&
        //     dest_rect.x < (map_cols * tile_width) - (x_movement_threshold * tile_width)) {
        //     sprite_blit_rect.x = center.x;
        // }
        // if (dest_rect.y > y_movement_threshold * tile_height &&
        //     dest_rect.y < (map_rows * tile_height) - (y_movement_threshold * tile_height)) {
        //     sprite_blit_rect.y = center.y;
        // }

        // SDL_BlitSurface(sprite_sheet, &sprite_rect, windowSurface, &sprite_blit_rect);
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
