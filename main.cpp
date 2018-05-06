#define _CRT_SECURE_NO_WARNINGS

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "gamedev_definitions.h"
#include "gamedev_math.h"
#include "gamedev_font.cpp"
#include "gamedev_sound.cpp"
#include "gamedev_asset_loading.cpp"
#include "gamedev_game.cpp"
#include "gamedev_sprite_sheet.cpp"
#include "gamedev_animation.cpp"
#include "gamedev_input.h"
#include "gamedev_entity.cpp"
#include "gamedev_tilemap.cpp"
#include "gamedev_camera.cpp"

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    // Game
    Game game = {};
    game.running = GD_TRUE;
    game_init(&game, 640, 480);

    // Input
    Input input = {};

    // Font
    TTFFile ttf_file = {};
    ttf_file.fname = "fonts/arialbd.ttf";

    TTFFont ttf_tens = {};
    float font_size = 32;
    ttf_font_init(&ttf_tens, &ttf_file, font_size);

    TTFFont ttf_ones = {};
    ttf_font_init(&ttf_ones, &ttf_file, font_size);

	// Sound
    Sound mud_sound = {};
    mud_sound.delay = 250;
    mud_sound.chunk = sound_load_wav("sounds/mud_walk.wav");

    // Hero
    // Hero hero = {};
    // entity_init_sprite_sheet(&hero.e, "sprites/link_walking.png", 11, 5);
    // entity_set_starting_pos(&hero.e, 85, 85);
    // entity_set_bounding_box_offset(&hero.e, 6, 5, 12, 7);
    // entity_init_dest(&hero.e);
    // hero.e.speed = 10;
    // hero.e.active = GD_TRUE;

    Hero hero = {};
    entity_init_sprite_sheet(&hero.e, "sprites/dude.png", 4, 4);
    entity_set_starting_pos(&hero.e, 85, 85);
    entity_set_bounding_box_offset(&hero.e, 6, 5, 12, 7);
    entity_init_dest(&hero.e);
    hero.e.speed = 10;
    hero.e.active = GD_TRUE;
    animation_init(&hero.e.animation, 4, 100);

    // Harlod
    Entity harlod = {};
    entity_init_sprite_sheet(&harlod, "sprites/Harlod_the_caveman.png", 1, 1);
    entity_set_starting_pos(&harlod, 150, 150);
    entity_set_bounding_box_offset(&harlod, 0, 0, 0, 0);
    entity_init_dest(&harlod);
    harlod.speed = 10;
    harlod.active = GD_TRUE;

    // Buffalo
    Entity buffalo = {};
    entity_init_sprite_sheet(&buffalo, "sprites/Buffalo.png", 4, 1);
    entity_set_starting_pos(&buffalo, 400, 200);
    entity_set_bounding_box_offset(&buffalo, 0, 0, 0, 0);
    entity_init_dest(&buffalo);
    buffalo.speed = 10;
    buffalo.active = GD_TRUE;
    animation_init(&buffalo.animation, 4, 100);

    EntityList entity_list = {};
    Entity* _entities[] = {&hero.e, &harlod, &buffalo};
    entity_list.entities = _entities;
    entity_list.count = 3;

    // Tiles
    Tile w = {};
    w.tile_width = w.tile_height = 80;
    tile_init(&w, tile_properties[TP_SOLID], game.colors[GREEN]);

    Tile f = {};
    f.tile_width = f.tile_height = 80;
    tile_init(&f, tile_properties[TP_NONE], game.colors[BLUE]);

    Tile m = {};
    m.tile_width = m.tile_height = 80;
    tile_init(&m, tile_properties[TP_QUICKSAND], game.colors[BROWN]);

    Tile wr = {};
    wr.tile_width = wr.tile_height = 80;
    tile_init(&wr, tile_properties[TP_WARP], game.colors[RUST]);

    Tile t = {};
    t.tile_width = t.tile_height = 80;
    tile_init(&t, tile_properties[TP_SOLID], game.colors[GREEN], "sprites/TropicalTree.png");
    tile_set_sprite_size(&t, 64, 64);
    t.active = GD_TRUE;

    Tile fire = {};
    fire.tile_width = fire.tile_height = 80;
    tile_init(&fire, tile_properties[TP_FIRE], game.colors[GREY], "sprites/Campfire.png");
    tile_set_sprite_size(&fire, 64, 64);
    animation_init(&fire.animation, 11, 100);
    fire.active = GD_TRUE;

    TileList tile_list = {};
    Tile* _tiles[] = {&w, &f, &m, &wr, &t, &fire};
    tile_list.tiles = _tiles;
    tile_list.count = 6;

    // Assumes every tile on every map is the same size
    int world_tile_width =  w.tile_width;
    int world_tile_height = w.tile_height;

    SDL_Rect current_tile;
    current_tile.x = hero.e.dest_rect.x / world_tile_width;
    current_tile.y = hero.e.dest_rect.y / world_tile_height;
    current_tile.w = w.tile_width;
    current_tile.h = w.tile_height;

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
    map_init(&map1, 12, 10, map1_tiles);
    map1.current = GD_TRUE;

    Map map2 = {};
    map_init(&map2, 12, 10, map2_tiles);

    MapList map_list = {};
    Map* _maps[] = {&map1, &map2};
    map_list.maps = _maps;
    map_list.count = 2;

    Map* current_map = &map1;

    // Add 1 to each to account for displaying half a tile.
    // int tile_rows_per_screen = (game.screen_height / world_tile_height) + 1;
    // int tile_cols_per_screen = (game.screen_width / world_tile_width) + 1;

    // Camera
    Camera camera = {};
    camera.viewport = {};
    camera.viewport.w = game.screen_width;
    camera.viewport.h = game.screen_height;
    camera.starting_pos = camera.viewport;
    camera.max_x = map1.width_pixels - camera.viewport.w;
    camera.max_y = map1.height_pixels - camera.viewport.h;
    camera.y_pixel_movement_threshold = game.screen_height / 2;
    camera.x_pixel_movement_threshold = game.screen_width / 2;

    Uint32 last_frame_duration = 0;

    // Main loop
    while(game.running)
    {
        u32 now = SDL_GetTicks();
        f32 fps = (1.0f * 1000) / last_frame_duration;
        char a[4];
        snprintf(a, 4, "%d\n", (u32)fps);
        ttf_font_create_bitmap(&ttf_tens, a[0]);
        ttf_font_create_bitmap(&ttf_ones, a[1]);

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
                    input.is_pressed[KEY_RIGHT] = GD_FALSE;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W)
                {
                    input.is_pressed[KEY_UP] = GD_FALSE;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S)
                {
                    input.is_pressed[KEY_DOWN] = GD_FALSE;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A)
                {
                    input.is_pressed[KEY_LEFT] = GD_FALSE;
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
                    input.is_pressed[KEY_RIGHT] = GD_TRUE;
                }
                if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_H ||
                    key == SDL_SCANCODE_A)
                {
                    input.is_pressed[KEY_LEFT]= GD_TRUE;
                }
                if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_K ||
                    key == SDL_SCANCODE_W)
                {
                    input.is_pressed[KEY_UP]= GD_TRUE;
                }
                if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_J ||
                    key == SDL_SCANCODE_S)
                {
                    input.is_pressed[KEY_DOWN]= GD_TRUE;
                }
                break;
            }
            // case SDL_MOUSEMOTION:
            // {
            //     // get vector from center of player to mouse cursor
            //     Point hero_center = {
            //         hero.e.dest_rect.x + (i32)(0.5 * hero.e.dest_rect.w),
            //         hero.e.dest_rect.y + (i32)(0.5 * hero.e.dest_rect.h)
            //     };
            //     Vec2 mouse_relative_to_hero;
            //     mouse_relative_to_hero.x = hero_center.x - ((float)event.motion.x + camera.x);
            //     mouse_relative_to_hero.y = hero_center.y - ((float)event.motion.y + camera.y);

            //     float angle = 0;
            //     if (mouse_relative_to_hero.x != 0 && mouse_relative_to_hero.y != 0)
            //     {
            //         angle = atan2f(mouse_relative_to_hero.y, mouse_relative_to_hero.x) + PI;
            //     }

            //     if (angle != 0)
            //     {
            //         hero.e.direction = get_direction_from_angle(angle);
            //     }
            //     break;
            // }
            // case SDL_MOUSEBUTTONUP:
            // {
            //     if (event.button.button == SDL_BUTTON_LEFT)
            //     {
            //         hero.swing_club = GD_TRUE;
            //     }
            //     break;
            // }
            }
        }

        // Update
        SDL_Rect saved_position = hero.e.dest_rect;
        SDL_Rect saved_camera = camera.viewport;
        SDL_Rect saved_tile = current_tile;

        map_update_tiles(current_map, last_frame_duration);
        hero_update(&hero, &input, &camera, current_map);
        hero_update_club(&hero, now);

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

        // Update camera
        // camera_center_over_point(&camera, &hero.e.dest_rect);
        camera.viewport.x = clamp(camera.viewport.x, 0, camera.max_x);
        camera.viewport.y = clamp(camera.viewport.y, 0, camera.max_y);

        hero_clamp_to_map(&hero, current_map);
        hero_set_collision_point(&hero);

        current_tile.x = (hero.collision_pt.x / world_tile_width) * world_tile_width;
        current_tile.y = (hero.collision_pt.y / world_tile_height) * world_tile_height;

        // Update entities
        entity_list_update(&entity_list);

        // Update animations
        animation_update(&buffalo.animation, last_frame_duration, GD_TRUE);
        animation_update(&hero.e.animation, last_frame_duration, hero.is_moving);

        int map_coord_x = current_tile.y / world_tile_height;
        int map_coord_y = current_tile.x / world_tile_width;
        int tile_index = map_coord_x * current_map->cols + map_coord_y;
        Tile* tile_at_hero_position_ptr = current_map->tiles[tile_index];

        // Handle all tiles
        if (tile_is_solid(tile_at_hero_position_ptr))
        {
            // Collisions. Revert to original state
            camera.viewport = saved_camera;
            hero.e.dest_rect = saved_position;
            current_tile = saved_tile;
        }
        if (tile_is_slow(tile_at_hero_position_ptr) && !hero.in_quicksand)
        {
            hero.e.speed -= 9;
            hero.in_quicksand = GD_TRUE;
            if (hero.is_moving)
            {
                sound_play(&mud_sound, now);
            }
        }
        else if (hero.in_quicksand)
        {
            hero.e.speed += 9;
            hero.in_quicksand = GD_FALSE;
        }
        if (tile_is_warp(tile_at_hero_position_ptr))
        {
            if (map1.current)
            {
                current_map = &map2;
                map1.current = GD_FALSE;
                map2.current = GD_TRUE;
                fire.active = GD_FALSE;
                buffalo.active = GD_FALSE;
            }
            else
            {
                current_map = &map1;
                map1.current = GD_TRUE;
                map2.current = GD_FALSE;
                fire.active = GD_TRUE;
                buffalo.active = GD_TRUE;
            }
            hero.e.dest_rect.x = (int)hero.e.starting_pos.x;
            hero.e.dest_rect.y = (int)hero.e.starting_pos.y;
            camera.viewport = camera.starting_pos;
        }

        ttf_font_update_pos(&ttf_tens, camera.viewport.x, camera.viewport.y);
        ttf_font_update_pos(&ttf_ones, camera.viewport.x + ((int)font_size / 2), camera.viewport.y);

        SDL_Rect tile_rect = {};
        tile_rect.w = world_tile_width;
        tile_rect.h = world_tile_height;

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
            SDL_FillRect(current_map->surface, &overlap_box, game.colors[MAGENTA]);

            // do pixel collision
        }

        // set_hero_sprite(&hero.e);

        // Check Harlod/club collisions
        if (overlaps(&harlod.bounding_box, &hero.club_rect) && now < hero.club_swing_timeout)
        {
            SDL_FillRect(current_map->surface, &harlod.bounding_box, game.colors[RED]);
        }

        // Draw
        // TODO: Don't redraw the whole map on every frame
        for (size_t row = 0; row < current_map->rows; ++row)
        {
            for (size_t col = 0; col < current_map->cols; ++col)
            {
                tile_rect.x = (int)col * world_tile_width;
                tile_rect.y = (int)row * world_tile_height;
                Tile* tp = current_map->tiles[row * current_map->cols + col];
                tile_draw(tp, current_map->surface, &tile_rect);
            }
        }

        // Highlight tile under player
        // SDL_FillRect(current_map->surface, &current_tile, game.colors[YELLOW]);

        // Draw sprites on map
        entity_list_draw(&entity_list, current_map->surface);

        // Draw hero club
        hero_draw_club(&hero, now, current_map->surface, game.colors[BLACK]);

        // Draw FPS
        SDL_BlitSurface(ttf_tens.surface, NULL, current_map->surface, &ttf_tens.dest);
        SDL_BlitSurface(ttf_ones.surface, NULL, current_map->surface, &ttf_ones.dest);

        // Draw map
        SDL_BlitSurface(current_map->surface, &camera.viewport, game.window_surface, NULL);

        SDL_UpdateWindowSurface(game.window);

        SDL_Delay(26);
        game.frames++;
        fflush(stdout);
        last_frame_duration = SDL_GetTicks() - now;
    }

    // Cleanup
    ttf_font_destroy(&ttf_tens);
    ttf_font_destroy(&ttf_ones);
    tile_list_destroy(&tile_list);
    map_list_destroy(&map_list);
    entity_list_destroy(&entity_list);
    game_destroy(&game);

    return 0;
}
