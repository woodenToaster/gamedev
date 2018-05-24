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

// TODO: Get forward declarations out of here.
struct Tile;
u8 tile_is_solid(Tile* t);
u8 tile_is_slow(Tile* t);
u8 tile_is_warp(Tile* t);

struct Plan;

struct Game;
void map_do_warp(Game* game);

#include "gamedev_animation.cpp"

#include "gamedev_math.h"
#include "gamedev_font.h"
#include "gamedev_sound.h"
#include "gamedev_globals.h"
#include "gamedev_input.h"
#include "gamedev_camera.cpp"
#include "gamedev_game.h"
#include "gamedev_sprite_sheet.h"
#include "gamedev_plan.h"
#include "gamedev_tilemap.h"
#include "gamedev_entity.h"
#include "gamedev_camera.h"

#include "gamedev_font.cpp"
#include "gamedev_sound.cpp"
#include "gamedev_globals.h"
#include "gamedev_asset_loading.cpp"
#include "gamedev_input.cpp"
#include "gamedev_game.cpp"
#include "gamedev_sprite_sheet.cpp"
#include "gamedev_plan.cpp"
#include "gamedev_entity.cpp"
#include "gamedev_tilemap.cpp"


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
    global_sounds[MUD_SOUND] = &mud_sound;

    SoundList sounds_to_play = {};

    Hero hero = {};
    entity_init_sprite_sheet(&hero.e, "sprites/dude.png", 4, 4);
    entity_set_starting_pos(&hero.e, 85, 85);
    entity_set_bounding_box_offset(&hero.e, 6, 5, 12, 7);
    entity_init_dest(&hero.e);
    hero.e.speed = 10;
    hero.e.active = GD_TRUE;
    hero.e.type = ET_HERO;
    hero.e.collision_pt_offset = 10;
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
    // TODO: :/
    Plan plan1 = {};
    Plan plan2 = {};
    Plan plan3 = {};
    Entity buffalo = create_buffalo(400, 200, &plan1);
    Entity buffalo2 = create_buffalo(500, 500, &plan2);
    Entity buffalo3 = create_buffalo(600, 100, &plan3);

    EntityList entity_list = {};
    Entity* _entities[] = {&hero.e, &harlod, &buffalo, &buffalo2, &buffalo3};
    entity_list.entities = _entities;
    entity_list.count = 5;

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
    wr.destination_map = 2;

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
    fire.has_animation = GD_TRUE;

    TileList tile_list = {};
    Tile* _tiles[] = {&w, &f, &m, &wr, &t, &fire};
    tile_list.tiles = _tiles;
    tile_list.count = 6;

    // Tileset
    Tileset tiles = {};
    tiles.surface = create_surface_from_png(&tiles.img_data, "sprites/jungle_tileset.png");

    Tile* grass = &tiles.tiles[0];
    grass->tile_width = 16;
    grass->tile_height = 16;
    grass->flags = tile_properties[TP_NONE];
    grass->color = game.colors[GREEN];
    grass->sprite = tiles.surface;
    grass->sprite_rect = {16, 16, 16, 16};

    Tile grass_warp = {};
    grass_warp.tile_width = grass_warp.tile_height = 16;
    tile_init(&grass_warp, tile_properties[TP_WARP], game.colors[RUST]);
    // TODO: Make destination work
    grass_warp.destination_map = 1;

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

    Tile* map3_tiles[3000];
    for (int i = 0; i < 3000; ++i)
    {
        map3_tiles[i] = grass;
    }

    Map map1 = {};
    map_init(&map1, 12, 10, map1_tiles);
    map1.current = GD_TRUE;
    Entity* map1_entities[] = {&hero.e, &harlod};
    map1.active_entities.entities = map1_entities;
    map1.active_entities.count = 2;
    Tile* map1_active_tiles[] = {&fire};
    map1.active_tiles.tiles = map1_active_tiles;
    map1.active_tiles.count = 1;

    Map map2 = {};
    map_init(&map2, 12, 10, map2_tiles);
    Entity* map2_entities[] = {&hero.e, &harlod, &buffalo, &buffalo2, &buffalo3,};
    map2.active_entities.entities = map2_entities;
    map2.active_entities.count = 5;

    Map map3 = {};
    map_init(&map3, 60, 50, map3_tiles);
    Entity* map3_entities[] = {&hero.e, &harlod};
    map3.active_entities.entities = map3_entities;
    map3.active_entities.count = 2;
    Tile* map3_active_tiles[] = {grass};
    map3.active_tiles.tiles = map3_active_tiles;
    map3.active_tiles.count = 1;

    // Draw warp out of 16 x 16 tiles
    for (int i = 4; i < 8; ++i)
    {
        int row = map3.cols * i + 200;
        map3_tiles[row] = &grass_warp;
        map3_tiles[row + 1] = &grass_warp;
        map3_tiles[row + 2] = &grass_warp;
        map3_tiles[row + 3] = &grass_warp;
    }

    MapList map_list = {};
    Map* _maps[] = {&map1, &map2, &map3};
    map_list.maps = _maps;
    map_list.count = 3;

    game.current_map = &map1;
    game_init_camera(&game);
    game.maps = &map_list;
    game.sounds = &sounds_to_play;

    u32 last_frame_duration = 0;

    /**************************************************************************/
    /* Main Loop                                                              */
    /**************************************************************************/
    while(game.running)
    {
        u32 now = SDL_GetTicks();
        f32 fps = (1.0f * 1000) / last_frame_duration;
        char a[4];
        snprintf(a, 4, "%d\n", (u32)fps);
        ttf_font_create_bitmap(&ttf_tens, a[0]);
        ttf_font_create_bitmap(&ttf_ones, a[1]);

        /*********************************************************************/
        /* Input                                                             */
        /*********************************************************************/
        input_poll(&input, &game);

        /*********************************************************************/
        /* Update                                                            */
        /*********************************************************************/
        game_update(&game, &input);
        map_update_tiles(game.current_map, last_frame_duration);
        hero_update(&hero, &input, &game);
        hero_update_club(&hero, now);
        camera_update(&game.camera);
        entity_list_update(&entity_list, game.current_map, last_frame_duration);
        animation_update(&hero.e.animation, last_frame_duration, hero.is_moving);
        ttf_font_update_pos(&ttf_tens, game.camera.viewport.x, game.camera.viewport.y);
        ttf_font_update_pos(&ttf_ones, game.camera.viewport.x + ((int)font_size / 2),
                            game.camera.viewport.y);
        sound_play_all(game.sounds, now);

        /*********************************************************************/
        /* Draw                                                              */
        /*********************************************************************/
        map_draw(&game);

        // Only for drawing overlap boxes
        hero_check_collisions_with_entities(&hero, &game);
        hero_draw_club(&hero, now, &game);
        SDL_BlitSurface(ttf_tens.surface, NULL, game.current_map->surface, &ttf_tens.dest);
        SDL_BlitSurface(ttf_ones.surface, NULL, game.current_map->surface, &ttf_ones.dest);
        SDL_BlitSurface(game.current_map->surface, &game.camera.viewport,
                        game.window_surface, NULL);
        SDL_UpdateWindowSurface(game.window);

        SDL_Delay(26);
        game.frames++;
        fflush(stdout);
        last_frame_duration = SDL_GetTicks() - now;
    }

    /**************************************************************************/
    /* Cleanup                                                                */
    /**************************************************************************/
    ttf_font_destroy(&ttf_tens);
    ttf_font_destroy(&ttf_ones);
    tile_list_destroy(&tile_list);
    tileset_destroy(&tiles);
    map_list_destroy(&map_list);
    entity_list_destroy(&entity_list);
    game_destroy(&game);

    return 0;
}
