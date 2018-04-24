#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

enum
{
    GD_FALSE,
    GD_TRUE
};

enum Direction
{
    UP,
    UP_RIGHT,
    RIGHT,
    DOWN_RIGHT,
    DOWN,
    DOWN_LEFT,
    LEFT,
    UP_LEFT
};

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

    u32 frames = 0;
    bool running = GD_TRUE;
    u8 initialized = GD_FALSE;
    SDL_Window* window;
    SDL_Surface* window_surface;

    Game();
};

Game::Game(): frames(0), running(GD_TRUE)
{}

void game_destroy(Game* g)
{
    Mix_Quit();
    IMG_Quit();
    SDL_DestroyWindow(g->window);
    SDL_Quit();
}

void game_init(Game* g)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL failed to initialize: %s\n", SDL_GetError());
        exit(1);
    }
    IMG_Init(IMG_INIT_PNG);

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        exit(1);
    }

    g->window = SDL_CreateWindow("gamedev",
                                 30,
                                 50,
                                 g->screen_width,
                                 g->screen_height,
                                 SDL_WINDOW_SHOWN);

    if (g->window == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        exit(1);
    }
    g->window_surface = SDL_GetWindowSurface(g->window);
    g->initialized = GD_TRUE;
}

struct Sound
{
    u8 is_playing;
    u32 delay;
    u64 last_play_time;
    Mix_Chunk* chunk;
};

Mix_Chunk* sound_load_wav(const char* fname)
{
    Mix_Chunk * result = Mix_LoadWAV(fname);
    if (result == NULL)
    {
        printf("Mix_LoadWAV error: %s\n", Mix_GetError());
        return NULL;
    }
    return result;
}

void sound_play(Sound* s, u64 now)
{
    if (now > s->last_play_time + s->delay)
    {
        Mix_PlayChannel(-1, s->chunk, 0);
        s->last_play_time = SDL_GetTicks();
    }
}

void sound_destroy(Sound* s)
{
    Mix_FreeChunk(s->chunk);
}

struct Input
{
    // ...
};

struct TTFFile
{
    char buf[1 << 25];
    char* fname;
    u8 initialized;
};

void ttf_file_read(TTFFile* t)
{
    FILE* f = fopen(t->fname, "rb");
    fread(t->buf, 1, 1 << 25, f);
    t->initialized = GD_TRUE;
    fclose(f);
}

struct TTFFont
{
    stbtt_fontinfo font;
    char* text;
    f32 size;
    f32 scale;
    int width;
    int height;
    unsigned char* bitmap;
    SDL_Surface* surface;
    TTFFile* ttf_file;
};

void ttf_font_init(TTFFont* f, TTFFile* file, f32 size)
{
    if (!file->initialized)
    {
        ttf_file_read(file);
    }

    f->ttf_file = file;
    f->size = size;

    stbtt_InitFont(&f->font, (u8*)file->buf, stbtt_GetFontOffsetForIndex((u8*)file->buf, 0));
    f->scale = stbtt_ScaleForPixelHeight(&f->font, f->size);
}

void ttf_font_create_bitmap(TTFFont* f, int character)
{
    f->bitmap = stbtt_GetCodepointBitmap(&f->font, 0, f->scale, character,
                                         &f->width, &f->height, 0, 0);

	f->surface = SDL_CreateRGBSurfaceFrom(
        (void*)f->bitmap,
        f->width, f->height,
        8,
        f->width,
        0, 0, 0, 0
    );

    if (!f->surface)
    {
        printf("%s\n", SDL_GetError());
        exit(1);
    }

    SDL_Color grayscale[256];
    for(int i = 0; i < 256; i++){
        grayscale[i].r = (u8)i;
        grayscale[i].g = (u8)i;
        grayscale[i].b = (u8)i;
    }
    SDL_SetPaletteColors(f->surface->format->palette, grayscale, 0, 256);
    return f->surface;
}

