#define _CRT_SECURE_NO_WARNINGS

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "gamedev_platform.h"

// #define SDL_MAIN_HANDLED

#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_mixer.h"

// TODO(cjh): Remove standard library dependency
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

global_variable PlatformAPI platform = {};
global_variable platformCreateTextureFromGreyscaleBitmap *createTextureFromGreyscaleBitmap;

#include "sdl2_gamedev.h"
#include "gamedev_definitions.h"
#include "gamedev_animation.h"
#include "gamedev_memory.h"
#include "gamedev_math.h"
#include "gamedev_renderer.h"
#include "gamedev_font.h"
#include "gamedev_sound.h"
#include "gamedev_input.h"
#include "gamedev_camera.h"
#include "gamedev_game.h"
#include "gamedev_sprite_sheet.h"
#include "gamedev_tilemap.h"
#include "gamedev_entity.h"

#include "gamedev_animation.cpp"
#include "gamedev_camera.cpp"
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

internal SDL_Texture* SDLCreateTextureFromPng(const char* fname, SDL_Renderer *renderer)
{
    unsigned char *img_data;
    int width;
    int height;
    int channels_in_file;
    img_data = stbi_load(fname, &width, &height, &channels_in_file, 0);

    if (!img_data)
    {
        printf("Loading image failed: %s\n", stbi_failure_reason());
        exit(1);
    }
    u32 rmask;
    u32 gmask;
    u32 bmask;
    u32 amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    bool bigEndian = true;
    int shift = (channels_in_file == STBI_rgb) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else
    bool bigEndian = false;
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = (channels_in_file == STBI_rgb) ? 0 : 0xff000000;
#endif

    int depth;
    int pitch;
    u32 pixelFormat;
    if (channels_in_file == STBI_rgb)
    {
        depth = 24;
        pitch = 3 * width;
        pixelFormat = bigEndian ? SDL_PIXELFORMAT_RGB888 : SDL_PIXELFORMAT_BGR888;
    }
    else
    {
        depth = 32;
        pitch = 4 * width;
        pixelFormat = bigEndian ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_ABGR8888;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STATIC, width, height);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(texture, NULL, (void*)img_data, pitch);

    stbi_image_free(img_data);

    return texture;
}

void SDLInitColors(Game *g, SDL_PixelFormat *pixelFormat)
{
    g->colors[Color_None] = 0xFFFFFFFF;
    g->colors[Color_White] = SDL_MapRGB(pixelFormat, 255, 255, 255);
    g->colors[Color_DarkGreen] = SDL_MapRGB(pixelFormat, 37, 71, 0);
    g->colors[Color_Blue] = SDL_MapRGB(pixelFormat, 0, 0, 255);
    g->colors[Color_Yellow] = SDL_MapRGB(pixelFormat, 235, 245, 65);
    g->colors[Color_Brown] = SDL_MapRGB(pixelFormat, 153, 102, 0);
    g->colors[Color_Rust] = SDL_MapRGB(pixelFormat, 153, 70, 77);
    g->colors[Color_Magenta] = SDL_MapRGB(pixelFormat, 255, 0, 255);
    g->colors[Color_Black] = SDL_MapRGB(pixelFormat, 0, 0, 0);
    g->colors[Color_Red] = SDL_MapRGB(pixelFormat, 255, 0, 0);
    g->colors[Color_Grey] = SDL_MapRGB(pixelFormat, 135, 135, 135);
    g->colors[Color_DarkBlue] = SDL_MapRGB(pixelFormat, 0, 51, 102);
    g->colors[Color_DarkOrange] = SDL_MapRGB(pixelFormat, 255, 140, 0);
    g->colors[Color_BabyBlue] = SDL_MapRGB(pixelFormat, 137, 207, 240);
    g->colors[Color_LimeGreen] = SDL_MapRGB(pixelFormat, 106, 190, 48);
}


