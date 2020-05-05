#include "gamedev_platform.h"
#include "gamedev_math.h"
#include "gamedev_sprite_sheet.h"
#include "gamedev.h"
#include "gamedev_renderer.h"

#include "gamedev_font.h"
#include "gamedev_sprite_sheet.h"
#include "gamedev_tilemap.h"
#include "gamedev_entity.h"

#include "gamedev_renderer.cpp"
#include "gamedev_font.cpp"
#include "gamedev_sound.cpp"
#include "gamedev_sprite_sheet.cpp"
#include "gamedev_asset_loading.cpp"
#include "gamedev_entity.cpp"
#include "gamedev_tilemap.cpp"


internal TemporaryMemory beginTemporaryMemory(Arena *arena)
{
    TemporaryMemory result = {};
    result.arena = arena;
    result.used = arena->used;
    arena->tmpCount++;

    return result;
}

internal void endTemporaryMemory(TemporaryMemory tempMem)
{
    Arena *arena = tempMem.arena;
    assert(arena->used >= tempMem.used);
    arena->used = tempMem.used;
    assert(arena->tmpCount > 0);
    --arena->tmpCount;
}

internal void checkArena(Arena *arena)
{
    assert(arena->tmpCount == 0);
}

void destroyGame(Game* g)
{
    // TODO(cjh): This should be stored in gameMemory instead of malloced
    free(g->dialogue);
    // TODO(cjh): Don't require typing these in here (may forget)
    // TODO(chogan): Clean up gl texture handles
    // rendererAPI.destroyTexture(g->heroTexture);
    // rendererAPI.destroyTexture(g->harvestableTreeTexture);
    // rendererAPI.destroyTexture(g->harlodTexture);
    // rendererAPI.destroyTexture(g->knightTexture);
    // rendererAPI.destroyTexture(g->flameTexture);
    // rendererAPI.destroyTexture(g->firePitTexture);
    // rendererAPI.destroyTexture(g->glowTreeTexture);
    // rendererAPI.destroyTexture(g->iconsTexture);
}

#if 0
void initCamera(Game* g, i32 screenWidth, i32 screenHeight)
{
    g->camera.viewport.w = screenWidth;
    g->camera.viewport.h = screenHeight;
    g->camera.startingPos = {0, 0};

    if (g->camera.viewport.w >= g->currentMap->widthPixels)
    {
        g->camera.maxX = 0;
    }
    else
    {
        g->camera.maxX = g->currentMap->widthPixels - g->camera.viewport.w;
    }
    if (g->camera.viewport.h >= g->currentMap->heightPixels)
    {
        g->camera.maxY = 0;
    }
    else
    {
        g->camera.maxY = absInt32(g->currentMap->heightPixels - g->camera.viewport.h);
    }
    g->camera.yPixelMovementThreshold = screenHeight / 2;
    g->camera.xPixelMovementThreshold = screenWidth / 2;
}
#else
void initCamera(Camera *camera)
{
    camera->position = vec3(0.0f, 0.0f, 1.75f);
    camera->up = vec3(0.0f, 1.0f, 0.0f);
    camera->right = vec3(1.0f, 0.0f, 0.0f);
    camera->direction = vec3(0.0f, 0.0f, 1.0);
}
#endif


void initColors(Vec4u8 *colors)
{
    colors[Color_None] = vec4u8(0, 0, 0, 0);
    colors[Color_White] = vec4u8(255, 255, 255, 255);
    colors[Color_DarkGreen] = vec4u8(37, 71, 0, 255);
    colors[Color_Blue] = vec4u8(0, 0, 255, 255);
    colors[Color_Yellow] = vec4u8(235, 245, 65, 255);
    colors[Color_Brown] = vec4u8(153, 102, 0, 255);
    colors[Color_Rust] = vec4u8(153, 70, 77, 255);
    colors[Color_Magenta] = vec4u8(255, 0, 255, 255);
    colors[Color_Black] = vec4u8(0, 0, 0, 255);
    colors[Color_Red] = vec4u8(255, 0, 0, 255);
    colors[Color_Grey] = vec4u8(135, 135, 135, 255);
    colors[Color_DarkBlue] = vec4u8(0, 51, 102, 255);
    colors[Color_DarkOrange] = vec4u8(255, 140, 0, 255);
    colors[Color_BabyBlue] = vec4u8(137, 207, 240, 255);
    colors[Color_LimeGreen] = vec4u8(106, 190, 48, 255);
}

