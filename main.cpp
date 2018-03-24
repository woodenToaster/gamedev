#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "stdio.h"
#include "stdlib.h"

const Uint32 NONE = 0x0;
const Uint32 SOLID = 0x01;
const Uint32 WATER = 0x01 << 1;
const Uint32 QUICKSAND = 0x01 << 2;
const Uint32 STICKY = 0x01 << 3;
const Uint32 REVERSE = 0x01 << 4;
const Uint32 WARP = 0x01 << 5;

struct Point {
    float x;
    float y;
};

bool overlaps(SDL_Rect* r1, SDL_Rect* r2) {
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

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

bool is_warp_tile(Uint64 t) {
    Uint32 flags = (Uint32)(t >> 32);
    return flags & WARP;
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL failed to initialize: %s\n", SDL_GetError());
        return 1;
    }

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

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("Could not create renderer: %s\n", SDL_GetError());
    }

    // Frame stats
    Uint32 frames = 0;
    Uint32 start = SDL_GetTicks();
    bool running = true;

    // Sound
    Mix_Chunk* mud_sound = NULL;
    bool mud_sound_playing = false;
    Uint32 mud_sound_delay = SDL_GetTicks();

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        exit(1);
    }

    mud_sound = Mix_LoadWAV("sounds/mud_walk.wav");
    if (mud_sound == NULL) {
        printf("Mix_LoadWAV error: %s\n", Mix_GetError());
        return 1;
    }

    struct Entity {
        SDL_Surface* sprite_sheet;
        SDL_Texture* sprite_texture;
        int sprite_sheet_width;
        int sprite_sheet_height;
        int num_x_sprites;
        int num_y_sprites;
        int w_increment;
        int h_increment;

        SDL_Rect sprite_rect;

        int current_frame;
        int speed;

        SDL_Rect bounding_box;
        SDL_Rect dest_rect;
    };

    // Hero
    Entity hero;
    hero.sprite_sheet = IMG_Load("sprites/dude.png");
    hero.sprite_texture = SDL_CreateTextureFromSurface(renderer, hero.sprite_sheet);
    hero.sprite_sheet_width = 256;
    hero.sprite_sheet_height = 256;
    hero.num_x_sprites = 4;
    hero.num_y_sprites = 4;
    hero.w_increment = hero.sprite_sheet_width / hero.num_x_sprites;
    hero.h_increment = hero.sprite_sheet_height / hero.num_y_sprites;
    hero.sprite_rect.x = 0;
    hero.sprite_rect.y = 0;
    hero.sprite_rect.w = hero.w_increment;
    hero.sprite_rect.h = hero.h_increment;
    hero.current_frame = 0;
    hero.speed = 10;
    hero.dest_rect.x = 85;
    hero.dest_rect.y = 85;
    hero.dest_rect.w = hero.w_increment;
    hero.dest_rect.h = hero.h_increment;
    hero.bounding_box.x = 0;
    hero.bounding_box.y = 0;
    hero.bounding_box.w = 0;
    hero.bounding_box.h = 0;

    SDL_Rect hero_starting_pos = hero.dest_rect;
    Point hero_collision_pt;
    bool hero_is_moving = false;
    bool in_quicksand = false;
    Uint32 next_frame_delay = SDL_GetTicks();

    // Enemy
    Entity harlod;
    harlod.sprite_sheet = IMG_Load("sprites/Harlod_the_caveman.png");
    harlod.sprite_sheet_width = 64;
    harlod.sprite_sheet_height = 64;
    harlod.num_x_sprites = 1;
    harlod.num_y_sprites = 1;
    harlod.w_increment = harlod.sprite_sheet_width / harlod.num_x_sprites;
    harlod.h_increment = harlod.sprite_sheet_height / harlod.num_y_sprites;
    harlod.sprite_rect.x = 0;
    harlod.sprite_rect.y = 0;
    harlod.sprite_rect.w = harlod.w_increment;
    harlod.sprite_rect.h = harlod.h_increment;
    harlod.current_frame = 0;
    harlod.speed = 5;
    harlod.dest_rect.x = 150;
    harlod.dest_rect.y = 150;
    harlod.dest_rect.w = harlod.w_increment;
    harlod.dest_rect.h = harlod.h_increment;

    bool right_is_pressed = false;
    bool left_is_pressed = false;
    bool up_is_pressed = false;
    bool down_is_pressed = false;

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    if (!hero.sprite_sheet || !harlod.sprite_sheet) {
        printf("Could not load sprite sheet: %s\n", SDL_GetError());
    }

    // Colors
    Uint32 green = SDL_MapRGB(window_surface->format, 0, 255, 0);
    Uint32 blue = SDL_MapRGB(window_surface->format, 0, 0, 255);
    Uint32 yellow = SDL_MapRGB(window_surface->format, 235, 245, 65);
    Uint32 brown = SDL_MapRGB(window_surface->format, 153, 102, 0);
    Uint32 rust = SDL_MapRGB(window_surface->format, 153, 70, 77);
    Uint32 magenta = SDL_MapRGB(window_surface->format, 255, 0, 255);

    // Tiles
    int tile_width = 80;
    int tile_height = 80;

    SDL_Rect current_tile;
    current_tile.x = hero.dest_rect.x / tile_width;
    current_tile.y = hero.dest_rect.y / tile_height;
    current_tile.w = tile_width;
    current_tile.h = tile_height;

    struct Tile {
        Uint32 flags;
        Uint32 color;
        Uint64 as_u64() {
            return (Uint64)flags << 32 | color;
        }
    };

    Tile wall;
    wall.flags = SOLID;
    wall.color = green;
    Uint64 w = wall.as_u64();

    Tile floor;
    floor.flags = NONE;
    floor.color = blue;
    Uint64 f = floor.as_u64();

    Tile mud;
    mud.flags = QUICKSAND;
    mud.color = brown;
    Uint64 m = mud.as_u64();

    Tile warp;
    warp.flags = WARP;
    warp.color = rust;
    Uint64 wr = warp.as_u64();

    // Map
    const int map_cols = 12;
    const int map_rows = 10;

    Uint64 map[map_rows][map_cols] = {
        {w, w, w, w, w, w, w, w, w, w, w, w},
        {w, f, f, w, f, f, f, f, f, f, f, f},
        {w, f, f, w, f, f, f, f, f, f, w, f},
        {w, f, f, f, f, f, f, f, f, f, f, m},
        {w, f, f, w, f, f, f, f, f, f, f, f},
        {w, f, f, w, w, w, w, f, f, w, f, wr},
        {w, f, f, f, f, f, w, f, f, w, f, f},
        {w, f, f, f, f, f, w, f, f, w, f, m},
        {w, f, f, f, f, f, f, f, f, w, f, f},
        {w, w, w, w, w, w, w, w, w, w, w, w}
    };

    bool in_map1 = true;

    Uint64 (*current_map)[map_rows][map_cols] = &map;

    Uint64 map2[map_rows][map_cols] = {
        {w, w, w, w, w, w, w, w, w, w, w, w},
        {w, f, f, f, f, f, f, f, f, f, f, w},
        {w, f, f, f, f, f, f, f, f, f, f, w},
        {w, f, f, f, f, f, f, f, f, f, f, w},
        {w, f, f, f, f, f, f, f, f, f, f, w},
        {w, f, f, f, f, wr, f, f, f, f, f, w},
        {w, f, f, f, f, f, f, f, f, f, f, w},
        {w, f, f, f, f, f, f, f, f, f, f, w},
        {w, f, f, f, f, f, f, f, f, f, f, w},
        {w, w, w, w, w, w, w, w, w, w, w, w}
    };

    int map_width_pixels = map_cols * tile_width;
    int map_height_pixels = map_rows * tile_height;

    // Add 1 to each to account for displaying half a tile.
    int tile_rows_per_screen = (screen_height / tile_height) + 1;
    int tile_cols_per_screen = (screen_width / tile_width) + 1;

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

    SDL_Rect camera_starting_pos = camera;

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
            case SDL_KEYUP: {
                SDL_Scancode key = event.key.keysym.scancode;

                if (key == SDL_SCANCODE_ESCAPE) {
                    running = false;
                }
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L) {
                    right_is_pressed = false;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K) {
                    up_is_pressed = false;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J) {
                    down_is_pressed = false;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H) {
                    left_is_pressed = false;
                }
                break;
            }
            case SDL_QUIT: {
                running = false;
                break;
            }
            case SDL_KEYDOWN: {
                SDL_Scancode key = event.key.keysym.scancode;
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L) {
                    right_is_pressed = true;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H) {
                    left_is_pressed = true;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K) {
                    up_is_pressed = true;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J) {
                    down_is_pressed = true;
                }
                break;
            }
            }
        }

        // Update
        SDL_Rect saved_position = hero.dest_rect;
        SDL_Rect saved_camera = camera;
        SDL_Rect saved_tile = current_tile;

        Uint32 now = SDL_GetTicks();

        if (right_is_pressed) {
            hero.dest_rect.x += hero.speed;
            hero.sprite_rect.y = 2 * hero.h_increment;
            hero.sprite_rect.x = hero.current_frame * hero.w_increment;
            if (now > next_frame_delay + 125) {
                hero.current_frame++;
                next_frame_delay = now;
            }
            if (hero.current_frame > hero.num_x_sprites - 1) {
                hero.current_frame = 0;
            }

            if (hero.dest_rect.x > x_pixel_movement_threshold &&
                camera.x < max_camera_x) {
                camera.x += hero.speed;
            }
        }
        if (left_is_pressed) {
            hero.dest_rect.x -= hero.speed;
            hero.sprite_rect.y = 1 * hero.h_increment;
            hero.sprite_rect.x = hero.current_frame * hero.w_increment;

            if (now > next_frame_delay + 125) {
                hero.current_frame++;
                next_frame_delay = now;
            }
            if (hero.current_frame > hero.num_x_sprites - 1) {
                hero.current_frame = 0;
            }

            if (hero.dest_rect.x <
                map_width_pixels - x_pixel_movement_threshold &&
                camera.x > 0) {
                camera.x -= hero.speed;
            }
        }
        if (up_is_pressed) {
            hero.dest_rect.y -= hero.speed;
            hero.sprite_rect.y = 3 * hero.h_increment;
            hero.sprite_rect.x = hero.current_frame * hero.h_increment;
            if (now > next_frame_delay + 125) {
                hero.current_frame++;
                next_frame_delay = now;
            }
            if (hero.current_frame > hero.num_x_sprites - 1) {
                hero.current_frame = 0;
            }

            if (hero.dest_rect.y <
                map_height_pixels - y_pixel_movement_threshold &&
                camera.y > 0) {
                camera.y -= hero.speed;
            }
        }
        if (down_is_pressed) {
            hero.dest_rect.y += hero.speed;
            hero.sprite_rect.y = 0 * hero.h_increment;
            hero.sprite_rect.x = hero.current_frame * hero.w_increment;
            if (now > next_frame_delay + 125) {
                hero.current_frame++;
                next_frame_delay = now;
            }

            if (hero.current_frame > hero.num_x_sprites - 1) {
                hero.current_frame = 0;
            }

            if (hero.dest_rect.y > y_pixel_movement_threshold &&
                camera.y < max_camera_y) {
                camera.y += hero.speed;
            }
        }

        if (saved_position.x != hero.dest_rect.x ||
            saved_position.y != hero.dest_rect.y) {
            hero_is_moving = true;
        }
        else {
            hero_is_moving = false;
        }

        if (!hero_is_moving) {
            hero.current_frame = 0;
            hero.sprite_rect.x = 0;
        }


        // Clamp camera
        camera.x = clamp(camera.x, 0, max_camera_x);
        camera.y = clamp(camera.y, 0, max_camera_y);

        // Clamp hero
        hero.dest_rect.x = clamp(hero.dest_rect.x, 0, map_width_pixels - hero.dest_rect.w);
        hero.dest_rect.y = clamp(hero.dest_rect.y, 0, map_height_pixels - hero.dest_rect.h);

        hero_collision_pt.y = (float)hero.dest_rect.y + hero.dest_rect.h - 10;
        hero_collision_pt.x = hero.dest_rect.x + hero.dest_rect.w / 2.0f;

        current_tile.x = ((int)hero_collision_pt.x / 80) * 80;
        current_tile.y = ((int)hero_collision_pt.y / 80) * 80;

        int map_coord_x = current_tile.y / tile_height;
        int map_coord_y = current_tile.x / tile_width;
        Uint64 tile_at_hero_position = (*current_map)[map_coord_x][map_coord_y];

        // Handle all tiles
        if (is_solid_tile(tile_at_hero_position)) {
            // Collisions. Reverse original state
            camera = saved_camera;
            hero.dest_rect = saved_position;
            current_tile = saved_tile;
        }
        if (is_slow_tile(tile_at_hero_position) && !in_quicksand) {
            hero.speed -= 8;
            in_quicksand = true;
            if (SDL_GetTicks() > mud_sound_delay + 250 && hero_is_moving) {
                mud_sound_playing = true;
                Mix_PlayChannel(-1, mud_sound, 0);
                mud_sound_delay = SDL_GetTicks();
            }
        }
        else if (in_quicksand) {
            hero.speed += 8;
            in_quicksand = false;
        }
        if (is_warp_tile(tile_at_hero_position)) {
            if (in_map1) {
                current_map = &map2;
                in_map1 = false;
            }
            else {
                current_map = &map;
                in_map1 = true;
            }
            hero.dest_rect = hero_starting_pos;
            camera = camera_starting_pos;
        }

        // Center camera over the hero
        // camera.x = hero.dest_rect.x + hero.dest_rect.w / 2;
        // camera.y = hero.dest_rect.y + hero.dest_rect.h / 2;

        // Get tile under camera x,y
        int camera_tile_row = camera.y / tile_height;
        int camera_tile_col = camera.x / tile_width;

        SDL_Rect tile_rect;
        tile_rect.w = tile_width;
        tile_rect.h = tile_height;

        // Draw
        // SDL_RenderClear(renderer);
        for (int row = camera_tile_row;

             row < tile_rows_per_screen + camera_tile_row;
             ++row) {

            for (int col = camera_tile_col;
                 col < tile_cols_per_screen + camera_tile_col;
                 ++col) {

                tile_rect.x = col * tile_width;
                tile_rect.y = row * tile_height;

                Uint32 fill_color = get_color_from_tile((*current_map)[row][col]);
                SDL_FillRect(map_surface, &tile_rect, fill_color);
                // SDL_SetRenderDrawColor(renderer, (fill_color & 0xFF00000) >> 4,
                //                        (fill_color & 0xFF00) >> 2, fill_color & 0xFF, 255);
                // SDL_Rect dest_rect;
                // dest_rect.x = tile_rect.x - camera.x;
                // dest_rect.y = tile_rect.y - camera.y;
                // dest_rect.w = tile_rect.w;
                // dest_rect.h = tile_rect.h;
                // SDL_RenderFillRect(renderer, &dest_rect);
            }
        }

        // Highlight tile under player
        // SDL_FillRect(map_surface, &current_tile, yellow);
        // SDL_SetRenderDrawColor(renderer, 235, 245, 65, 255);
        // SDL_RenderFillRect(renderer, &current_tile);

        // Draw hero bounding box
        hero.bounding_box.x = hero.dest_rect.x + 12;
        hero.bounding_box.y = hero.dest_rect.y + 10;
        hero.bounding_box.w = hero.dest_rect.w - 25;
        hero.bounding_box.h = hero.dest_rect.h - 14;

        SDL_Rect bb_top;
        SDL_Rect bb_bottom;
        SDL_Rect bb_left;
        SDL_Rect bb_right;

        bb_top.x = hero.bounding_box.x;
        bb_top.y = hero.bounding_box.y;
        bb_top.w = hero.bounding_box.w;
        bb_top.h = 2;

        bb_left.x = hero.bounding_box.x;
        bb_left.y = hero.bounding_box.y;
        bb_left.w = 2;
        bb_left.h = hero.bounding_box.h;

        bb_right.x = hero.bounding_box.x + hero.bounding_box.w;
        bb_right.y = hero.bounding_box.y;
        bb_right.w = 2;
        bb_right.h = hero.bounding_box.h;

        bb_bottom.x = hero.bounding_box.x;
        bb_bottom.y = hero.bounding_box.y + hero.bounding_box.h;
        bb_bottom.w = hero.bounding_box.w + 2;
        bb_bottom.h = 2;


        SDL_FillRect(map_surface, &bb_top, magenta);
        SDL_FillRect(map_surface, &bb_left, magenta);
        SDL_FillRect(map_surface, &bb_right, magenta);
        SDL_FillRect(map_surface, &bb_bottom, magenta);

        // Draw harlod bounding box
        harlod.bounding_box.x = harlod.dest_rect.x;
        harlod.bounding_box.y = harlod.dest_rect.y;
        harlod.bounding_box.w = harlod.dest_rect.w;
        harlod.bounding_box.h = harlod.dest_rect.h;

        bb_top.x = harlod.bounding_box.x;
        bb_top.y = harlod.bounding_box.y;
        bb_top.w = harlod.bounding_box.w;
        bb_top.h = 2;

        bb_left.x = harlod.bounding_box.x;
        bb_left.y = harlod.bounding_box.y;
        bb_left.w = 2;
        bb_left.h = harlod.bounding_box.h;

        bb_right.x = harlod.bounding_box.x + harlod.bounding_box.w;
        bb_right.y = harlod.bounding_box.y;
        bb_right.w = 2;
        bb_right.h = harlod.bounding_box.h;

        bb_bottom.x = harlod.bounding_box.x;
        bb_bottom.y = harlod.bounding_box.y + harlod.bounding_box.h;
        bb_bottom.w = harlod.bounding_box.w + 2;
        bb_bottom.h = 2;


        SDL_FillRect(map_surface, &bb_top, magenta);
        SDL_FillRect(map_surface, &bb_left, magenta);
        SDL_FillRect(map_surface, &bb_right, magenta);
        SDL_FillRect(map_surface, &bb_bottom, magenta);

        // Check hero/harlod collisions
        if (overlaps(&hero.bounding_box, &harlod.bounding_box)) {
            // Draw overlapping bounding boxes
            SDL_Rect overlap_box;
            if (hero.bounding_box.x > harlod.bounding_box.x) {
                overlap_box.x = hero.bounding_box.x;
                overlap_box.w = harlod.bounding_box.x + harlod.bounding_box.w -
                    hero.bounding_box.x;
                overlap_box.w = min(overlap_box.w, hero.bounding_box.w);
            }
            else {
                overlap_box.x = harlod.bounding_box.x;
                overlap_box.w = hero.bounding_box.x + hero.bounding_box.w -
                    harlod.bounding_box.x;
                overlap_box.w = min(overlap_box.w, harlod.bounding_box.w);
            }

            if (hero.bounding_box.y > harlod.bounding_box.y) {
                overlap_box.y = hero.bounding_box.y;
                overlap_box.h = harlod.bounding_box.y + harlod.bounding_box.h -
                    hero.bounding_box.y;
                overlap_box.h = min(overlap_box.h, hero.bounding_box.h);
            }
            else {
                overlap_box.y = harlod.bounding_box.y;
                overlap_box.h = hero.bounding_box.y + hero.bounding_box.h -
                    harlod.bounding_box.y;
                overlap_box.h = min(overlap_box.h, harlod.bounding_box.h);
            }
            SDL_FillRect(map_surface, &overlap_box, magenta);

            // do pixel collision
        }

        // Draw sprites on map
        SDL_BlitSurface(hero.sprite_sheet, &hero.sprite_rect, map_surface, &hero.dest_rect);
        // SDL_RenderCopy(renderer, hero.sprite_texture, &hero.sprite_rect, &hero.dest_rect);
        if (!in_map1) {
            SDL_BlitSurface(
                harlod.sprite_sheet,
                &harlod.sprite_rect,
                map_surface,
                &harlod.dest_rect
            );
        }
        // Draw portion of map visible to camera on the screen
        SDL_BlitSurface(map_surface, &camera, window_surface, NULL);

        SDL_UpdateWindowSurface(window);
        // SDL_RenderPresent(renderer);

        SDL_Delay(33);
        frames++;
        fflush(stdout);
    }

    // Cleanup
    SDL_FreeSurface(hero.sprite_sheet);
    SDL_FreeSurface(map_surface);
    SDL_DestroyTexture(hero.sprite_texture);
    Mix_FreeChunk(mud_sound);
    Mix_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
