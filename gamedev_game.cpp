#include "gamedev_game.h"

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
    destroyTexture(g->linkTexture);
    destroyTexture(g->harvestableTreeTexture);
    destroyTexture(g->harlodTexture);
    destroyTexture(g->knightTexture);
    destroyTexture(g->flameTexture);
    destroyTexture(g->firePitTexture);
    destroyTexture(g->glowTreeTexture);
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

void updateGame(Game *g, Input *input)
{
    if (input->keyPressed[KEY_ESCAPE])
    {
        g->running = false;
    }
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