void startDialogueMode(Game *g, char *dialogue)
{
    g->mode = GameMode_Dialogue;
    g->dialogue = dialogue;
}

void endDialogMode(Game *g)
{
    g->mode = GameMode_Playing;
    g->dialogue = NULL;
}

void updateDialogMode(Game *g, Input *input)
{
    if (input->keyPressed[Key_B])
    {
        endDialogMode(g);
    }
}

void startInventoryMode(Game *g)
{
    g->mode = GameMode_Inventory;
}

void endInventoryMode(Game *g)
{
    g->mode = GameMode_Playing;
}

void updateInventoryMode(Game *g, Input *input)
{
    if (input->keyPressed[Key_B])
    {
        endInventoryMode(g);
    }
}

void initAnimation(Animation* a, int frames, int ms_delay)
{
    a->totalFrames = frames;
    a->delay = ms_delay;
}

internal void updateAnimation(Animation* a, u64 elapsed_last_frame, b32 active)
{
    a->elapsed += elapsed_last_frame;
    if (a->elapsed > a->delay && active) {
        a->currentFrame++;
        if (a->currentFrame > a->totalFrames - 1) {
            a->currentFrame = 0;
        }
        a->elapsed = 0;
    }
}

#if GAMEDEV_SDL
internal void centerCameraOverPoint(Camera* c, Vec2 pos)
{
    c->viewport.x = (int)pos.x - c->viewport.w / 2;
    c->viewport.y = (int)pos.y - c->viewport.h / 2;
}

internal void updateCamera(Camera* c, Vec2 centerPos)
{
    centerCameraOverPoint(c, centerPos);
    c->viewport.x = clampInt32(c->viewport.x, 0, c->maxX);
    c->viewport.y = clampInt32(c->viewport.y, 0, c->maxY);
}

internal void playQueuedSounds(SoundList *sl, u64 now)
{
    for (u32 i = 0; i < sl->count; ++i)
    {
        audioAPI.playSound(sl->items[i], now);
        sl->items[i] = NULL;
    }
    sl->count = 0;
}
#else
internal void updateCamera(RenderCommands *commands, Game *game, Entity *hero)
{
    int worldWidth = game->currentMap->cols * game->currentMap->tileWidth;
    int worldHeight = game->currentMap->rows;  game->currentMap->tileHeight;

    f32 pixelsToMeters = 1.0f / commands->metersToPixels;

    Camera *camera = &game->camera;
    f32 viewportWidthInMeters = commands->windowWidth * pixelsToMeters;
    f32 viewportHeightInMeters = commands->windowHeight * pixelsToMeters;

    f32 minCameraPx = minFloat32(0.5f * viewportWidthInMeters, 0.5f * worldWidth);
    f32 minCameraPy = minFloat32(0.5f * viewportHeightInMeters, 0.5f * worldHeight);

    f32 maxCameraPx;
    if (worldWidth > viewportWidthInMeters)
    {
        maxCameraPx = worldWidth - 0.5f * viewportWidthInMeters;
    }
    else
    {
        maxCameraPx = 0.5f * worldWidth;
    }

    f32 maxCameraPy;
    if (worldHeight > viewportHeightInMeters)
    {
        maxCameraPy = worldHeight - 0.5f * viewportHeightInMeters;
    }
    else
    {
        maxCameraPy = 0.5f * worldHeight;
    }

    camera->position.x = hero->position.x + 0.5f * hero->size.x;
    camera->position.y = hero->position.y + 0.5f * hero->size.y;

    // camera->direction.x = pixelsToMeters * 0.5f * commands->windowWidth;
    // camera->direction.y = pixelsToMeters * 0.5f * commands->windowHeight;

    // camera->position.x = pixelsToMeters * 0.5f * commands->windowWidth;
    // camera->position.y = pixelsToMeters * 0.25f* commands->windowHeight - 6;

    // camera->direction.y -= 0.01f;
    // Vec3 normalizedDirection = normalizeV3(&camera->direction);
    // camera->up = crossV3(&camera->right, &normalizedDirection);
    // camera->up *= -1.0f;

    camera->position.x = clampFloat(camera->position.x, minCameraPx, maxCameraPx);
    camera->position.y = clampFloat(camera->position.y, minCameraPy, maxCameraPy);

    commands->camera = *camera;
}

