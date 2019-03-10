#define _CRT_SECURE_NO_WARNINGS

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// #define SDL_MAIN_HANDLED

#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_mixer.h"

// TODO(chj): Remove standard library dependency
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "gamedev_definitions.h"
#include "gamedev_forward_declarations.h"
#include "gamedev_animation.cpp"

#include "gamedev_plan.h"
#include "gamedev_math.h"
#include "gamedev_font.h"
#include "gamedev_sound.h"
#include "gamedev_globals.h"
#include "gamedev_input.h"
#include "gamedev_camera.cpp"
#include "gamedev_game.h"
#include "gamedev_sprite_sheet.h"
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
#include "gamedev_memory.cpp"

#define aalloc(type) ((type*)arena_push(&arena, sizeof(type)))
#define arraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

void harlodInteractWithHero(Entity *e, Entity *h, Game *g)
{
    (void)h;

    if (!e->dialogFile.contents)
    {
        e->dialogFile = readEntireFile("dialogs/harlod_dialogs.txt");
        // Remove the EOF character
        --e->dialogFile.size;
        // TODO(chj): Need to null terminate everything. This will change. We
        // want to parse files for strings and tokenize, etc. For testing it's
        // hard coded
        e->dialogFile.contents[9] = '\0';
        // TODO(chj): Free dialog.contents
    }
    startDialogMode(g, (char*)e->dialogFile.contents);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Arena arena = {};
    arena_init(&arena, (size_t)MEGABYTES(1));

    u32 screenWidth = 640;
    u32 screenHeight = 480;

    // Game
    Game* game = aalloc(Game);
    initGame(game, screenWidth, screenHeight);

    // OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext ogl_context = SDL_GL_CreateContext(game->window);

    if (ogl_context == NULL)
    {
        fprintf(stderr, "Failed to create opengl context: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture *transparentBlackTexture = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGBA8888,
                                                             SDL_TEXTUREACCESS_TARGET, screenWidth, screenHeight);
    SDL_SetTextureBlendMode(transparentBlackTexture, SDL_BLENDMODE_BLEND);
    // TODO(chj): free

    // Input
    Input input = {};
    initControllers(&input);

    // Font
    FontMetadata fontMetadata = {};
    generateFontData(&fontMetadata, game);

	// Sound
    Sound mudSound = {};
    mudSound.delay = 250;
    mudSound.chunk = sound_load_wav("sounds/mud_walk.wav");
    global_sounds[SOUND_MUD] = &mudSound;

    SoundList sounds_to_play = {};

    Hero hero = {};
    entity_init_sprite_sheet(&hero.e, "sprites/dude.png", 4, 4, game->renderer);
    entity_set_starting_pos(&hero.e, 85, 85);
    hero.e.position.x = (f32)hero.e.starting_pos.x;
    hero.e.position.y = (f32)hero.e.starting_pos.y;
    entity_set_bounding_box_offset(&hero.e, 6, 5, 12, 7);
    entity_init_dest(&hero.e);
    hero.speed = 2000; // m/s^2
    // TODO(chj): Replace entity speed with f32 and use that for hero
    hero.e.speed = 10;
    hero.e.active = GD_TRUE;
    hero.e.type = ET_HERO;
    hero.e.collision_pt_offset = 10;
    animation_init(&hero.e.animation, 4, 100);
    for (int i = 0; i < INV_COUNT; ++i)
    {
        hero.inventory[i] = 0;
    }

    // Harlod
    Harlod harlod = {};
    entity_init_sprite_sheet(&harlod.e, "sprites/Harlod_the_caveman.png", 1, 1, game->renderer);
    entity_set_starting_pos(&harlod.e, 150, 150);
    entity_set_bounding_box_offset(&harlod.e, 0, 0, 0, 0);
    entity_init_dest(&harlod.e);
    harlod.e.speed = 10;
    harlod.e.type = ET_HARLOD;
    harlod.onHeroInteract = &harlodInteractWithHero;
    harlod.e.active = GD_TRUE;

    // Buffalo
    Entity buffalo = create_buffalo(400, 200, game->renderer);
    Entity buffalo2 = create_buffalo(500, 500, game->renderer);
    Entity buffalo3 = create_buffalo(600, 100, game->renderer);

    EntityList entity_list = {};
    Entity* _entities[] = {&hero.e, &harlod.e, &buffalo, &buffalo2, &buffalo3};
    entity_list.entities = _entities;
    entity_list.count = 5;

    // Tiles
    Tile w = {};
    w.tile_width = w.tile_height = 80;
    initTile(&w, tile_properties[TP_SOLID], game->colors[COLOR_GREEN], game->renderer);

    Tile f = {};
    f.tile_width = f.tile_height = 80;
    initTile(&f, tile_properties[TP_NONE], game->colors[COLOR_BLUE], game->renderer);

    Tile m = {};
    m.tile_width = m.tile_height = 80;
    initTile(&m, tile_properties[TP_QUICKSAND], game->colors[COLOR_BROWN], game->renderer);

    Tile wr = {};
    wr.tile_width = wr.tile_height = 80;
    initTile(&wr, tile_properties[TP_WARP], game->colors[COLOR_RUST], game->renderer);
    wr.destination_map = 2;

    Tile t = {};
    t.tile_width = t.tile_height = 80;
    initTile(&t, tile_properties[TP_SOLID], game->colors[COLOR_GREEN], game->renderer, "sprites/TropicalTree.png");
    tile_set_sprite_size(&t, 64, 64);
    t.active = GD_TRUE;

    Tile fire = {};
    fire.tile_width = fire.tile_height = 80;
    initTile(&fire, tile_properties[TP_FIRE], game->colors[COLOR_GREY], game->renderer, "sprites/Campfire.png");
    tile_set_sprite_size(&fire, 64, 64);
    animation_init(&fire.animation, 11, 100);
    fire.active = GD_TRUE;
    fire.has_animation = GD_TRUE;

    // Harvestable tree
    Tile h_tree = {};
    h_tree.tile_width = h_tree.tile_height = 80;
    initTile(&h_tree, tile_properties[TP_HARVEST] | tile_properties[TP_SOLID],
              game->colors[COLOR_NONE], game->renderer, "sprites/tree.png");
    tile_set_sprite_size(&h_tree, 64, 64);
    h_tree.active = GD_TRUE;
    h_tree.is_harvestable = GD_TRUE;
    h_tree.harvestedItem = INV_LEAVES;

    Tile h_tree1 = {};
    h_tree1.tile_width = h_tree1.tile_height = 80;
    initTile(&h_tree1, tile_properties[TP_HARVEST] | tile_properties[TP_SOLID],
              game->colors[COLOR_NONE], game->renderer, "sprites/tree.png");
    tile_set_sprite_size(&h_tree1, 64, 64);
    h_tree1.active = GD_TRUE;
    h_tree1.is_harvestable = GD_TRUE;
    h_tree1.harvestedItem = INV_LEAVES;

    TileList tile_list = {};
    Tile* _tiles[] = {&w, &f, &m, &wr, &t, &fire, &h_tree, &h_tree1};
    tile_list.tiles = _tiles;

    tile_list.count = arraySize(tile_list.tiles);

    // Tileset
    Tileset jungle_tiles = {};
    SDL_Surface* jungle_tiles_surface = create_surface_from_png(&jungle_tiles.img_data, "sprites/jungle_tileset.png");
    jungle_tiles.texture = SDL_CreateTextureFromSurface(game->renderer, jungle_tiles_surface);

    Tile* grass = &jungle_tiles.tiles[0];
    grass->tile_width = 16;
    grass->tile_height = 16;
    grass->flags = tile_properties[TP_NONE];
    grass->color = game->colors[COLOR_NONE];
    grass->sprite = jungle_tiles.texture;
    grass->sprite_rect = {16, 16, 16, 16};

    Tile grass_warp = {};
    grass_warp.tile_width = grass_warp.tile_height = 16;
    initTile(&grass_warp, tile_properties[TP_WARP], game->colors[COLOR_RUST], game->renderer);
    grass_warp.destination_map = 1;

    // Map
    Tile* map1_tiles[] = {
        &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w, &w,
        &w, &f, &f, &h_tree1, &f, &f, &f, &f, &f, &f, &f, &f,
        &w, &f, &f, &t, &f, &f, &f, &fire, &f, &f, &t, &f,
        &w, &f, &f, &f, &f, &f, &f, &f, &f, &f, &f, &m,
        &w, &f, &f, &h_tree, &f, &f, &f, &f, &f, &f, &f, &f,
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
    initMap(&map1, 12, 10, map1_tiles, game->renderer);
    map1.current = GD_TRUE;
    Entity* map1_entities[] = {&hero.e, &harlod.e};
    map1.active_entities.entities = map1_entities;
    map1.active_entities.count = 2;
    Tile* map1_active_tiles[] = {&fire};
    map1.active_tiles.tiles = map1_active_tiles;
    map1.active_tiles.count = 1;

    Map map2 = {};
    initMap(&map2, 12, 10, map2_tiles, game->renderer);
    Entity* map2_entities[] = {&hero.e, &harlod.e, &buffalo, &buffalo2, &buffalo3,};
    map2.active_entities.entities = map2_entities;
    map2.active_entities.count = 5;

    Map map3 = {};
    initMap(&map3, 60, 50, map3_tiles, game->renderer);
    Entity* map3_entities[] = {&hero.e, &harlod.e};
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
    map_list.count = arraySize(map_list.maps);

    game->current_map = &map1;
    initCamera(game);
    game->maps = &map_list;
    game->sounds = &sounds_to_play;

    /**************************************************************************/
    /* Main Loop                                                              */
    /**************************************************************************/
    while(game->running)
    {
        u32 now = SDL_GetTicks();

        /*********************************************************************/
        /* Input                                                             */
        /*********************************************************************/
        pollInput(&input, game);

        /*********************************************************************/
        /* Update                                                            */
        /*********************************************************************/
        if (game->mode == GAME_MODE_PLAYING)
        {
            updateGame(game, &input);
            map_update_tiles(game);
            updateHero(&hero, &input, game);
            hero_update_club(&hero, now);
            updateCamera(&game->camera, &hero.e.dest_rect);
            entity_list_update(&entity_list, game->current_map, game->dt);
            updateAnimation(&hero.e.animation, game->dt, hero.is_moving);
            sound_play_all(game->sounds, now);
        }
        else if (game->mode == GAME_MODE_DIALOG)
        {
            updateDialogMode(game, &input);
        }
        else if (game->mode == GAME_MODE_INVENTORY)
        {
            updateInventoryMode(game, &input);
        }

        /*********************************************************************/
        /* Draw                                                              */
        /*********************************************************************/
        SDL_SetRenderTarget(game->renderer, game->current_map->texture);
        drawMap(game);

        if (game->mode == GAME_MODE_DIALOG)
        {
            darkenBackground(game);
            drawDialogScreen(game, &fontMetadata);
        }

        if (game->mode == GAME_MODE_INVENTORY)
        {
            darkenBackground(game);
            drawInventoryScreen(game, &hero, &fontMetadata);
        }

        // Only for drawing overlap boxes. Move to update section once real
        // collisions are being handled.
        for (size_t i = 0; i < game->current_map->active_entities.count; ++i)
        {
            Entity* e = game->current_map->active_entities.entities[i];
            entity_check_collisions_with_entities(e, game);
        }

        // hero_draw_club(&hero, now, game);

        // Draw FPS
        // f32 fps = 1000.0f / game->dt;
        // char fps_str[9] = {0};
        // snprintf(fps_str, 9, "FPS: %03d", (u32)fps);
        // drawText(game, &fontMetadata, fps_str, game->camera.viewport.x, game->camera.viewport.y);

        // char v[30];
        // snprintf(v, 30, "v: {%.6f, %.6f}", input.stickX, input.stickY);
        // drawText(game, &fontMetadata, v, game->camera.viewport.x, game->camera.viewport.y + 24);
        // char p[30];
        // snprintf(p, 30, "p: {%.6f, %.6f}", hero.e.position.x, hero.e.position.y);
        // drawText(game, &fontMetadata, p, game->camera.viewport.x, game->camera.viewport.y + 48);

        SDL_SetRenderTarget(game->renderer, NULL);
        SDL_RenderCopy(game->renderer, game->current_map->texture, &game->camera.viewport, NULL);

        game->dt = SDL_GetTicks() - now;
        game_fix_frame_rate(game);

        SDL_RenderPresent(game->renderer);
        fflush(stdout);
    }

    /**************************************************************************/
    /* Cleanup                                                                */
    /**************************************************************************/
    destroyFontMetadata(&fontMetadata);
    destroyTileList(&tile_list);
    destroyTileset(&jungle_tiles);
    map_list_destroy(&map_list);
    entity_list_destroy(&entity_list);
    destroyControllers(&input);
    game_destroy(game);
    arena_destroy(&arena);

    return 0;
}