TextureHandle SDLCreateTextureFromGreyscaleBitmap(Game *g, u8 *bitmap, i32 width, i32 height)
{
    TextureHandle result = {};

    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!surface)
    {
        printf("%s\n", SDL_GetError());
        exit(1);
    }

    SDL_LockSurface(surface);
    u8 *srcPixel = bitmap;
    u32 *destPixel = (u32*)surface->pixels;

    for (int i = 0; i < height * width; ++i)
    {
        u8 val = *srcPixel++;
        *destPixel++ = ((val << 24) | (val << 16) | (val << 8) | (val << 0));
    }
    SDL_UnlockSurface(surface);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(g->renderer, surface);
    result.texture = texture;
    stbtt_FreeBitmap(bitmap, 0);
    SDL_FreeSurface(surface);

    return result;
}

EntireFile SDLReadEntireFile(char *filename)
{
    EntireFile result = {};
    SDL_RWops *file = SDL_RWFromFile(filename, "rb");

    if (file)
    {
        Sint64 fileSize = SDL_RWseek(file, 0, RW_SEEK_END);
        if (fileSize >= 0)
        {
            result.size = (u64)fileSize;
            if (SDL_RWseek(file, 0, RW_SEEK_SET) >= 0)
            {
                result.contents = (u8*)malloc(result.size);
                SDL_RWread(file, (void*)result.contents, 1, result.size);
                SDL_RWclose(file);
            }
            else
            {
                // TODO(cjh):
                // printf(stderr, "%s\n", SDL_GetError());
            }
        }
        else
        {
            // TODO(cjh):
            // printf("%s\n", SDL_GetError());
        }
    }
    else
    {
        // TODO(cjh):
        // fprintf(stderr, "%s\n", SDL_GetError());
    }
    return result;
}

void SDLFreeFileMemory(EntireFile *file)
{
    if (file->contents)
    {
        free(file->contents);
    }
}

