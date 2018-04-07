#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "gamedev_math.h"
#include "sprite_sheet.cpp"
#include "entity.cpp"
#include "tile_map.cpp"

bool overlaps(SDL_Rect* r1, SDL_Rect* r2)
{
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
}

struct Game
{
    static const int screen_width = 640;
    static const int screen_height = 480;

    Uint32 frames = 0;
    bool running = true;
    SDL_Window* window;
    SDL_Surface* window_surface;

    Game();
    ~Game();
    void init();
    void create_window();
};

Game::Game(): frames(0), running(true)
{}

Game::~Game()
{
    SDL_DestroyWindow(window);
}

void Game::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL failed to initialize: %s\n", SDL_GetError());
        exit(1);
    }
    IMG_Init(IMG_INIT_PNG);
}

void Game::create_window()
{
    window = SDL_CreateWindow("gamedev",
                              30,
                              50,
                              screen_width,
                              screen_height,
                              SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        exit(1);
    }
    window_surface = SDL_GetWindowSurface(window);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    Game game;
    game.init();
    game.create_window();

    // Sound
    Mix_Chunk* mud_sound = NULL;
    bool mud_sound_playing = false;
    Uint32 mud_sound_delay = SDL_GetTicks();

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        exit(1);
    }

    mud_sound = Mix_LoadWAV("sounds/mud_walk.wav");
    if (mud_sound == NULL)
    {
        printf("Mix_LoadWAV error: %s\n", Mix_GetError());
        return 1;
    }

    // SpriteSheet sword = {};
    // sword.load("sprites/sword.png", 12, 4);

    // Hero
    Entity hero("sprites/link_walking.png", 11, 5, 10, 85, 85, 6, 5, 12, 7, true);

    Point hero_collision_pt;
    bool hero_is_moving = false;
    bool in_quicksand = false;
    // Uint32 next_frame_delay = SDL_GetTicks();
    Uint32 next_club_swing_delay = SDL_GetTicks();
    bool swing_club = false;
    SDL_Rect club_rect;
    Uint32 club_swing_timeout = SDL_GetTicks();

    // Enemy
    Entity harlod("sprites/Harlod_the_caveman.png", 1, 1, 10, 150, 150, 0, 0, 0, 0, true);
    Entity buffalo("sprites/Buffalo.png", 4, 1, 3, 400, 400, 0, 0, 0, 0, true);

    bool right_is_pressed = false;
    bool left_is_pressed = false;
    bool up_is_pressed = false;
    bool down_is_pressed = false;

    // Colors
    SDL_PixelFormat* window_pixel_format = game.window_surface->format;
    Uint32 green = SDL_MapRGB(window_pixel_format, 0, 255, 0);
    Uint32 blue = SDL_MapRGB(window_pixel_format, 0, 0, 255);
    // Uint32 yellow = SDL_MapRGB(window_pixel_format, 235, 245, 65);
    Uint32 brown = SDL_MapRGB(window_pixel_format, 153, 102, 0);
    Uint32 rust = SDL_MapRGB(window_pixel_format, 153, 70, 77);
    Uint32 magenta = SDL_MapRGB(window_pixel_format, 255, 0, 255);
    Uint32 black = SDL_MapRGB(window_pixel_format, 0, 0, 0);
    Uint32 red = SDL_MapRGB(window_pixel_format, 255, 0, 0);
    Uint32 grey = SDL_MapRGB(window_pixel_format, 135, 135, 135);

    // Tiles
    SDL_Rect current_tile;
    current_tile.x = hero.dest_rect.x / Tile::tile_width;
    current_tile.y = hero.dest_rect.y / Tile::tile_height;
    current_tile.w = Tile::tile_width;
    current_tile.h = Tile::tile_height;

    Tile w(Tile::SOLID, green);
    Tile f(Tile::NONE, blue);
    Tile m(Tile::QUICKSAND, brown);
    Tile wr(Tile::WARP, rust);

    Tile t(Tile::SOLID, green, "sprites/TropicalTree.png");
    t.set_sprite_size(64, 64);
    t.active = true;

    Tile fire(Tile::FIRE, grey, "sprites/Campfire.png");
    fire.set_sprite_size(64, 64);
    fire.animation = {};
    fire.animation.total_frames = 11;
    fire.animation.delay = 100;
    fire.active = true;

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

    int map_width_pixels = map_cols * Tile::tile_width;
    int map_height_pixels = map_rows * Tile::tile_height;

    // Add 1 to each to account for displaying half a tile.
    int tile_rows_per_screen = (Game::screen_height / Tile::tile_height) + 1;
    int tile_cols_per_screen = (Game::screen_width / Tile::tile_width) + 1;

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
    camera.w = Game::screen_width;
    camera.h = Game::screen_height;

    SDL_Rect camera_starting_pos = camera;

    int max_camera_x = map_width_pixels - camera.w;
    int max_camera_y = map_height_pixels - camera.h;

    int y_pixel_movement_threshold = Game::screen_height / 2;
    int x_pixel_movement_threshold = Game::screen_width / 2;

    Uint32 last_frame_duration = 0;

    // Main loop
    while(game.running)
    {
        Uint32 now = SDL_GetTicks();
        // if (SDL_GetTicks() > start + 1000) {
        //     printf("FPS: %d\n", frames);
        //     frames = 0;
        //     start = SDL_GetTicks();
        // }

        // Input
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYUP:
            {
                SDL_Scancode key = event.key.keysym.scancode;

                if (key == SDL_SCANCODE_ESCAPE)
                {
                    game.running = false;
                }
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L ||
                    key == SDL_SCANCODE_D)
                {
                    right_is_pressed = false;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W)
                {
                    up_is_pressed = false;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S)
                {
                    down_is_pressed = false;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A)
                {
                    left_is_pressed = false;
                }
                if (key == SDL_SCANCODE_F)
                {
                    swing_club = true;
                }
                break;
            }
            case SDL_QUIT:
            {
                game.running = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                SDL_Scancode key = event.key.keysym.scancode;
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L ||
                    key == SDL_SCANCODE_D)
                {
                    right_is_pressed = true;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A)
                {
                    left_is_pressed = true;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W)
                {
                    up_is_pressed = true;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S)
                {
                    down_is_pressed = true;
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                // get vector from center of player to mouse cursor
                Point hero_center(hero.dest_rect.x + (0.5f * hero.dest_rect.w),
                                  hero.dest_rect.y + (0.5f * hero.dest_rect.h));
                Vec2 mouse_relative_to_hero;
                mouse_relative_to_hero.x = hero_center.x - ((float)event.motion.x + camera.x);
                mouse_relative_to_hero.y = hero_center.y - ((float)event.motion.y + camera.y);

                float angle = 0;
                if (mouse_relative_to_hero.x != 0 && mouse_relative_to_hero.y != 0)
                {
                    angle = atan2f(mouse_relative_to_hero.y, mouse_relative_to_hero.x) + PI;
                }

                // Get direction. One piece of a circle split in 8 sections
                // The radians start at 2*PI on (1, 0) and go to zero counter-clockwise
                float direction_increment = (2.0f * PI) / 8.0f;
                float half_increment = 0.5f * direction_increment;
                Direction old_direction = hero.direction;

                if (angle >= (3.0f * PI) / 2.0f - half_increment &&
                    angle < (3.0f * PI) / 2.0f + half_increment)
                {
                    hero.direction = UP;
                }
                else if (angle >= (3.0f * PI) / 2.0f + half_increment &&
                         angle < 2.0f * PI - half_increment)
                {
                    hero.direction = UP_RIGHT;
                }
                else if (angle >= 2.0f * PI - half_increment ||
                         angle < half_increment)
                {
                    hero.direction = RIGHT;
                }
                else if (angle >= half_increment &&
                         angle < PI / 2.0f - half_increment)
                {
                    hero.direction = DOWN_RIGHT;
                }
                else if (angle >= PI / 2.0f - half_increment &&
                         angle < PI / 2.0f + half_increment)
                {
                    hero.direction = DOWN;
                }
                else if (angle >= PI / 2.0f + half_increment &&
                         angle < PI - half_increment)
                {
                    hero.direction = DOWN_LEFT;
                }
                else if (angle >= PI - half_increment &&
                         angle < PI + half_increment)
                {
                    hero.direction = LEFT;
                }
                else if (angle >= PI + half_increment &&
                         angle < (3.0f * PI) / half_increment)
                {
                    hero.direction = UP_LEFT;
                }
                if (angle == 0)
                {
                    hero.direction = old_direction;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
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

        if (right_is_pressed)
        {
            hero.dest_rect.x += hero.speed;
            // hero.sprite_rect.y = 2 * hero.sprite_sheet.sprite_height;
            // hero.sprite_rect.x = hero.current_frame * hero.sprite_sheet.sprite_width;
            // if (now > next_frame_delay + 125)
            // {
                // hero.current_frame++;
                // next_frame_delay = now;
            // }
            // if (hero.current_frame > hero.num_x_sprites - 1)
            // {
                // hero.current_frame = 0;
            // }

            if (hero.dest_rect.x > x_pixel_movement_threshold &&
                camera.x < max_camera_x)
            {
                camera.x += hero.speed;
            }
        }
        if (left_is_pressed)
        {
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
                camera.x > 0)
            {
                camera.x -= hero.speed;
            }
        }
        if (up_is_pressed)
        {
            if (left_is_pressed || right_is_pressed)
            {
                hero.dest_rect.y -= 7;
            }
            else
            {
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
                camera.y > 0)
            {
                camera.y -= hero.speed;
            }
        }
        if (down_is_pressed) {
            if (left_is_pressed || right_is_pressed)
            {
                hero.dest_rect.y += 7;
            }
            else
            {
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
                camera.y < max_camera_y)
            {
                camera.y += hero.speed;
            }
        }

        if (saved_position.x != hero.dest_rect.x ||
            saved_position.y != hero.dest_rect.y)
        {
            hero_is_moving = true;
        }
        else
        {
            hero_is_moving = false;
        }

        if (!hero_is_moving)
        {
            hero.animation.current_frame = 0;
            hero.sprite_rect.x = 0;
        }

        // Handle club
        club_rect.x = hero.dest_rect.x + hero.dest_rect.w / 2;
        club_rect.y = hero.dest_rect.y + hero.dest_rect.h / 2;

        switch(hero.direction)
        {
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

        if (swing_club && now > next_club_swing_delay + 500)
        {
            next_club_swing_delay = now;
            club_swing_timeout = now + 500;
        }
        else
        {
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

        hero.update();
        harlod.update();
        buffalo.update();

        int map_coord_x = current_tile.y / Tile::tile_height;
        int map_coord_y = current_tile.x / Tile::tile_width;
        Tile* tile_at_hero_position_ptr = &(*current_map)[map_coord_x][map_coord_y];

        // Handle all tiles
        if (tile_at_hero_position_ptr->is_solid())
        {
            // Collisions. Revert to original state
            camera = saved_camera;
            hero.dest_rect = saved_position;
            current_tile = saved_tile;
        }
        if (tile_at_hero_position_ptr->is_slow() && !in_quicksand)
        {
            hero.speed -= 8;
            in_quicksand = true;
            if (SDL_GetTicks() > mud_sound_delay + 250 && hero_is_moving)
            {
                mud_sound_playing = true;
                Mix_PlayChannel(-1, mud_sound, 0);
                mud_sound_delay = SDL_GetTicks();
            }
        }
        else if (in_quicksand)
        {
            hero.speed += 8;
            in_quicksand = false;
        }
        if (tile_at_hero_position_ptr->is_warp())
        {
            if (in_map1)
            {
                current_map = &map2;
                in_map1 = false;
                fire.active = false;
                buffalo.active = false;
            }
            else
            {
                current_map = &map;
                in_map1 = true;
                fire.active = true;
                buffalo.active = true;
            }
            hero.dest_rect.x = (int)hero.starting_pos.x;
            hero.dest_rect.y = (int)hero.starting_pos.y;
            camera = camera_starting_pos;
        }

        // Center camera over the hero
        // camera.x = hero.dest_rect.x + hero.dest_rect.w / 2;
        // camera.y = hero.dest_rect.y + hero.dest_rect.h / 2;

        // Get tile under camera x,y
        int camera_tile_row = camera.y / Tile::tile_height;
        int camera_tile_col = camera.x / Tile::tile_width;

        SDL_Rect tile_rect;
        tile_rect.w = Tile::tile_width;
        tile_rect.h = Tile::tile_height;

        // Draw
        for (int row = camera_tile_row;
             row < tile_rows_per_screen + camera_tile_row;
             ++row)
        {
            for (int col = camera_tile_col;
                 col < tile_cols_per_screen + camera_tile_col;
                 ++col)
            {
                tile_rect.x = col * Tile::tile_width;
                tile_rect.y = row * Tile::tile_height;

                Tile* tp = &(*current_map)[row][col];
                // TODO: Move this out of the draw section to the update section
                tp->animation.update(last_frame_duration);
                tp->draw(map_surface, &tile_rect);
            }
        }

        // Highlight tile under player
        // SDL_FillRect(map_surface, &current_tile, yellow);
        // SDL_SetRenderDrawColor(renderer, 235, 245, 65, 255);
        // SDL_RenderFillRect(renderer, &current_tile);

        // Check hero/harlod collisions
        if (overlaps(&hero.bounding_box, &harlod.bounding_box))
        {
            // Draw overlapping bounding boxes
            SDL_Rect overlap_box;
            if (hero.bounding_box.x > harlod.bounding_box.x)
            {
                overlap_box.x = hero.bounding_box.x;
                overlap_box.w = harlod.bounding_box.x + harlod.bounding_box.w -
                    hero.bounding_box.x;
                overlap_box.w = min(overlap_box.w, hero.bounding_box.w);
            }
            else
            {
                overlap_box.x = harlod.bounding_box.x;
                overlap_box.w = hero.bounding_box.x + hero.bounding_box.w -
                    harlod.bounding_box.x;
                overlap_box.w = min(overlap_box.w, harlod.bounding_box.w);
            }

            if (hero.bounding_box.y > harlod.bounding_box.y)
            {
                overlap_box.y = hero.bounding_box.y;
                overlap_box.h = harlod.bounding_box.y + harlod.bounding_box.h -
                    hero.bounding_box.y;
                overlap_box.h = min(overlap_box.h, hero.bounding_box.h);
            }
            else
            {
                overlap_box.y = harlod.bounding_box.y;
                overlap_box.h = hero.bounding_box.y + hero.bounding_box.h -
                    harlod.bounding_box.y;
                overlap_box.h = min(overlap_box.h, harlod.bounding_box.h);
            }
            SDL_FillRect(map_surface, &overlap_box, magenta);

            // do pixel collision
        }

        // Set sprite based on hero.direction
        switch (hero.direction)
        {
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
        hero.draw(map_surface);
        harlod.draw(map_surface);
        buffalo.draw(map_surface);

        // Check Harlod/club collisions
        if (overlaps(&harlod.bounding_box, &club_rect) && now < club_swing_timeout)
        {
            SDL_FillRect(map_surface, &harlod.bounding_box, red);
        }

        // Draw hero club
        if (now < club_swing_timeout)
        {
            SDL_FillRect(map_surface, &club_rect, black);
        }

        // Draw portion of map visible to camera on the screen
        SDL_BlitSurface(map_surface, &camera, game.window_surface, NULL);

        SDL_UpdateWindowSurface(game.window);

        SDL_Delay(33);
        game.frames++;
        fflush(stdout);
        last_frame_duration = SDL_GetTicks() - now;
    }

    // Cleanup
    SDL_FreeSurface(map_surface);
    map_surface = NULL;
    Mix_FreeChunk(mud_sound);
    mud_sound = NULL;
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
