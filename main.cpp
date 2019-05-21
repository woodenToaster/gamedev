#define _CRT_SECURE_NO_WARNINGS

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// #define SDL_MAIN_HANDLED

#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_mixer.h"

// TODO(cjh): Remove standard library dependency
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "gamedev_definitions.h"
#include "gamedev_animation.cpp"

#include "gamedev_memory.h"
#include "gamedev_renderer.h"
#include "gamedev_math.h"
#include "gamedev_font.h"
#include "gamedev_sound.h"
#include "gamedev_input.h"
#include "gamedev_camera.cpp"
#include "gamedev_game.h"
#include "gamedev_sprite_sheet.h"
#include "gamedev_tilemap.h"
#include "gamedev_entity.h"
#include "gamedev_camera.h"

#include "gamedev_memory.cpp"
#include "gamedev_renderer.cpp"
#include "gamedev_font.cpp"
#include "gamedev_sound.cpp"
#include "gamedev_asset_loading.cpp"
#include "gamedev_input.cpp"
#include "gamedev_game.cpp"
#include "gamedev_sprite_sheet.cpp"
#include "gamedev_entity.cpp"
#include "gamedev_tilemap.cpp"


int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    GameMemory memory = {};
    memory.permanentStorageSize = (size_t)MEGABYTES(1);
    memory.transientStorageSize = (size_t)MEGABYTES(4);
    memory.permanentStorage = calloc(memory.permanentStorageSize + memory.transientStorageSize, sizeof(u8));
    memory.transientStorage = (u8*)memory.permanentStorage + memory.permanentStorageSize;

    // Game
    assert(sizeof(Game) < memory.permanentStorageSize);
    Game* game = (Game*)memory.permanentStorage;
    initArena(&game->worldArena, memory.permanentStorageSize - sizeof(Game),
              (u8*)memory.permanentStorage + sizeof(Game));
    initArena(&game->transientArena, memory.transientStorageSize, (u8*)memory.transientStorage);

    u32 screenWidth = 960;
    u32 screenHeight = 540;
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

    game->linkTexture = createTextureFromPng("sprites/link_walking.png", game->renderer);
    game->harvestableTreeTexture = createTextureFromPng("sprites/harvestable_tree.png", game->renderer);
    game->harlodTexture = createTextureFromPng("sprites/Harlod_the_caveman.png", game->renderer);
    game->knightTexture = createTextureFromPng("sprites/knight_alligned.png", game->renderer);
    game->flameTexture = createTextureFromPng("sprites/flame.png", game->renderer);
    game->firePitTexture = createTextureFromPng("sprites/fire_pit.png", game->renderer);
    game->glowTreeTexture = createTextureFromPng("sprites/glow_tree.png", game->renderer);

    // Input
    Input input = {};
    initControllers(&input);

    // Font
    FontMetadata fontMetadata = {};
    generateFontData(&fontMetadata, game);

    // Map
    u32 tileWidth = 80;
    u32 tileHeight = 80;
    Map *map0 = PushStruct(&game->worldArena, Map);
    map0->tileWidth = tileWidth;
    map0->tileHeight = tileHeight;
    map0->rows = 10;
    map0->cols = 12;
    map0->widthPixels = map0->cols * tileWidth;
    map0->heightPixels = map0->rows * tileHeight;
    map0->texture = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                      map0->widthPixels, map0->heightPixels);

    for (u32 row = 0; row < map0->rows; ++row)
    {
        for (u32 col = 0; col < map0->cols; ++col)
        {
            Entity *tile = addEntity(map0);
            tile->type = ET_TILE;
            tile->color = game->colors[Color_None];
            tile->width = map0->tileWidth;
            tile->height = map0->tileHeight;
            tile->position = {col*tile->width + 0.5f*tile->width, row*tile->height + 0.5f*tile->height};
            if (row == 0 || col == 0 || row == map0->rows - 1 || col == map0->cols - 1)
            {
                addTileFlags(tile, TP_SOLID);
                tile->color = game->colors[Color_DarkGreen];
                tile->collides = true;
            }
            if (row == 4 && col == 1)
            {
                // Quicksand
                addTileFlags(tile, TP_QUICKSAND);
                tile->color = game->colors[Color_Brown];
                tile->collides = false;
            }
            if ((row == 2 && (col == 4 || col == 5 || col == 6 || col == 7)) ||
                ((row == 3 || row == 4 || row == 5 || row == 6) && col == 7))
            {
                // Harvestable tree
                addTileFlags(tile, (u32)(TP_HARVEST | TP_SOLID | TP_FLAMMABLE));
                tile->color = game->colors[Color_None];
                tile->collides = true;
                tile->burntTileIndex = 2;
                initEntitySpriteSheet(tile, game->harvestableTreeTexture, 3, 1);
                tile->active = true;
                tile->isHarvestable = true;
                tile->harvestedItem = INV_LEAVES;
            }

            if (row == 2 && col == 2)
            {
                // Glow tree
                addTileFlags(tile, (u32)(TP_HARVEST | TP_SOLID));
                tile->collides = true;
                initEntitySpriteSheet(tile, game->glowTreeTexture, 2, 1);
                tile->active = true;
                tile->isHarvestable = true;
                tile->harvestedItem = INV_GLOW;
            }

            if (row == 1 && col == 7)
            {
                // Lightable fire
                addTileFlags(tile, TP_CAMPFIRE | TP_INTERACTIVE);
                initEntitySpriteSheet(tile, game->firePitTexture, 1, 1);
            }
        }
    }

    // Hero
    Entity *hero = addEntity(map0);
    initEntitySpriteSheet(hero, game->linkTexture, 11, 5);
    hero->spriteSheet.scale = 2;
    hero->width = 20;
    hero->height = 10;
    hero->spriteDims = {45, 60};
    initAnimation(&hero->animation, 8, 80);
    hero->position = {120, 120};
    hero->speed = 2000;
    hero->active = true;
    hero->type = ET_HERO;

    // Harlod
    // Entity *harlod = addEntity(map0);
    // *harlod = {};
    // initEntitySpriteSheet(harlod, game->harlodTexture, 1, 1);
    // harlod->collides = true;
    // harlod->active = true;
    // harlod->width = 20;
    // harlod->height = 10;
    // harlod->spriteDims = {60, 60};
    // harlod->position = {300, 300};
    // harlod->speed = 10;
    // harlod->active = true;
    // harlod->type = ET_HARLOD;

    // Knight
    // Entity *knight = addEntity(map0);
    // initEntitySpriteSheet(knight, game->knightTexture, 8, 5);
    // knight->collides = true;
    // knight->width = 20;
    // knight->height = 10;
    // knight->spriteDims = {45, 45};
    // knight->position = {500, 500};
    // knight->speed = 1000;
    // knight->type = ET_ENEMY;
    // knight->active = true;
    // knight->spriteRect.y = knight->spriteRect.h * 3 + 4;
    // initAnimation(&knight->animation, 2, 400);

    game->currentMap = map0;
    initCamera(game);

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

        switch (game->mode)
        {
            case GAME_MODE_PLAYING:
            {
                updateGame(game, &input);
                updateHero(hero, &input, game);
                updateCamera(&game->camera, hero->position);
                updateAnimation(&hero->animation, game->dt, hero->isMoving);
                playQueuedSounds(&game->sounds, now);
                updateTiles(game);
                break;
            }
            case GAME_MODE_DIALOGUE:
            {
                updateDialogMode(game, &input);
                break;
            }
            case GAME_MODE_INVENTORY:
            {
                updateInventoryMode(game, &input);
                break;
            }
        }

        /*********************************************************************/
        /* Draw                                                              */
        /*********************************************************************/
        TemporaryMemory renderMemory = beginTemporaryMemory(&game->transientArena);
        RenderGroup *group = allocateRenderGroup(&game->transientArena, MEGABYTES(2));

        SDL_SetRenderTarget(game->renderer, game->currentMap->texture);
        drawBackground(group, game);
        drawTiles(group, game);
        drawEntities(group, game);
        drawPlacingTile(group, game, hero);
        drawHUD(group, game, hero, &fontMetadata);

        // Hero interaction region
        // SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
        // drawCircle(game->renderer, (i32)heroInteractionRegion.center.x,
        //                 (i32)heroInteractionRegion.center.y, (i32)heroInteractionRegion.radius);

        if (game->mode == GAME_MODE_DIALOGUE)
        {
            darkenBackground(group, game);
            drawDialogScreen(group, game, &fontMetadata);
        }

        if (game->mode == GAME_MODE_INVENTORY)
        {
            darkenBackground(group, game);
            drawInventoryScreen(group, game, hero, &fontMetadata);
        }

        // sortRenderGroup(group);
        drawRenderGroup(game->renderer, group);

        /**************************************************************************/
        /* Debug text                                                             */
        /**************************************************************************/