void SDLSleepIfAble(Game* g)
{
    if (g->dt < g->targetMsPerFrame)
    {
        while (g->dt < g->targetMsPerFrame)
        {
            u32 sleep_ms = g->targetMsPerFrame - g->dt;
            g->dt += sleep_ms;
            SDL_Delay(sleep_ms);
        }
    }
    else
    {
#ifdef DEBUG
        printf("Frame missed!\n");
#endif
    }

    g->totalFramesElapsed++;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    GameMemory memory = {};
    memory.permanentStorageSize = (size_t)MEGABYTES(1);
    memory.transientStorageSize = (size_t)MEGABYTES(4);
    memory.permanentStorage = calloc(memory.permanentStorageSize + memory.transientStorageSize, sizeof(u8));
    memory.transientStorage = (u8*)memory.permanentStorage + memory.permanentStorageSize;
    memory.platformAPI.readEntireFile = SDLReadEntireFile;
    memory.platformAPI.freeFileMemory = SDLFreeFileMemory;
    memory.platformAPI.getTicks = SDL_GetTicks;

    createTextureFromGreyscaleBitmap = SDLCreateTextureFromGreyscaleBitmap;

    platform = memory.platformAPI;

    // Game
    assert(sizeof(Game) < memory.permanentStorageSize);
    Game* game = (Game*)memory.permanentStorage;
    initArena(&game->worldArena, memory.permanentStorageSize - sizeof(Game),
              (u8*)memory.permanentStorage + sizeof(Game));
    initArena(&game->transientArena, memory.transientStorageSize, (u8*)memory.transientStorage);

    game->screenWidth = 960;
    game->screenHeight = 540;
    game->targetFps = 60;
    game->dt = (i32)((1.0f / (f32)game->targetFps) * 1000);
    game->targetMsPerFrame = (u32)(1000.0f / (f32)game->targetFps);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("SDL failed to initialize: %s\n", SDL_GetError());
        exit(1);
    }

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow("gamedev",
                              30,
                              50,
                              game->screenWidth,
                              game->screenHeight,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        exit(1);
    }

    game->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (game->renderer == NULL)
    {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_PixelFormat *pixelFormat = SDL_GetWindowSurface(window)->format;
    SDLInitColors(game, pixelFormat);

    game->initialized = true;
    game->running = true;

    // OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext ogl_context = SDL_GL_CreateContext(window);

    if (ogl_context == NULL)
    {
        fprintf(stderr, "Failed to create opengl context: %s\n", SDL_GetError());
        exit(1);
    }

    // Asset loading
    // TODO(cjh): Packed asset file
    // TODO(cjh): asset streaming

    // PNGs
    game->linkTexture.texture = SDLCreateTextureFromPng("sprites/link_walking.png", game->renderer);
    game->harvestableTreeTexture.texture = SDLCreateTextureFromPng("sprites/harvestable_tree.png", game->renderer);
    // game->harlodTexture = SDLCreateTextureFromPng("sprites/Harlod_the_caveman.png", game->renderer);
    // game->knightTexture = SDLCreateTextureFromPng("sprites/knight_alligned.png", game->renderer);
    game->flameTexture.texture = SDLCreateTextureFromPng("sprites/flame.png", game->renderer);
    game->firePitTexture.texture = SDLCreateTextureFromPng("sprites/fire_pit.png", game->renderer);
    game->glowTreeTexture.texture = SDLCreateTextureFromPng("sprites/glow_tree.png", game->renderer);

    // Sounds
    game->mudSound.delay = 250;
    game->mudSound.chunk = loadWav("sounds/mud_walk.wav");

    // Fonts
    FontMetadata fontMetadata = {};
    generateFontData(&fontMetadata, game);
    game->fontMetadata = &fontMetadata;

    // Input
    Input input = {};
    initControllers(&input);

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
    map0->texture.texture = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
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
            if (row == 4 && (col == 1 || col == 2 || col == 3 || col ==4))
            {
                // Quicksand
                addTileFlags(tile, TP_QUICKSAND);
                tile->color = game->colors[Color_Brown];
                tile->collides = false;
            }
            if ((row == 2 && (col == 4 || col == 5 || col == 6 || col == 7)) ||
                ((row == 3 || row == 4 || row == 5 || row == 6) && col == 7))
            {
                initHarvestableTree(tile, game);
            }

            if (row == 2 && col == 2)
            {
                // Glow tree
                initGlowTree(tile, game);
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

        TemporaryMemory renderMemory = beginTemporaryMemory(&game->transientArena);
        RenderGroup *group = allocateRenderGroup(&game->transientArena, MEGABYTES(2));

        /*********************************************************************/
        /* Input                                                             */
        /*********************************************************************/
        pollInput(&input, game);

        /*********************************************************************/
        /* Update                                                            */
        /*********************************************************************/

        switch (game->mode)
        {
            case GameMode_Playing:
            {
                updateGame(game, &input);
                updateHero(group, hero, &input, game);
                updateCamera(&game->camera, hero->position);
                updateAnimation(&hero->animation, game->dt, hero->isMoving);
                playQueuedSounds(&game->sounds, now);
                updateTiles(game);
                break;
            }
            case GameMode_Dialogue:
            {
                updateDialogMode(game, &input);
                break;
            }
            case GameMode_Inventory:
            {
                updateInventoryMode(game, &input);
                break;
            }
        }

        /*********************************************************************/
        /* Draw                                                              */
        /*********************************************************************/

        SDL_SetRenderTarget(game->renderer, (SDL_Texture*)game->currentMap->texture.texture);
        drawBackground(group, game);
        drawTiles(group, game);
        drawEntities(group, game);
        drawPlacingTile(group, game, hero);
        drawHUD(group, game, hero, &fontMetadata);

        // Hero interaction region
        // SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
        // drawCircle(game->renderer, (i32)heroInteractionRegion.center.x,
        //                 (i32)heroInteractionRegion.center.y, (i32)heroInteractionRegion.radius);

        if (game->mode == GameMode_Dialogue)
        {
            darkenBackground(group, game);
            drawDialogScreen(group, game, &fontMetadata);
        }

        if (game->mode == GameMode_Inventory)
        {
            darkenBackground(group, game);
            drawInventoryScreen(group, game, hero, &fontMetadata);
        }

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

        // TODO(cjh): sortRenderGroup(group);
        drawRenderGroup(game->renderer, group);

        SDL_SetRenderTarget(game->renderer, NULL);
        SDL_Rect viewport = {game->camera.viewport.x, game->camera.viewport.y,
                             game->camera.viewport.w, game->camera.viewport.h};
        SDL_RenderCopy(game->renderer, (SDL_Texture*)game->currentMap->texture.texture, &viewport, NULL);
        game->dt = SDL_GetTicks() - now;
        SDLSleepIfAble(game);
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
    Mix_Quit();
    SDL_DestroyRenderer(game->renderer);
    free(memory.permanentStorage);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