#endif

internal void queueSound(SoundList *sl, Sound *s)
{
    sl->items[sl->count++] = s;
}

#if GAMEDEV_SDL
extern "C" void gameUpdateAndRender(GameMemory *memory, Input *input, TextureHandle outputTarget,
                                    Rect *viewport, void *rendererState)
#else
extern "C" void gameUpdateAndRender(GameMemory *memory, Input *input, RenderCommands *renderCommands, LoadedBitmap *codepointZ)
#endif
{
    platform = memory->platformAPI;
    rendererAPI = memory->rendererAPI;

    fontAPI = memory->fontAPI;
    audioAPI = memory->audioAPI;

    assert(sizeof(Game) < memory->permanentStorageSize);
    Game* game = (Game*)memory->permanentStorage;
#if GAMEDEV_SDL
    u64 now = memory->currentTickCount;
#endif

    f32 metersToPixels = renderCommands->metersToPixels;
    f32 pixelsToMeters = 1.0f / metersToPixels;

    ObjData castle_data = {};

    Glyph zGlyph = {};

    if (!memory->isInitialized)
    {
        initArena(&game->worldArena, memory->permanentStorageSize - sizeof(Game),
                  (u8*)memory->permanentStorage + sizeof(Game));
        initArena(&game->transientArena, memory->transientStorageSize, (u8*)memory->transientStorage);

        initColors(game->colors);

        castle_data = loadObjFromFile(&game->transientArena, &game->worldArena, "models/castle.obj");

        game->camera.position = vec3(0.0f, 0.0f, 1.63f);
        game->camera.up = vec3(0.0f, 1.0f, 0.0f);
        game->camera.right = vec3(1.0f, 0.0f, 0.0f);
        game->camera.direction = vec3(0.0f, 0.0f, 1.0f);

        zGlyph.bitmap = codepointZ;
        zGlyph.id = rendererAPI.loadTexture(codepointZ);

        // Asset loading
        // TODO(cjh): Packed asset file
        // TODO(cjh): asset streaming
#if GAMEDEV_SDL
        game->linkTexture = rendererAPI.createTextureFromPng("sprites/link_walking.png", rendererState);
        game->harvestableTreeTexture = rendererAPI.createTextureFromPng("sprites/harvestable_tree.png",
                                                                        rendererState);
        game->flameTexture = rendererAPI.createTextureFromPng("sprites/flame.png", rendererState);
        game->firePitTexture = rendererAPI.createTextureFromPng("sprites/fire_pit.png", rendererState);
        game->glowTreeTexture = rendererAPI.createTextureFromPng("sprites/glow_tree.png", rendererState);
        game->harlodTexture = rendererAPI.createTextureFromPng("sprites/Harlod_the_caveman.png",
                                                               rendererState);
        game->iconsTexture = rendererAPI.createTextureFromPng("sprites/fantasy_icons_transparent.png",
                                                              rendererState);
        game->knightTexture = rendererAPI.createTextureFromPng("sprites/knight_alligned.png",
                                                               rendererState);
        game->mudSound.chunk = audioAPI.loadWav("sounds/mud_walk.wav");
        game->mudSound.delay = 250;
        fontAPI.generateFontData(&game->fontMetadata, rendererState);
#endif
        // Map
        Map *map0 = PUSH_STRUCT(&game->worldArena, Map);
#if GAMEDEV_SDL
        u32 tileWidth = 80;
        u32 tileHeight = 80;
        map0->tileWidth = tileWidth;
        map0->tileHeight = tileHeight;
        map0->rows = 10;
        map0->cols = 12;
        map0->widthPixels = map0->cols * tileWidth;
        map0->heightPixels = map0->rows * tileHeight;
        map0->texture = outputTarget;

        for (u32 row = 0; row < map0->rows; ++row)
        {
            for (u32 col = 0; col < map0->cols; ++col)
            {
                Entity *tile = addEntity(map0);
                tile->type = EntityType_Tile;
                tile->isVisible = true;
                tile->color = game->colors[Color_None];
                tile->width = map0->tileWidth;
                tile->height = map0->tileHeight;
                tile->position = {col*tile->width + 0.5f*tile->width,
                    row*tile->height + 0.5f*tile->height};

                // NOTE(chogan): Walls
                if (row == 0 || col == 0 || row == map0->rows - 1 || col == map0->cols - 1)
                {
                    addTileFlags(tile, TileProperty_Solid);
                    tile->color = game->colors[Color_DarkGreen];
                    tile->collides = true;
                }
                // NOTE(chogan): Quicksand
                if (row == 4 && (col == 1 || col == 2 || col == 3 || col ==4))
                {
                    // addTileFlags(tile, TileProperty_Quicksand);
                    // tile->color = game->colors[Color_Brown];
                    // tile->collides = false;
                }
                if ((row == 2 && (col == 4 || col == 5 || col == 6 || col == 7)) ||
                    ((row == 3 || row == 4 || row == 5 || row == 6) && col == 7))
                {
                    // initHarvestableTree(tile, game);
                }

                if (row == 2 && col == 2)
                {
                    // Glow tree
                    // initGlowTree(tile, game);
                }

                if (row == 1 && col == 7)
                {
                    // Lightable fire
                    // addTileFlags(tile, TileProperty_Campfire | TileProperty_Interactive);
                    // initEntitySpriteSheet(tile, game->firePitTexture, 1, 1);
                }
            }
        }
#else

        f32 tileWidthMeters = 1.0f;
        f32 tileHeightMeters = 1.0f;
        map0->tileWidth = (u32)tileWidthMeters;
        map0->tileHeight = (u32)tileHeightMeters;

        int worldWidth = 16;
        int worldHeight = 9;
        map0->rows = worldHeight;
        map0->cols = worldWidth;

        u32 rowStart = 0;
        u32 colStart = 0;
        u32 rowEnd = worldHeight;
        u32 colEnd = worldWidth;

        for (u32 row = rowStart; row < rowEnd; ++row)
        {
            for (u32 col = colStart; col < colEnd; ++col)
            {
                Entity *tile = addEntity(map0);
                tile->type = EntityType_Tile;
                tile->isVisible = true;
                tile->color = game->colors[Color_None];
                tile->width = (i32)tileWidthMeters;
                tile->height = (i32)tileHeightMeters;
                tile->position = vec2((f32)col, (f32)row);
                tile->size = vec2(tileWidthMeters, tileHeightMeters);

                if (row == 0 || col == 0 || row == map0->rows - 1 || col == map0->cols - 1)
                {
                    addTileFlags(tile, TileProperty_Solid);
                    tile->color = game->colors[Color_DarkGreen];
                    tile->collides = true;
                }
                if ((row == 2 && (col == 4 || col == 5 || col == 6 || col == 7)) ||
                    ((row == 3 || row == 4 || row == 5 || row == 6) && col == 7))
                {
                    tile->texture.bitmap = rendererAPI.loadBitmap("sprites/harvestable_tree.bmp");
                    tile->texture.id = rendererAPI.loadTexture(&tile->texture.bitmap);
                    initHarvestableTree(tile, game);
                }
            }
        }
#endif
        // Hero
        game->hero = addEntity(map0);
        Entity *hero = game->hero;
        Vec2 heroScale = {1.875f, 1.875f};
        // Vec2 heroScale = {1.0f, 1.0f};
        hero->texture.bitmap  = rendererAPI.loadBitmap("sprites/link_walking.bmp");
        hero->texture.id = rendererAPI.loadTexture(&hero->texture.bitmap);
        initEntitySpriteSheet(hero, 11, 5, heroScale);
        // hero->width = 20;
        // hero->height = 10;
        hero->shouldAnimate = true;
        hero->position = {1.2f, 1.2f};
        hero->size = vec2(pixelsToMeters * 24, pixelsToMeters * 32);
        hero->direction = Direction_Down;
        hero->speed = 20;
        hero->isVisible = true;
        hero->type = EntityType_Hero;
        hero->animation.totalFrames = 8;
        hero->animation.delay = 80;

        // Icons sheet
#if 0
        int iconsInRow[] = {11, 5, 7, 16, 9, 16, 12, 16, 10, 16, 16, 16, 16, 16, 16, 15, 15, 11, 6, 0};
        SpriteSheet iconsSheet = {};
        initSpriteSheet(&iconsSheet, game->iconsTexture, 16, 20);
        for (int row = 0, iconIndex = -1; row < iconsSheet.numY; ++row)
        {
            for (int col = 0; col < iconsSheet.numX; ++col)
            {
                if (col == iconsInRow[row])
                {
                    break;
                }

                iconIndex++;
                assert(iconIndex < Icon_Count);
                Sprite *sprite = game->icons + iconIndex;
                sprite->x = col * iconsSheet.spriteWidth;
                sprite->y = row * iconsSheet.spriteHeight;
                sprite->width = iconsSheet.spriteWidth;
                sprite->height = iconsSheet.spriteHeight;
                sprite->sheet = iconsSheet;
            }
        }

        // Harlod
        Entity *harlod = addEntity(map0);
        *harlod = {};
        initEntitySpriteSheet(harlod, game->harlodTexture, 1, 1);
        harlod->collides = true;
        harlod->width = 20;
        harlod->height = 10;
        harlod->position = {0, 0};
        harlod->speed = 10;
        harlod->isVisible = true;
        harlod->type = EntityType_Harlod;
#endif
        game->currentMap = map0;
#if GAMEDEV_SDL
        initCamera(game, viewport->w, viewport->h);
#else
        // initCamera(&renderCommands->camera);
#endif

        memory->isInitialized = true;
    }

    // TemporaryMemory renderMemory = beginTemporaryMemory(&game->transientArena);
    // RenderGroup *group = allocateRenderGroup(&game->transientArena, MEGABYTES(2));

    if (input->keyPressed[Key_I])
    {
        startInventoryMode(game);
    }

    Entity *hero = game->hero;
    switch (game->mode)
    {
        case GameMode_Playing:
        {
            updateHero(renderCommands, hero, input, game);
            updateEntities(game, input);
#if GAMEDEV_SDL
            updateCamera(&game->camera, hero->position);
            viewport->x = game->camera.viewport.x;
            viewport->y = game->camera.viewport.y;
            playQueuedSounds(&game->sounds, now);
#else
            updateCamera(renderCommands, game, hero);
#endif
            updateTiles(game, input);
            break;
        }
        case GameMode_Dialogue:
        {
            updateDialogMode(game, input);
            break;
        }
        case GameMode_Inventory:
        {
            updateInventoryMode(game, input);
            break;
        }
    }

    drawBackground(renderCommands, game);
    drawTiles(renderCommands, game);
    pushTexture(renderCommands, hero);

    pushGlyph(renderCommands, &zGlyph, vec2(1, 1), vec2(0.5, 0.5));

    // drawEntities(renderCommands, game);
    // drawPlacingTile(renderCommands, game, hero);
    // drawHUD(renderCommands, game, hero, &game->fontMetadata);

    if (game->mode == GameMode_Dialogue)
    {
        darkenBackground(renderCommands, game);
        // drawDialogScreen(renderCommands, game, &game->fontMetadata);
    }

    if (game->mode == GameMode_Inventory)
    {
        darkenBackground(renderCommands, game);
        // drawInventoryScreen(renderCommands, game, hero, &game->fontMetadata);
    }

    /**************************************************************************/
    /* Debug text                                                             */
    /**************************************************************************/
#if 0
    // TODO(chogan): Move this to the platform layer
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
    // drawRenderGroup(game->renderer, group);

    // endTemporaryMemory(renderMemory);

    checkArena(&game->transientArena);
    checkArena(&game->worldArena);
}