void ttf_font_destroy(TTFFont* f)
{
    SDL_FreeSurface(f->surface);
    stbtt_FreeBitmap(f->bitmap, NULL);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    Game game;
    game_init(&game);

    if (!game.initialized)
    {
        fprintf(stderr, "Game was not initialized.\n");
        exit(1);
    }

    // Font
    TTFFile ttf_file = {};
    ttf_file.fname = "fonts/arialbd.ttf";

    TTFFont ttf_font = {};
    float font_size = 100;
    ttf_font_init(&ttf_font, &ttf_file, font_size);
    ttf_font_create_bitmap(&ttf_font, 'R');

    // SDL_Surface* letters[256];
    // for(int i = 'a'; i <= 'z'; i++)
    // {
    //     letters[i] = ttf_font_create_bitmap(&ttf_font, i);
    // }

	// Sound
    Sound mud_sound = {};
    mud_sound.delay = 250;
    mud_sound.chunk = sound_load_wav("sounds/mud_walk.wav");

    // Hero
    Hero hero = {};
    entity_init_sprite_sheet(&hero.e, "sprites/link_walking.png", 11, 5);
    entity_set_starting_pos(&hero.e, 85, 85);
    entity_set_bounding_box_offset(&hero.e, 6, 5, 12, 7);
    entity_init_dest(&hero.e);
    hero.e.speed = 10;
    hero.e.active = GD_TRUE;

    // Enemy
    Entity harlod = {};
    entity_init_sprite_sheet(&harlod, "sprites/Harlod_the_caveman.png", 1, 1);
    entity_set_starting_pos(&harlod, 150, 150);
    entity_set_bounding_box_offset(&harlod, 0, 0, 0, 0);
    entity_init_dest(&harlod);
    harlod.speed = 10;
    harlod.active = GD_TRUE;

    // Entity buffalo("sprites/Buffalo.png", 4, 1, 3, 400, 400, 0, 0, 0, 0, GD_TRUE);

    EntityList entity_list = {};
    Entity* _entities[] = {&hero.e, &harlod};
    entity_list.entities = _entities;
    entity_list.count = 2;

    bool right_is_pressed = GD_FALSE;
    bool left_is_pressed = GD_FALSE;
    bool up_is_pressed = GD_FALSE;
    bool down_is_pressed = GD_FALSE;

    // Colors
    SDL_PixelFormat* window_pixel_format = game.window_surface->format;
    Uint32 green = SDL_MapRGB(window_pixel_format, 37, 71, 0);
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
    current_tile.x = hero.e.dest_rect.x / Tile::tile_width;
    current_tile.y = hero.e.dest_rect.y / Tile::tile_height;
    current_tile.w = Tile::tile_width;
    current_tile.h = Tile::tile_height;

    Tile w(Tile::SOLID, green);
    Tile f(Tile::NONE, blue);
    Tile m(Tile::QUICKSAND, brown);
    Tile wr(Tile::WARP, rust);

    Tile t(Tile::SOLID, green, "sprites/TropicalTree.png");
    t.set_sprite_size(64, 64);
    t.active = GD_TRUE;

    Tile fire(Tile::FIRE, grey, "sprites/Campfire.png");
    fire.set_sprite_size(64, 64);
    fire.animation.init(11, 100);
    fire.active = GD_TRUE;

    // Map
    Tile* map1_tiles[] = {
        &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w,
        &w, &f, &f, &t, &f, &f, &f, &f, &f, &f, &f, &f,
        &w, &f, &f, &t, &f, &f, &f, &fire, &f, &f, &t, &f,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &m,
        &w, &f, &f, &t, &f, &f, &f, &f, &f, &f, &f, &f,
        &w, &f, &f, &t, &t, &t, &t, &f, &f, &t, &f, &wr,
        &w, &f, &f, &f, &f, &f, &t, &f, &f, &t, &f, &f,
        &w, &f, &f, &f, &f, &f, &t, &f, &f, &t, &f, &m,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &t, &f, &f,
        &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w
    };

    Tile* map2_tiles[] = {
        &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &w,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &w,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &w,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &w,
        &w, &f, &f, &f, &f, &wr, &f, &f, &f, &f, &f, &w,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &w,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &w,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &w,
        &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w
    };

    Map map1 = {};
    map1.cols = 12;
    map1.rows = 10;
    map1.tiles = map1_tiles;
    map1.current = GD_TRUE;
    map1.width_pixels = map1.cols * Tile::tile_width;
    map1.height_pixels = map1.rows * Tile::tile_height;
    map1.surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        map1.width_pixels,
        map1.height_pixels,
        32,
        SDL_PIXELFORMAT_RGB888
    );


    Map map2 = {};
    map2.cols = 12;
    map2.rows = 10;
    map2.tiles = map2_tiles;
    map2.width_pixels = map2.cols * Tile::tile_width;
    map2.height_pixels = map2.rows * Tile::tile_height;
    map2.surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        map2.width_pixels,
        map2.height_pixels,
        32,
        SDL_PIXELFORMAT_RGB888
    );

    MapList map_list = {};
    Map* _maps[] = {&map1, &map2};
    map_list.maps = _maps;
    map_list.count = 2;

    Map* current_map = &map1;

    // Add 1 to each to account for displaying half a tile.
    // int tile_rows_per_screen = (Game::screen_height / Tile::tile_height) + 1;
    // int tile_cols_per_screen = (Game::screen_width / Tile::tile_width) + 1;

    // Camera
    SDL_Rect camera;
    camera.x = 0;
    camera.y = 0;
    camera.w = Game::screen_width;
    camera.h = Game::screen_height;

    SDL_Rect camera_starting_pos = camera;

    int max_camera_x = map1.width_pixels - camera.w;
    int max_camera_y = map1.height_pixels - camera.h;

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
                    game.running = GD_FALSE;
                }
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L ||
                    key == SDL_SCANCODE_D)
                {
                    right_is_pressed = GD_FALSE;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W)
                {
                    up_is_pressed = GD_FALSE;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S)
                {
                    down_is_pressed = GD_FALSE;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A)
                {
                    left_is_pressed = GD_FALSE;
                }
                if (key == SDL_SCANCODE_F)
                {
                    hero.swing_club = GD_TRUE;
                }
                break;
            }
            case SDL_QUIT:
            {
                game.running = GD_FALSE;
                break;
            }
            case SDL_KEYDOWN:
            {
                SDL_Scancode key = event.key.keysym.scancode;
                if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_L ||
                    key == SDL_SCANCODE_D)
                {
                    right_is_pressed = GD_TRUE;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A)
                {
                    left_is_pressed = GD_TRUE;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W)
                {
                    up_is_pressed = GD_TRUE;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S)
                {
                    down_is_pressed = GD_TRUE;
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                // get vector from center of player to mouse cursor
                Point hero_center = {
                    hero.e.dest_rect.x + (i32)(0.5 * hero.e.dest_rect.w),
                    hero.e.dest_rect.y + (i32)(0.5 * hero.e.dest_rect.h)
                };
                Vec2 mouse_relative_to_hero;
                mouse_relative_to_hero.x = hero_center.x - ((float)event.motion.x + camera.x);
                mouse_relative_to_hero.y = hero_center.y - ((float)event.motion.y + camera.y);

                float angle = 0;
                if (mouse_relative_to_hero.x != 0 && mouse_relative_to_hero.y != 0)
                {
                    angle = atan2f(mouse_relative_to_hero.y, mouse_relative_to_hero.x) + PI;
                }

                if (angle != 0)
                {
                    hero.e.direction = get_direction_from_angle(angle);
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    hero.swing_club = GD_TRUE;
                }
                break;
            }
            }
        }

        // Update
        SDL_Rect saved_position = hero.e.dest_rect;
        SDL_Rect saved_camera = camera;
        SDL_Rect saved_tile = current_tile;

        map_update_tiles(current_map, last_frame_duration);

        if (right_is_pressed)
        {
            hero.e.dest_rect.x += hero.e.speed;
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

            if (hero.e.dest_rect.x > x_pixel_movement_threshold &&
                camera.x < max_camera_x)
            {
                camera.x += hero.e.speed;
            }
        }
        if (left_is_pressed)
        {
            hero.e.dest_rect.x -= hero.e.speed;
            // hero.sprite_rect.y = 1 * hero.sprite_sheet.sprite_height;
            // hero.sprite_rect.x = hero.current_frame * hero.sprite_sheet.sprite_width;

            // if (now > next_frame_delay + 125) {
            //     hero.current_frame++;
            //     next_frame_delay = now;
            // }
            // if (hero.current_frame > hero.num_x_sprites - 1) {
            //     hero.current_frame = 0;
            // }

            if (hero.e.dest_rect.x <
                map1.width_pixels - x_pixel_movement_threshold &&
                camera.x > 0)
            {
                camera.x -= hero.e.speed;
            }
        }
        if (up_is_pressed)
        {
            if (left_is_pressed || right_is_pressed)
            {
                hero.e.dest_rect.y -= 7;
            }
            else
            {
                hero.e.dest_rect.y -= hero.e.speed;
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

            if (hero.e.dest_rect.y <
                map1.height_pixels - y_pixel_movement_threshold &&
                camera.y > 0)
            {
                camera.y -= hero.e.speed;
            }
        }
        if (down_is_pressed) {
            if (left_is_pressed || right_is_pressed)
            {
                hero.e.dest_rect.y += 7;
            }
            else
            {
                hero.e.dest_rect.y += hero.e.speed;
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

            if (hero.e.dest_rect.y > y_pixel_movement_threshold &&
                camera.y < max_camera_y)
            {
                camera.y += hero.e.speed;
            }
        }

        if (saved_position.x != hero.e.dest_rect.x ||
            saved_position.y != hero.e.dest_rect.y)
        {
            hero.is_moving = GD_TRUE;
        }
        else
        {
            hero.is_moving = GD_FALSE;
        }

        if (!hero.is_moving)
        {
            hero.e.animation.current_frame = 0;
            hero.e.sprite_rect.x = 0;
        }

        // Handle club
        hero.club_rect.x = hero.e.dest_rect.x + hero.e.dest_rect.w / 2;
        hero.club_rect.y = hero.e.dest_rect.y + hero.e.dest_rect.h / 2;

        switch(hero.e.direction)
        {
        case DOWN:
            hero.club_rect.w = 8;
            hero.club_rect.x -= 4;
            hero.club_rect.h = 32;
            hero.club_rect.y += 16;
            break;
        case LEFT:
            hero.club_rect.w = 32;
            hero.club_rect.h = 8;
            hero.club_rect.y += 16;
            hero.club_rect.x -= 32;
            break;
        case RIGHT:
            hero.club_rect.y += 16;
            hero.club_rect.w = 32;
            hero.club_rect.h = 8;
            break;
        case UP:
            hero.club_rect.x -= 4;
            hero.club_rect.y -= 32;
            hero.club_rect.w = 8;
            hero.club_rect.h = 32;
            break;
        }

        if (hero.swing_club && now > hero.next_club_swing_delay + 500)
        {
            hero.next_club_swing_delay = now;
            hero.club_swing_timeout = now + 500;
        }
        else
        {
            hero.swing_club = GD_FALSE;
        }

        // Clamp camera
        camera.x = clamp(camera.x, 0, max_camera_x);
        camera.y = clamp(camera.y, 0, max_camera_y);

        // Clamp hero
        hero.e.dest_rect.x = clamp(hero.e.dest_rect.x, 0, map1.width_pixels - hero.e.dest_rect.w);
        hero.e.dest_rect.y = clamp(hero.e.dest_rect.y, 0, map1.height_pixels - hero.e.dest_rect.h);

        hero.collision_pt.y = hero.e.dest_rect.y + hero.e.dest_rect.h - 10;
        hero.collision_pt.x = hero.e.dest_rect.x + (i32)(hero.e.dest_rect.w / 2.0);

        current_tile.x = (hero.collision_pt.x / 80) * 80;
        current_tile.y = (hero.collision_pt.y / 80) * 80;

        entity_update(&hero.e);
        entity_update(&harlod);
        // entity_update(&buffalo);

        int map_coord_x = current_tile.y / Tile::tile_height;
        int map_coord_y = current_tile.x / Tile::tile_width;
        int tile_index = map_coord_x * current_map->cols + map_coord_y;
        Tile* tile_at_hero_position_ptr = current_map->tiles[tile_index];

        // Handle all tiles
        if (tile_at_hero_position_ptr->is_solid())
        {
            // Collisions. Revert to original state
            camera = saved_camera;
            hero.e.dest_rect = saved_position;
            current_tile = saved_tile;
        }
        if (tile_at_hero_position_ptr->is_slow() && !hero.in_quicksand)
        {
            hero.e.speed -= 8;
            hero.in_quicksand = GD_TRUE;
            if (hero.is_moving)
            {
                sound_play(&mud_sound, now);
            }
        }
        else if (hero.in_quicksand)
        {
            hero.e.speed += 8;
            hero.in_quicksand = GD_FALSE;
        }
        if (tile_at_hero_position_ptr->is_warp())
        {
            if (map1.current)
            {
                current_map = &map2;
                map1.current = GD_FALSE;
                map2.current = GD_TRUE;
                fire.active = GD_FALSE;
                // buffalo.active = GD_FALSE;
            }
            else
            {
                current_map = &map1;
                map1.current = GD_TRUE;
                map2.current = GD_FALSE;
                fire.active = GD_TRUE;
                // buffalo.active = GD_TRUE;
            }
            hero.e.dest_rect.x = (int)hero.e.starting_pos.x;
            hero.e.dest_rect.y = (int)hero.e.starting_pos.y;
            camera = camera_starting_pos;
        }

        // Center camera over the hero
        // camera.x = hero.e.dest_rect.x + hero.e.dest_rect.w / 2;
        // camera.y = hero.e.dest_rect.y + hero.e.dest_rect.h / 2;

        // Get tile under camera x,y
        // int camera_tile_row = camera.y / Tile::tile_height;
        // int camera_tile_col = camera.x / Tile::tile_width;

        SDL_Rect tile_rect;
        tile_rect.w = Tile::tile_width;
        tile_rect.h = Tile::tile_height;


        // Highlight tile under player
        // SDL_FillRect(current_map->surface, &current_tile, yellow);
        // SDL_SetRenderDrawColor(renderer, 235, 245, 65, 255);
        // SDL_RenderFillRect(renderer, &current_tile);

        // Check hero/harlod collisions
        if (overlaps(&hero.e.bounding_box, &harlod.bounding_box))
        {
            // Draw overlapping bounding boxes
            SDL_Rect overlap_box;
            if (hero.e.bounding_box.x > harlod.bounding_box.x)
            {
                overlap_box.x = hero.e.bounding_box.x;
                overlap_box.w = harlod.bounding_box.x + harlod.bounding_box.w -
                    hero.e.bounding_box.x;
                overlap_box.w = min(overlap_box.w, hero.e.bounding_box.w);
            }
            else
            {
                overlap_box.x = harlod.bounding_box.x;
                overlap_box.w = hero.e.bounding_box.x + hero.e.bounding_box.w -
                    harlod.bounding_box.x;
                overlap_box.w = min(overlap_box.w, harlod.bounding_box.w);
            }

            if (hero.e.bounding_box.y > harlod.bounding_box.y)
            {
                overlap_box.y = hero.e.bounding_box.y;
                overlap_box.h = harlod.bounding_box.y + harlod.bounding_box.h -
                    hero.e.bounding_box.y;
                overlap_box.h = min(overlap_box.h, hero.e.bounding_box.h);
            }
            else
            {
                overlap_box.y = harlod.bounding_box.y;
                overlap_box.h = hero.e.bounding_box.y + hero.e.bounding_box.h -
                    harlod.bounding_box.y;
                overlap_box.h = min(overlap_box.h, harlod.bounding_box.h);
            }
            SDL_FillRect(current_map->surface, &overlap_box, magenta);

            // do pixel collision
        }

        set_hero_sprite(&hero.e);

        // Check Harlod/club collisions
        if (overlaps(&harlod.bounding_box, &hero.club_rect) && now < hero.club_swing_timeout)
        {
            SDL_FillRect(current_map->surface, &harlod.bounding_box, red);
        }

        // Draw
        // TODO: Don't redraw the whole map on every frame
        for (size_t row = 0; row < current_map->rows; ++row)
            // for (int row = camera_tile_row;
            //      row < tile_rows_per_screen + camera_tile_row;
            //      ++row)
        {
            for (size_t col = 0; col < current_map->cols; ++col)
                // for (int col = camera_tile_col;
                //      col < tile_cols_per_screen + camera_tile_col;
                //      ++col)
            {
                tile_rect.x = (int)col * Tile::tile_width;
                tile_rect.y = (int)row * Tile::tile_height;
                Tile* tp = current_map->tiles[row * current_map->cols + col];
                tp->draw(current_map->surface, &tile_rect);
            }
        }

        // Draw sprites on map
        entity_draw(&hero.e, current_map->surface);
        entity_draw(&harlod, current_map->surface);
        // buffalo.draw(current_map->surface);

        // Draw hero club
        if (now < hero.club_swing_timeout)
        {
            SDL_FillRect(current_map->surface, &hero.club_rect, black);
        }

        // Draw map
        // SDL_Rect font_dest = {camera.x, camera.y, font_width * 3, font_height * 3};
        SDL_BlitSurface(ttf_font.surface, NULL, current_map->surface, NULL); //&font_dest);
        SDL_BlitSurface(current_map->surface, &camera, game.window_surface, NULL);

        SDL_UpdateWindowSurface(game.window);

        SDL_Delay(33);
        game.frames++;
        fflush(stdout);
        last_frame_duration = SDL_GetTicks() - now;
    }

    // Cleanup
    ttf_font_destroy(&ttf_font);
    map_list_destroy(&map_list);
    entity_list_destroy(&entity_list);
    game_destroy(&game);
    return 0;
}
