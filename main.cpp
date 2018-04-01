#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "entity.cpp"
#include "tile_map.cpp"
#include "gamedev_math.h"

struct Point {
    float x;
    float y;
};

bool overlaps(SDL_Rect* r1, SDL_Rect* r2) {
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
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

    // SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // if (renderer == NULL) {
    //     printf("Could not create renderer: %s\n", SDL_GetError());
    // }

    // Frame stats
    Uint32 frames = 0;
    // Uint32 start = SDL_GetTicks();
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


    SpriteSheet sword = {};
    sword.sheet = IMG_Load("data/sprites/sword.png");

    // Hero
    Entity hero = {};
    hero.sprite_sheet.load("sprites/link_walking.png", 11, 5);
    hero.sprite_rect.w = hero.sprite_sheet.sprite_width;
    hero.sprite_rect.h = hero.sprite_sheet.sprite_height;
    hero.speed = 10;
    hero.direction = DOWN;
    hero.dest_rect.x = 85;
    hero.dest_rect.y = 85;
    hero.dest_rect.w = hero.sprite_sheet.sprite_width;
    hero.dest_rect.h = hero.sprite_sheet.sprite_height;

    SDL_Rect hero_starting_pos = hero.dest_rect;
    Point hero_collision_pt;
    bool hero_is_moving = false;
    bool in_quicksand = false;
    Uint32 next_frame_delay = SDL_GetTicks();
    Uint32 next_club_swing_delay = SDL_GetTicks();
    bool swing_club = false;
    SDL_Rect club_rect;
    Uint32 club_swing_timeout = SDL_GetTicks();

    // Enemy
    Entity harlod = {};
    harlod.sprite_sheet.load("sprites/Harlod_the_caveman.png", 1, 1);
    harlod.sprite_rect.w = harlod.sprite_sheet.sprite_width;
    harlod.sprite_rect.h = harlod.sprite_sheet.sprite_height;
    harlod.speed = 10;
    harlod.dest_rect.x = 150;
    harlod.dest_rect.y = 150;
    harlod.dest_rect.w = harlod.sprite_sheet.sprite_width;
    harlod.dest_rect.h = harlod.sprite_sheet.sprite_height;
    harlod.direction = DOWN;

    Entity buffalo = {};
    buffalo.sprite_sheet.load("sprites/Buffalo.png", 4, 1);
    buffalo.sprite_rect.w = buffalo.sprite_sheet.sprite_width;
    buffalo.sprite_rect.h = buffalo.sprite_sheet.sprite_height;
    buffalo.speed = 3;
    buffalo.dest_rect.x = 400;
    buffalo.dest_rect.y = 400;
    buffalo.dest_rect.w = buffalo.sprite_sheet.sprite_width;
    buffalo.dest_rect.h = buffalo.sprite_sheet.sprite_height;

    bool right_is_pressed = false;
    bool left_is_pressed = false;
    bool up_is_pressed = false;
    bool down_is_pressed = false;

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    // Colors
    Uint32 green = SDL_MapRGB(window_surface->format, 0, 255, 0);
    Uint32 blue = SDL_MapRGB(window_surface->format, 0, 0, 255);
    // Uint32 yellow = SDL_MapRGB(window_surface->format, 235, 245, 65);
    Uint32 brown = SDL_MapRGB(window_surface->format, 153, 102, 0);
    Uint32 rust = SDL_MapRGB(window_surface->format, 153, 70, 77);
    Uint32 magenta = SDL_MapRGB(window_surface->format, 255, 0, 255);
    Uint32 black = SDL_MapRGB(window_surface->format, 0, 0, 0);
    Uint32 red = SDL_MapRGB(window_surface->format, 255, 0, 0);
    Uint32 grey = SDL_MapRGB(window_surface->format, 135, 135, 135);

    // Tiles
    int tile_width = 80;
    int tile_height = 80;

    SDL_Rect current_tile;
    current_tile.x = hero.dest_rect.x / tile_width;
    current_tile.y = hero.dest_rect.y / tile_height;
    current_tile.w = tile_width;
    current_tile.h = tile_height;


    Tile w;
    w.flags = Tile::SOLID;
    w.color = green;
    w.sprite = NULL;

    Tile f;
    f.flags = Tile::NONE;
    f.color = blue;
    f.sprite = NULL;

    Tile m;
    m.flags = Tile::QUICKSAND;
    m.color = brown;
    m.sprite = NULL;

    Tile wr;
    wr.flags = Tile::WARP;
    wr.color = rust;
    wr.sprite = NULL;

    Tile t;
    t.flags = Tile::SOLID;
    t.color = green;
    t.sprite = IMG_Load("sprites/TropicalTree.png");
    t.sprite_rect = {};
    t.sprite_rect.w = 64;
    t.sprite_rect.h = 64;

    Tile fire;
    fire.flags = Tile::FIRE;
    fire.color = grey;
    fire.sprite = IMG_Load("sprites/Campfire.png");
    fire.sprite_rect = {};
    fire.sprite_rect.w = 64;
    fire.sprite_rect.h = 64;
    fire.animation = {};
    fire.animation.total_frames = 11;
    fire.animation.delay = 100;

    if (t.sprite == NULL) {
        printf("Couldn't load TropcialTree.png: %s\n", SDL_GetError());
        exit(1);
    }

    // Map
    const int map_cols = 12;
    const int map_rows = 10;

    Tile map[map_rows][map_cols] = {
        {w, w, w, w, w, w, w, w, w, w, w, w},
        {w, f, f, t, f, f, f, f, f, f, f, f},
        {w, f, f, t, f, f, f, fire, f, f, t, f},
        {w, f, f, f, f, f, f, f, f, f, f, m},
        {w, f, f, t, f, f, f, f, f, f, f, f},
        {w, f, f, t, t, t, t, f, f, t, f, wr},
        {w, f, f, f, f, f, t, f, f, t, f, f},
        {w, f, f, f, f, f, t, f, f, t, f, m},
        {w, f, f, f, f, f, f, f, f, t, f, f},
        {w, w, w, w, w, w, w, w, w, w, w, w}
    };

    bool in_map1 = true;

    Tile (*current_map)[map_rows][map_cols] = &map;

    Tile map2[map_rows][map_cols] = {
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

    Uint32 last_frame_duration = 0;

    // Main loop
    while(running) {
        Uint32 now = SDL_GetTicks();
        // if (SDL_GetTicks() > start + 1000) {
        //     printf("FPS: %d\n", frames);
        //     frames = 0;
        //     start = SDL_GetTicks();
        // }

        // Input
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYUP: {
                SDL_Scancode key = event.key.keysym.scancode;

                if (key == SDL_SCANCODE_ESCAPE) {
                    running = false;
                }
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L ||
                    key == SDL_SCANCODE_D) {
                    right_is_pressed = false;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W) {
                    up_is_pressed = false;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S) {
                    down_is_pressed = false;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A) {
                    left_is_pressed = false;
                }
                if (key == SDL_SCANCODE_F) {
                    swing_club = true;
                }
                break;
            }
            case SDL_QUIT: {
                running = false;
                break;
            }
            case SDL_KEYDOWN: {
                SDL_Scancode key = event.key.keysym.scancode;
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L ||
                    key == SDL_SCANCODE_D) {
                    right_is_pressed = true;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A) {
                    left_is_pressed = true;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W) {
                    up_is_pressed = true;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S) {
                    down_is_pressed = true;
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                // get vector from center of player to mouse cursor
                Point hero_center;
                hero_center.x = hero.dest_rect.x + (0.5f * hero.dest_rect.w);
                hero_center.y = hero.dest_rect.y + (0.5f * hero.dest_rect.h);
                Vec2 mouse_relative_to_hero;
                mouse_relative_to_hero.x = hero_center.x - ((float)event.motion.x + camera.x);
                mouse_relative_to_hero.y = hero_center.y - ((float)event.motion.y + camera.y);

                float angle = 0;
                if (mouse_relative_to_hero.x != 0 && mouse_relative_to_hero.y != 0) {
                    angle = atan2f(mouse_relative_to_hero.y, mouse_relative_to_hero.x) + PI;
                }

                // Get direction. One piece of a circle split in 8 sections
                // The radians start at 2*PI on (1, 0) and go to zero counter-clockwise
                float direction_increment = (2.0f * PI) / 8.0f;
                float half_increment = 0.5f * direction_increment;
                Direction old_direction = hero.direction;

                if (angle >= (3.0f * PI) / 2.0f - half_increment &&
                    angle < (3.0f * PI) / 2.0f + half_increment) {
                    hero.direction = UP;
                }
                else if (angle >= (3.0f * PI) / 2.0f + half_increment &&
                         angle < 2.0f * PI - half_increment) {
                    hero.direction = UP_RIGHT;
                }
                else if (angle >= 2.0f * PI - half_increment ||
                         angle < half_increment) {
                    hero.direction = RIGHT;
                }
                else if (angle >= half_increment &&
                         angle < PI / 2.0f - half_increment) {
                    hero.direction = DOWN_RIGHT;
                }
                else if (angle >= PI / 2.0f - half_increment &&
                         angle < PI / 2.0f + half_increment) {
                    hero.direction = DOWN;
                }
                else if (angle >= PI / 2.0f + half_increment &&
                         angle < PI - half_increment) {
                    hero.direction = DOWN_LEFT;
                }
                else if (angle >= PI - half_increment &&
                         angle < PI + half_increment) {
                    hero.direction = LEFT;
                }
                else if (angle >= PI + half_increment &&
                         angle < (3.0f * PI) / half_increment) {
                    hero.direction = UP_LEFT;
                }
                if (angle == 0) {
                    hero.direction = old_direction;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    swing_club = true;
                }
                break;
            }
            }
        }

        // Update
        SDL_Rect saved_position = hero.dest_rect;
        SDL_Rect saved_camera = camera;
        SDL_Rect saved_tile = current_tile;

        if (right_is_pressed) {
            hero.dest_rect.x += hero.speed;
            // hero.sprite_rect.y = 2 * hero.sprite_sheet.sprite_height;
            // hero.sprite_rect.x = hero.current_frame * hero.sprite_sheet.sprite_width;
            // if (now > next_frame_delay + 125) {
                // hero.current_frame++;
                // next_frame_delay = now;
            // }
            // if (hero.current_frame > hero.num_x_sprites - 1) {
                // hero.current_frame = 0;
            // }

            if (hero.dest_rect.x > x_pixel_movement_threshold &&
                camera.x < max_camera_x) {
                camera.x += hero.speed;
            }
        }
        if (left_is_pressed) {
            hero.dest_rect.x -= hero.speed;
            // hero.sprite_rect.y = 1 * hero.sprite_sheet.sprite_height;
            // hero.sprite_rect.x = hero.current_frame * hero.sprite_sheet.sprite_width;

            // if (now > next_frame_delay + 125) {
            //     hero.current_frame++;
            //     next_frame_delay = now;
            // }
            // if (hero.current_frame > hero.num_x_sprites - 1) {
            //     hero.current_frame = 0;
            // }

            if (hero.dest_rect.x <
                map_width_pixels - x_pixel_movement_threshold &&
                camera.x > 0) {
                camera.x -= hero.speed;
            }
        }
        if (up_is_pressed) {
            if (left_is_pressed || right_is_pressed) {
                hero.dest_rect.y -= 7;
            }
            else {
                hero.dest_rect.y -= hero.speed;
            }
            // hero.sprite_rect.y = 3 * hero.sprite_sheet.sprite_height;
            // hero.sprite_rect.x = hero.current_frame * hero.sprite_sheet.sprite_height;
            // if (now > next_frame_delay + 125) {
            //     hero.current_frame++;
            //     next_frame_delay = now;
            // }
            // if (hero.current_frame > hero.num_x_sprites - 1) {
            //     hero.current_frame = 0;
            // }

            if (hero.dest_rect.y <
                map_height_pixels - y_pixel_movement_threshold &&
                camera.y > 0) {
                camera.y -= hero.speed;
            }
        }
        if (down_is_pressed) {
            if (left_is_pressed || right_is_pressed) {
                hero.dest_rect.y += 7;
            }
            else {
                hero.dest_rect.y += hero.speed;
            }
            // hero.sprite_rect.y = 0 * hero.sprite_sheet.sprite_height;
            // hero.sprite_rect.x = hero.current_frame * hero.sprite_sheet.sprite_width;
            // if (now > next_frame_delay + 125) {
            //     hero.current_frame++;
            //     next_frame_delay = now;
            // }

            // if (hero.current_frame > hero.num_x_sprites - 1) {
            //     hero.current_frame = 0;
            // }

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
            hero.animation.current_frame = 0;
            hero.sprite_rect.x = 0;
        }

        // Handle club
        club_rect.x = hero.dest_rect.x + hero.dest_rect.w / 2;
        club_rect.y = hero.dest_rect.y + hero.dest_rect.h / 2;

        switch(hero.direction) {
        case DOWN:
            club_rect.w = 8;
            club_rect.x -= 4;
            club_rect.h = 32;
            club_rect.y += 16;
            break;
        case LEFT:
            club_rect.w = 32;
            club_rect.h = 8;
            club_rect.y += 16;
            club_rect.x -= 32;
            break;
        case RIGHT:
            club_rect.y += 16;
            club_rect.w = 32;
            club_rect.h = 8;
            break;
        case UP:
            club_rect.x -= 4;
            club_rect.y -= 32;
            club_rect.w = 8;
            club_rect.h = 32;
            break;
        }

        if (swing_club && now > next_club_swing_delay + 500) {
            next_club_swing_delay = now;
            club_swing_timeout = now + 500;
        }
        else {
            swing_club = false;
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
        Tile* tile_at_hero_position_ptr = &(*current_map)[map_coord_x][map_coord_y];
        // Handle all tiles
        if (tile_at_hero_position_ptr->is_solid()) {
            // Collisions. Reverse original state
            camera = saved_camera;
            hero.dest_rect = saved_position;
            current_tile = saved_tile;
        }
        if (tile_at_hero_position_ptr->is_slow() && !in_quicksand) {
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
        if (tile_at_hero_position_ptr->is_warp()) {
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

                Tile* tp = &(*current_map)[row][col];
                // TODO: Move this out of the draw section to the update section
                tp->animation.update(last_frame_duration);
                tp->draw(map_surface, &tile_rect);
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

        // Set sprite based on hero.direction
        switch (hero.direction) {
        case UP:
            hero.sprite_rect.x = 0;
            hero.sprite_rect.y = hero.sprite_sheet.sprite_height;
            break;
        case UP_RIGHT:
            hero.sprite_rect.x = 8 * hero.sprite_sheet.sprite_width;
            hero.sprite_rect.y = 0;
            break;
        case RIGHT:
            hero.sprite_rect.x = 0;
            hero.sprite_rect.y = 0;
            break;
        case DOWN_RIGHT:
            hero.sprite_rect.x = 8 * hero.sprite_sheet.sprite_width;
            hero.sprite_rect.y = 4 * hero.sprite_sheet.sprite_height;
            break;
        case DOWN:
            hero.sprite_rect.x = 0;
            hero.sprite_rect.y = 4 * hero.sprite_sheet.sprite_height;
            break;
        case DOWN_LEFT:
            hero.sprite_rect.x = 8 * hero.sprite_sheet.sprite_width;
            hero.sprite_rect.y = 3 * hero.sprite_sheet.sprite_height;
            break;
        case LEFT:
            hero.sprite_rect.x = 0;
            hero.sprite_rect.y = 3 * hero.sprite_sheet.sprite_height;
            break;
        case UP_LEFT:
            hero.sprite_rect.x = 8 * hero.sprite_sheet.sprite_width;
            hero.sprite_rect.y = hero.sprite_sheet.sprite_height;
            break;
        default:
            break;
        }
        // Draw sprites on map
        SDL_BlitSurface(hero.sprite_sheet.sheet, &hero.sprite_rect, map_surface, &hero.dest_rect);
        // SDL_RenderCopy(renderer, hero.sprite_texture, &hero.sprite_rect, &hero.dest_rect);
        // if (!in_map1) {
        SDL_BlitSurface(
            harlod.sprite_sheet.sheet,
            &harlod.sprite_rect,
            map_surface,
            &harlod.dest_rect
        );
        // }
        SDL_BlitSurface(
            buffalo.sprite_sheet.sheet,
            &buffalo.sprite_rect,
            map_surface,
            &buffalo.dest_rect
        );
        // Check Harlod/club collisions
        if (overlaps(&harlod.bounding_box, &club_rect) && now < club_swing_timeout) {
            SDL_FillRect(map_surface, &harlod.bounding_box, red);
        }

        // Draw hero club
        if (now < club_swing_timeout) {
            SDL_FillRect(map_surface, &club_rect, black);
        }
        // Draw portion of map visible to camera on the screen
        SDL_BlitSurface(map_surface, &camera, window_surface, NULL);

        SDL_UpdateWindowSurface(window);
        // SDL_RenderPresent(renderer);

        SDL_Delay(33);
        frames++;
        fflush(stdout);
        last_frame_duration = SDL_GetTicks() - now;
    }

    // Cleanup
    SDL_FreeSurface(map_surface);
    Mix_FreeChunk(mud_sound);
    Mix_Quit();
    IMG_Quit();
    // SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
