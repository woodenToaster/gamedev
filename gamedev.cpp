#include "gamedev.h"

void initArena(Arena *arena, size_t bytes, u8 *start)
{
    arena->start = start;
    arena->maxCap = bytes;
    arena->used = 0;
    arena->tmpCount = 0;
}

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

u8* pushSize(Arena *arena, size_t size)
{
    assert(size + arena->used < arena->maxCap);
    u8* result = arena->start + arena->used;
    arena->used += size;
    memset(result, 0, size);
    return result;
}

void destroyGame(Game* g)
{
    // TODO(cjh): This should be stored in gameMemory instead of malloced
    free(g->dialogue);
    rendererAPI.destroyTexture(g->linkTexture);
    rendererAPI.destroyTexture(g->harvestableTreeTexture);
    rendererAPI.destroyTexture(g->harlodTexture);
    rendererAPI.destroyTexture(g->knightTexture);
    rendererAPI.destroyTexture(g->flameTexture);
    rendererAPI.destroyTexture(g->firePitTexture);
    rendererAPI.destroyTexture(g->glowTreeTexture);
}

void initCamera(Game* g)
{
    g->camera.viewport.w = g->screenWidth;
    g->camera.viewport.h = g->screenHeight;
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
    g->camera.yPixelMovementThreshold = g->screenHeight / 2;
    g->camera.xPixelMovementThreshold = g->screenWidth / 2;
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
    if (input->keyPressed[KEY_ESCAPE])
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
    if (input->keyPressed[KEY_ESCAPE])
    {
        endInventoryMode(g);
    }
}

internal void setKeyState(Input* input, Key key, b32 isDown)
{
    if (input->keyDown[key] && !isDown)
    {
        input->keyPressed[key] = true;
    }
    input->keyDown[key] = isDown;
}

internal f32 normalizeStickInput(short unnormalizedStick)
{
    f32 result = 0.0f;
    if (unnormalizedStick < 0)
    {
        result = (f32)unnormalizedStick / 32768.0f;
    }
    else
    {
        result = (f32)unnormalizedStick / 32767.0f;
    }
    return result;
}

void initAnimation(Animation* a, int frames, int ms_delay)
{
    a->totalFrames = frames;
    a->delay = ms_delay;
}

internal void updateAnimation(Animation* a, u32 elapsed_last_frame, b32 active)
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

void gameUpdateAndRender(GameMemory *memory, Input *input, TextureHandle outputTarget, Rect *viewport,
                         RendererHandle renderer, i32 screenWidth, i32 screenHeight)
{
    assert(sizeof(Game) < memory->permanentStorageSize);
    Game* game = (Game*)memory->permanentStorage;
    u64 now = memory->currentTickCount;
    game->dt = memory->dt;

    if (!memory->isInitialized)
    {
        initArena(&game->worldArena, memory->permanentStorageSize - sizeof(Game),
                  (u8*)memory->permanentStorage + sizeof(Game));
        initArena(&game->transientArena, memory->transientStorageSize, (u8*)memory->transientStorage);

        game->targetFps = 60;
        memory->targetMsPerFrame = (u32)(1000.0f / (f32)game->targetFps);

        game->renderer = renderer;
        game->colors = memory->colors;
        game->linkTexture = memory->linkTexture;
        game->harvestableTreeTexture = memory->harvestableTreeTexture;
        game->flameTexture = memory->flameTexture;
        game->firePitTexture = memory->firePitTexture;
        game->glowTreeTexture = memory->glowTreeTexture;
        game->mudSound = memory->mudSound;
        game->fontMetadata = &memory->fontMetadata;

        game->screenWidth = screenWidth;
        game->screenHeight = screenHeight;


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
        map0->texture = outputTarget;

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
        game->hero = addEntity(map0);
        Entity *hero = game->hero;
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

        game->currentMap = map0;
        initCamera(game);

        memory->isInitialized = true;
    }

    TemporaryMemory renderMemory = beginTemporaryMemory(&game->transientArena);
    RenderGroup *group = allocateRenderGroup(&game->transientArena, MEGABYTES(2));

    if (input->keyPressed[KEY_I])
    {
        startInventoryMode(game);
    }

    Entity *hero = game->hero;
    switch (game->mode)
    {
    case GameMode_Playing:
    {
        if (input->keyPressed[KEY_ESCAPE])
        {
            globalRunning = false;
        }
        updateHero(group, hero, input, game);
        updateCamera(&game->camera, hero->position);
        *viewport = game->camera.viewport;
        updateAnimation(&hero->animation, game->dt, hero->isMoving);
        playQueuedSounds(&game->sounds, now);
        updateTiles(game);
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

    drawBackground(group, game);
    drawTiles(group, game);
    drawEntities(group, game);
    drawPlacingTile(group, game, hero);
    drawHUD(group, game, hero, &memory->fontMetadata);

    if (game->mode == GameMode_Dialogue)
    {
        darkenBackground(group, game);
        drawDialogScreen(group, game, &memory->fontMetadata);
    }

    if (game->mode == GameMode_Inventory)
    {
        darkenBackground(group, game);
        drawInventoryScreen(group, game, hero, &memory->fontMetadata);
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

    endTemporaryMemory(renderMemory);

    checkArena(&game->transientArena);
    checkArena(&game->worldArena);
}