#if 0
        f32 fps = 1000.0f / game->dt;
        char fpsText[9] = {0};
        snprintf(fpsText, 9, "FPS: %03d", (u32)fps);
        drawText(group, &fontMetadata, fpsText, game->camera.viewport.x, game->camera.viewport.y);

        char heroPosition[20] = {0};
        snprintf(heroPosition, 20, "x: %.2f, y: %.2f", hero->position.x, hero->position.y);
        drawText(group, &fontMetadata, heroPosition, game->camera.viewport.x, game->camera.viewport.y);

        char bytesUsed[35] = {};
        snprintf(bytesUsed, 35, "%zu / %zu bytes in use", game->worldArena.used, game->worldArena.maxCap);
        drawText(group, &fontMetadata, bytesUsed, game->camera.viewport.x, game->camera.viewport.y);
#endif

        SDL_SetRenderTarget(game->renderer, NULL);
        SDL_RenderCopy(game->renderer, game->currentMap->texture, &game->camera.viewport, NULL);
        game->dt = SDL_GetTicks() - now;
        sleepIfAble(game);
        SDL_RenderPresent(game->renderer);

        endTemporaryMemory(renderMemory);

        checkArena(&game->transientArena);
        checkArena(&game->worldArena);
    }

    /**************************************************************************/
    /* Cleanup                                                                */
    /**************************************************************************/
    destroyFontMetadata(&fontMetadata);
    destroyControllers(&input);
    destroyMap(map0);
    destroyGame(game);
    free(memory.permanentStorage);

    return 0;
}
