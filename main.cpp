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

#define PushStruct(arena, type) ((type*)pushSize((arena), sizeof(type)))
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Arena arena = {};
    initArena(&arena, (size_t)MEGABYTES(1));

    u32 screenWidth = 960;
    u32 screenHeight = 540;

    // Game
    Game* game = PushStruct(&arena, Game);
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

    SDL_Texture *transparentBlackTexture =
        SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_TARGET, screenWidth, screenHeight);
    SDL_SetTextureBlendMode(transparentBlackTexture, SDL_BLENDMODE_BLEND);

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
    initEntitySpriteSheet(&hero.e, "sprites/link_walking.png", 11, 5, game->renderer);
    hero.e.sprite_sheet.scale = 2;
    hero.e.width = 20;
    hero.e.height = 10;
    hero.e.spriteDims = {45, 60};
    initAnimation(&hero.e.animation, 8, 80);
    hero.e.starting_pos = {120, 120};
    hero.e.position = hero.e.starting_pos;
    hero.e.bb_x_offset = 10;
    hero.e.bb_y_offset = 18;
    hero.e.bounding_box = {0, 0, hero.e.width - 2 * hero.e.bb_x_offset, hero.e.height - hero.e.bb_y_offset};
    hero.e.speed = 2000;
    hero.e.active = true;
    hero.e.type = ET_HERO;
    hero.e.collision_pt_offset = 5;

    // SpriteSheet heroSword = {};
    // loadSpriteSheet(&heroSword, "sprites/sword.png", 12, 4, game->renderer);

    // Sprite heroSword = {};
    // heroSword.x = 188;
    // heroSword.y = 1;
    // heroSword.width = 16;
    // heroSword.height = 50;
    // heroSword.offsetX = 2;
    // heroSword.offsetY = -25;

    // Tiles
    // Tile w = {};
    // w.width = w.height = 80;
    // initTile(&w, tile_properties[TP_SOLID], game->colors[COLOR_GREEN], game->renderer);

    // Tile f = {};
    // f.width = f.height = 80;
    // initTile(&f, tile_properties[TP_NONE], game->colors[COLOR_BLUE], game->renderer);

    // Tile m = {};
    // m.width = m.height = 80;
    // initTile(&m, tile_properties[TP_QUICKSAND], game->colors[COLOR_BROWN], game->renderer);

    // Tile wr = {};
    // wr.width = wr.height = 80;
    // initTile(&wr, tile_properties[TP_WARP], game->colors[COLOR_RUST], game->renderer);
    // wr.destination_map = 2;

    // Tile t = {};
    // t.width = t.height = 80;
    // initTile(&t, tile_properties[TP_SOLID], game->colors[COLOR_GREEN], game->renderer, "sprites/TropicalTree.png");
    // setTileSpriteSize(&t, 64, 64);
    // t.active = true;

    // Tile fire = {};
    // fire.width = fire.height = 80;
    // initTile(&fire, tile_properties[TP_FIRE] | tile_properties[TP_INTERACTIVE],
    //          game->colors[COLOR_GREY], game->renderer, "sprites/Campfire.png");
    // setTileSpriteSize(&fire, 64, 64);
    // initAnimation(&fire.animation, 11, 100);
    // fire.active = true;
    // fire.has_animation = true;
    // fire.onHeroInteract = lightFire;

    // Harvestable tree
    // Tile h_tree = {};
    // h_tree.width = h_tree.height = 80;
    // initTile(&h_tree, tile_properties[TP_HARVEST] | tile_properties[TP_SOLID],
    //           game->colors[COLOR_NONE], game->renderer, "sprites/tree.png");
    // setTileSpriteSize(&h_tree, 64, 64);
    // h_tree.active = true;
    // h_tree.is_harvestable = true;
    // h_tree.harvestedItem = INV_LEAVES;

    // Tile h_tree1 = {};
    // h_tree1.width = h_tree1.height = 80;
    // initTile(&h_tree1, tile_properties[TP_HARVEST] | tile_properties[TP_SOLID],
    //           game->colors[COLOR_NONE], game->renderer, "sprites/tree.png");
    // setTileSpriteSize(&h_tree1, 64, 64);
    // h_tree1.active = true;
    // h_tree1.is_harvestable = true;
    // h_tree1.harvestedItem = INV_LEAVES;

    // TileList tile_list = {};
    // Tile* _tiles[] = {&w, &f, &m, &wr, &t, &fire, &h_tree, &h_tree1};
    // tile_list.tiles = _tiles;

    // tile_list.count = ArrayCount(tile_list.tiles);

    // Tileset
    // Tileset jungle_tiles = {};
    // jungle_tiles.texture = create_texture_from_png("sprites/jungle_tileset.png", game->renderer);

    // Tile* grass = &jungle_tiles.tiles[0];
    // grass->width = 16;
    // grass->height = 16;
    // grass->flags = tile_properties[TP_NONE];
    // grass->color = game->colors[COLOR_NONE];
    // grass->sprite = jungle_tiles.texture;
    // grass->sprite_rect = {16, 16, 16, 16};

    // Tile grass_warp = {};
    // grass_warp.width = grass_warp.height = 16;
    // initTile(&grass_warp, tile_properties[TP_WARP], game->colors[COLOR_RUST], game->renderer);
    // grass_warp.destination_map = 1;

    u32 tileWidth = 80;
    u32 tileHeight = 80;
    Map map0 = {};
    map0.tile_width = tileWidth;
    map0.tile_height = tileHeight;
    map0.current = true;
    map0.rows = 10;
    map0.cols = 12;
    for (u32 row = 0; row < map0.rows; ++row)
    {
        for (u32 col = 0; col < map0.cols; ++col)
        {
            Entity *tile = &map0.entities[map0.entityCount++];
            tile->width = tileWidth;
            tile->height = tileHeight;
            tile->position = {col*tile->width + 0.5f*tile->width, row*tile->height + 0.5f*tile->height};
            if (row == 0 || col == 0 || row == map0.rows - 1 || col == map0.cols - 1)
            {
                // tile->flags = tile_properties[TP_SOLID];
                tile->color = game->colors[COLOR_GREEN];
                tile->collides = true;
            }
            if (row == 4 && col == 1)
            {
                // Quicksand
                tile->tileFlags = tile_properties[TP_QUICKSAND];
                tile->color = game->colors[COLOR_BROWN];
                tile->collides = false;
            }
            if (row == 2 && col == 4)
            {
                // Harvestable tree
                tile->tileFlags = tile_properties[TP_HARVEST];
                tile->color = game->colors[COLOR_NONE];
                tile->collides = true;
                initEntitySpriteSheet(tile, "sprites/tree.png", 1, 1, game->renderer);
                tile->active = true;
                tile->isHarvestable = true;
                tile->harvestedItem = INV_LEAVES;
            }
        }
    }
    map0.width_pixels = map0.cols * tileWidth;
    map0.height_pixels = map0.rows * tileHeight;
    map0.texture = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
                                     map0.width_pixels, map0.height_pixels);

    // Harlod
    Entity *harlod = &map0.entities[map0.entityCount++];
    *harlod = {};
    initEntitySpriteSheet(harlod, "sprites/Harlod_the_caveman.png", 1, 1, game->renderer);
    harlod->collides = true;
    harlod->width = 20;
    harlod->height = 10;
    harlod->spriteDims = {60, 60};
    harlod->starting_pos = {300, 300};
    harlod->position = harlod->starting_pos;
    harlod->speed = 10;
    harlod->type = ET_HARLOD;
    // harlod->onHeroInteract = &harlodInteractWithHero;
    harlod->active = true;

    // Knight
    Entity *knight = &map0.entities[map0.entityCount++];
    initEntitySpriteSheet(knight, "sprites/knight_alligned.png", 8, 5, game->renderer);
    // initEntityWidthHeight(&knight.e);
    knight->collides = true;
    knight->width = 20;
    knight->height = 10;
    knight->spriteDims = {45, 45};
    knight->starting_pos = {500, 500};
    knight->position = knight->starting_pos;
    knight->can_move = true;
    knight->speed = 1000;
    knight->collision_pt_offset = 25;
    knight->type = ET_ENEMY;
    knight->active = true;
    knight->sprite_rect.y = knight->sprite_rect.h * 3 + 4;
    initAnimation(&knight->animation, 2, 400);

    Entity* map0_entities[] = {&hero.e, knight, harlod};
    map0.active_entities.entities = map0_entities;
    map0.active_entities.count = ArrayCount(map0_entities);

    game->current_map = &map0;
    initCamera(game);
    // game->maps = &map_list;
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
            // updateMap(game);
            updateHero(&hero, &input, game);
            updateCamera(&game->camera, hero.e.position);
            // updateEntityList(&entity_list, game->current_map, game->dt);
            updateAnimation(&hero.e.animation, game->dt, hero.isMoving);
            sound_play_all(game->sounds, now);

            // update knight
            updateAnimation(&knight->animation, game->dt, true);
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

        // Player interaction region
        // drawCircle(game->renderer, (int)hero.e.position.x, (int)(hero.e.position.y - hero.e.height), 30);

        // Draw sword for knight walking right
        // int swordSpriteWidth = 16;
        // int swordSpriteHeight = 50;
        // SDL_Rect knightSwordLocationInSheet = {188, 1, swordSpriteWidth, swordSpriteHeight};
        // int currentKnightFrame = knight.e.sprite_rect.x / knight.e.sprite_rect.w;
        // bool swordIsUp = currentKnightFrame == 0 || currentKnightFrame == 3;
        // int swordOffsetX = 2;
        // int swordOffsetY = swordIsUp ? -13 : -11;
        // SDL_Rect knightSwordLocationOnMap = {knight.e.dest_rect.x + swordOffsetX,
        //                                      knight.e.dest_rect.y + swordOffsetY,
        //                                      swordSpriteWidth, swordSpriteHeight};
        // SDL_RenderCopy(game->renderer, knight.e.sprite_sheet.sheet,
        //                &knightSwordLocationInSheet, &knightSwordLocationOnMap);

        if (game->current_map == &map0)
        {
            // Draw sword for knight attacking to right
            Sprite attackingSwordRaised = {};
            attackingSwordRaised.x = 188;
            attackingSwordRaised.y = 1;
            attackingSwordRaised.width = 16;
            attackingSwordRaised.height = 50;
            attackingSwordRaised.offsetX = 2;
            attackingSwordRaised.offsetY = -25;

            Sprite attackingSwordDown = {};
            attackingSwordDown.x = 307;
            attackingSwordDown.y = 1;
            attackingSwordDown.width = 50;
            attackingSwordDown.height = 16;
            attackingSwordDown.offsetX = 12;
            attackingSwordDown.offsetY = 11;

            int currentFrame = knight->sprite_rect.x / knight->sprite_rect.w;
            Sprite *currentSwordSprite = currentFrame == 0 ? &attackingSwordRaised : &attackingSwordDown;

            SDL_Rect raisedSwordLocationInSheet = {currentSwordSprite->x, currentSwordSprite->y,
                                                currentSwordSprite->width, currentSwordSprite->height};

            SDL_Rect raisedSwordLocationOnMap = {(int)knight->position.x + currentSwordSprite->offsetX,
                                                 (int)knight->position.y + currentSwordSprite->offsetY,
                                                 currentSwordSprite->width, currentSwordSprite->height};
            // SDL_RenderCopy(game->renderer, knight.e.sprite_sheet.sheet,
            //             &raisedSwordLocationInSheet, &raisedSwordLocationOnMap);
        }

        // SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
        // drawCircle(game->renderer, (i32)heroInteractionRegion.center.x,
        //                 (i32)heroInteractionRegion.center.y, (i32)heroInteractionRegion.radius);

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
            checkEntityCollisionsWithEntities(e, game);
        }

        // Draw FPS
        f32 fps = 1000.0f / game->dt;
        char fps_str[9] = {0};
        snprintf(fps_str, 9, "FPS: %03d", (u32)fps);
        drawText(game, &fontMetadata, fps_str, game->camera.viewport.x, game->camera.viewport.y);

        // char pos_str[20] = {0};
        // snprintf(pos_str, 20, "x: %.2f, y: %.2f", hero.e.position.x, hero.e.position.y);
        // drawText(game, &fontMetadata, pos_str, game->camera.viewport.x, game->camera.viewport.y);

        SDL_SetRenderTarget(game->renderer, NULL);
        SDL_RenderCopy(game->renderer, game->current_map->texture, &game->camera.viewport, NULL);

        game->dt = SDL_GetTicks() - now;
        sleepIfAble(game);

        SDL_RenderPresent(game->renderer);
        fflush(stdout);
    }

    /**************************************************************************/
    /* Cleanup                                                                */
    /**************************************************************************/
    SDL_DestroyTexture(transparentBlackTexture);
    destroyFontMetadata(&fontMetadata);
    // destroyTileList(&tile_list);
    // destroyTileset(&jungle_tiles);
    // map_list_destroy(&map_list);
    // destroyEntityList(&entity_list);
    destroyControllers(&input);
    game_destroy(game);
    destroyArena(&arena);

    return 0;
}
