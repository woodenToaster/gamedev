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
#include "gamedev_entity.cpp"
#include "gamedev_tilemap.cpp"

#include "gamedev_input.h"

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
    Hero hero = {};
    entity_init_sprite_sheet(&hero.e, "sprites/link_walking.png", 11, 5);
    entity_set_starting_pos(&hero.e, 85, 85);
    entity_set_bounding_box_offset(&hero.e, 6, 5, 12, 7);
    entity_init_dest(&hero.e);
    hero.e.speed = 10;
    hero.e.active = GD_TRUE;

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
    SDL_Rect current_tile;
    current_tile.x = hero.e.dest_rect.x / Tile::tile_width;
    current_tile.y = hero.e.dest_rect.y / Tile::tile_height;
    current_tile.w = Tile::tile_width;
    current_tile.h = Tile::tile_height;

    Tile w(Tile::SOLID, game.colors[GREEN]);
    Tile f(Tile::NONE, game.colors[BLUE]);
    Tile m(Tile::QUICKSAND, game.colors[BROWN]);
    Tile wr(Tile::WARP, game.colors[RUST]);

    Tile t(Tile::SOLID, game.colors[GREEN], "sprites/TropicalTree.png");
    t.set_sprite_size(64, 64);
    t.active = GD_TRUE;

    Tile fire(Tile::FIRE, game.colors[GREY], "sprites/Campfire.png");
    fire.set_sprite_size(64, 64);
    animation_init(&fire.animation, 11, 100);
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
    // int tile_rows_per_screen = (Game::screen_height / Tile::tile_height) + 1;
    // int tile_cols_per_screen = (Game::screen_width / Tile::tile_width) + 1;

    // Camera
    SDL_Rect camera;
    camera.x = 0;
    camera.y = 0;
    camera.w = game.screen_width;
    camera.h = game.screen_height;

    SDL_Rect camera_starting_pos = camera;

    int max_camera_x = map1.width_pixels - camera.w;
    int max_camera_y = map1.height_pixels - camera.h;

    int y_pixel_movement_threshold = game.screen_height / 2;
    int x_pixel_movement_threshold = game.screen_width / 2;

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

        if (input.is_pressed[KEY_RIGHT])
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
        if (input.is_pressed[KEY_LEFT])
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
        if (input.is_pressed[KEY_UP])
        {
            if (input.is_pressed[KEY_LEFT] || input.is_pressed[KEY_RIGHT])
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
        if (input.is_pressed[KEY_DOWN]) {
            if (input.is_pressed[KEY_LEFT] || input.is_pressed[KEY_RIGHT])
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
        entity_update(&buffalo);
        animation_update(&buffalo.animation, last_frame_duration);

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
            camera = camera_starting_pos;
        }

        // Center camera over the hero
        // camera.x = hero.e.dest_rect.x + hero.e.dest_rect.w / 2;
        // camera.y = hero.e.dest_rect.y + hero.e.dest_rect.h / 2;

        // Get tile under camera x,y
        // int camera_tile_row = camera.y / Tile::tile_height;
        // int camera_tile_col = camera.x / Tile::tile_width;

        ttf_font_update_pos(&ttf_tens, camera.x, camera.y);
        ttf_font_update_pos(&ttf_ones, camera.x + ((int)font_size / 2), camera.y);

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
            SDL_FillRect(current_map->surface, &overlap_box, game.colors[MAGENTA]);

            // do pixel collision
        }

        set_hero_sprite(&hero.e);

        // Check Harlod/club collisions
        if (overlaps(&harlod.bounding_box, &hero.club_rect) && now < hero.club_swing_timeout)
        {
            SDL_FillRect(current_map->surface, &harlod.bounding_box, game.colors[RED]);
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
        entity_draw(&buffalo, current_map->surface);

        // Draw hero club
        if (now < hero.club_swing_timeout)
        {
            SDL_FillRect(current_map->surface, &hero.club_rect, game.colors[BLACK]);
        }

        // Draw FPS
        SDL_BlitSurface(ttf_tens.surface, NULL, current_map->surface, &ttf_tens.dest);
        SDL_BlitSurface(ttf_ones.surface, NULL, current_map->surface, &ttf_ones.dest);

        // Draw map
        SDL_BlitSurface(current_map->surface, &camera, game.window_surface, NULL);

        SDL_UpdateWindowSurface(game.window);

        SDL_Delay(26);
        game.frames++;
        fflush(stdout);
        last_frame_duration = SDL_GetTicks() - now;
    }

    // Cleanup
    ttf_font_destroy(&ttf_tens);
    ttf_font_destroy(&ttf_ones);
    map_list_destroy(&map_list);
    entity_list_destroy(&entity_list);
    game_destroy(&game);
    return 0;
}
