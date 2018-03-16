#include "SDL.h"
#include "SDL_image.h"
#include "stdio.h"
#include "stdlib.h"

Uint32 NONE = 0x0;
Uint32 SOLID = 0x01;
Uint32 WATER = 0x01 << 1;
Uint32 QUICKSAND = 0x01 << 2;
Uint32 STICKY = 0x01 << 3;
Uint32 REVERSE = 0x01 << 4;

struct Point {
    float x;
    float y;
};

int clamp(int val, int min, int max) {
    if (val < min) {
        return min;
    }
    else if (val > max) {
        return max;
    }
    else {
        return val;
    }
}

bool is_solid_tile(Uint64 t) {
    Uint32 flags = (Uint32)(t >> 32);
    return flags & SOLID;
}

bool is_slow_tile(Uint64 t) {
    Uint32 flags = (Uint32)(t >> 32);
    return flags & QUICKSAND;
}

Uint32 get_color_from_tile(Uint64 t) {
    return (Uint32)(t & 0xFFFFFFFF);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Window
    const int screen_width = 640;
    const int screen_height = 480;

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window;
    window = SDL_CreateWindow(
        "gamedev",
        30,
        50,
        screen_width,
        screen_height,
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
    dest_rect.x = 85;
    dest_rect.y = 85;
    dest_rect.w = w_increment;
    dest_rect.h = h_increment;

    Point hero_collision_pt;
    hero_collision_pt.x = (float)dest_rect.x / 2;
    hero_collision_pt.y = (float)dest_rect.y + dest_rect.h;
    bool in_quicksand = false;

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    if (!sprite_sheet) {
        printf("Could not load sprite sheet: %s\n", SDL_GetError());
    }

    // Tiles
    int tile_width = 80;
    int tile_height = 80;

    SDL_Rect current_tile;
    current_tile.x = dest_rect.x / tile_width;
    current_tile.y = dest_rect.y / tile_height;
    current_tile.w = tile_width;
    current_tile.h = tile_height;

    // Colors
    Uint32 green = SDL_MapRGB(window_surface->format, 0, 255, 0);
    Uint32 blue = SDL_MapRGB(window_surface->format, 0, 0, 255);
    Uint32 yellow = SDL_MapRGB(window_surface->format, 235, 245, 65);
    Uint32 brown = SDL_MapRGB(window_surface->format, 153, 102, 0);

    enum colors_enum {GREEN, BLUE, YELLOW, BROWN};
    Uint32 colors[] = {green, blue, yellow, brown};

    struct Tile {
        Uint32 flags;
        Uint32 color;
        Uint64 as_u64() {
            return (Uint64)flags << 32 | color;
        }
    };

    Tile wall;
    wall.flags = SOLID;
    wall.color = colors[GREEN];
    Uint64 w = wall.as_u64();

    Tile floor;
    floor.flags = NONE;
    floor.color = colors[BLUE];
    Uint64 f = floor.as_u64();

    Tile mud;
    mud.flags = QUICKSAND;
    mud.color = colors[BROWN];
    Uint64 m = mud.as_u64();

    // Map
    const int map_cols = 12;
    const int map_rows = 10;

    Uint64 map[map_rows][map_cols] = {
        {w, w, w, w, w, w, w, w, w, w, w, w},
        {w, f, f, w, f, f, f, f, f, f, f, f},
        {w, f, f, w, f, f, f, f, f, f, w, f},
        {w, f, f, f, f, f, f, f, f, f, f, m},
        {w, f, f, w, f, f, f, f, f, f, f, f},
        {w, f, f, w, w, w, w, f, f, w, f, f},
        {w, f, f, f, f, f, w, f, f, w, f, f},
        {w, f, f, f, f, f, w, f, f, w, f, m},
        {w, f, f, f, f, f, f, f, f, w, f, f},
        {w, w, w, w, w, w, w, w, w, w, w, w}
#if 0
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
#endif
    };

    int map_width_pixels = map_cols * tile_width;
    int map_height_pixels = map_rows * tile_height;

    SDL_Surface* map_surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        map_width_pixels,
        map_height_pixels,
        32,
        SDL_PIXELFORMAT_RGB888
    );

    // Camera
    SDL_Rect camera;
    camera.x = 0;
    camera.y = 0;
    camera.w = screen_width;
    camera.h = screen_height;

    int max_camera_x = map_width_pixels - camera.w;
    int max_camera_y = map_height_pixels - camera.h;

    int y_pixel_movement_threshold = screen_height / 2;
    int x_pixel_movement_threshold = screen_width / 2;

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
                    SDL_Rect saved_position = dest_rect;
                    SDL_Rect saved_camera = camera;
                    SDL_Rect saved_tile = current_tile;

                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT ||
                        event.key.keysym.scancode == SDL_SCANCODE_L) {
                        dest_rect.x += sprite_speed;
                        sprite_rect.y = 2 * h_increment;

                        if (dest_rect.x > x_pixel_movement_threshold &&
                            camera.x < max_camera_x) {
                            camera.x += sprite_speed;
                        }
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT ||
                        event.key.keysym.scancode == SDL_SCANCODE_H) {
                        dest_rect.x -= sprite_speed;
                        sprite_rect.y = 1 * h_increment;

                        if (dest_rect.x <
                            map_width_pixels - x_pixel_movement_threshold &&
                            camera.x > 0) {
                            camera.x -= sprite_speed;
                        }
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP ||
                        event.key.keysym.scancode == SDL_SCANCODE_K) {
                        dest_rect.y -= sprite_speed;
                        sprite_rect.y = 3 * h_increment;

                        if (dest_rect.y <
                            map_height_pixels - y_pixel_movement_threshold &&
                            camera.y > 0) {
                            camera.y -= sprite_speed;
                        }
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_DOWN ||
                        event.key.keysym.scancode == SDL_SCANCODE_J) {
                        dest_rect.y += sprite_speed;
                        sprite_rect.y = 0 * h_increment;

                        if (dest_rect.y > y_pixel_movement_threshold &&
                            camera.y < max_camera_y) {
                            camera.y += sprite_speed;
                        }
                    }

                    // Clamp camera
                    camera.x = clamp(camera.x, 0, max_camera_x);
                    camera.y = clamp(camera.y, 0, max_camera_y);

                    // Clamp hero
                    dest_rect.x = clamp(dest_rect.x, 0, map_width_pixels - dest_rect.w);
                    dest_rect.y = clamp(dest_rect.y, 0, map_height_pixels - dest_rect.h);

                    hero_collision_pt.y = (float)dest_rect.y + dest_rect.h;
                    hero_collision_pt.x = dest_rect.x + dest_rect.w / 2.0f;

                    current_tile.x = ((int)hero_collision_pt.x / 80) * 80;
                    current_tile.y = ((int)hero_collision_pt.y / 80) * 80;

                    // Check hero collision with walls
                    int map_coord_x = current_tile.y / tile_height;
                    int map_coord_y = current_tile.x / tile_width;
                    Uint64 tile_at_hero_position = map[map_coord_x][map_coord_y];

                    if (is_solid_tile(tile_at_hero_position)) {
                        // Collsions. Reverse movement
                        camera = saved_camera;
                        dest_rect = saved_position;
                        current_tile = saved_tile;
                    }
                    if (is_slow_tile(tile_at_hero_position) && !in_quicksand) {
                        sprite_speed -= 5;
                        in_quicksand = true;
                    }
                    else if (in_quicksand) {
                        sprite_speed += 5;
                        in_quicksand = false;
                    }


            }
        }

        // Draw map
        for (int row = 0; row < map_rows; ++row) {
            for (int col = 0; col < map_cols; ++col) {
                SDL_Rect tile_rect;
                tile_rect.x = col * tile_width;
                tile_rect.y = row * tile_height;
                tile_rect.w = tile_width;
                tile_rect.h = tile_height;

                Uint32 fill_color = get_color_from_tile(map[row][col]);
                SDL_FillRect(map_surface, &tile_rect, fill_color);
            }
        }

        SDL_FillRect(map_surface, &current_tile, yellow);
        // Draw sprite on map
        SDL_BlitSurface(sprite_sheet, &sprite_rect, map_surface, &dest_rect);
        // Draw portion of map visible to camera on the screen
        SDL_BlitSurface(map_surface, &camera, window_surface, NULL);

        SDL_UpdateWindowSurface(window);

        SDL_Delay(33);
        frames++;
        fflush(stdout);
    }

    // Cleanup
    SDL_FreeSurface(sprite_sheet);
    SDL_FreeSurface(map_surface);
    IMG_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
