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

// NOTE(cjh): SDL has already taken care of endianness for these color accessors
u8 getAlphaFromU32(u32 color)
{
    u8 a = (u8)((color & 0xFF000000) >> 24);
    return a;
}

u8 getRedFromU32(u32 color)
{
    u8 r = (u8)((color & 0x00FF0000) >> 16);
    return r;
}

u8 getGreenFromU32(u32 color)
{
    u8 g = (u8)((color & 0x0000FF00) >> 8);
    return g;
}

u8 getBlueFromU32(u32 color)
{
    u8 b = (u8)((color & 0x000000FF) >> 0);
    return b;
}

void destroyGame(Game* g)
{
    // TODO(cjh): This should be stored in gameMemory instead of malloced
    free(g->dialogue);
    SDL_DestroyTexture(g->linkTexture);
    SDL_DestroyTexture(g->harvestableTreeTexture);
    SDL_DestroyTexture(g->harlodTexture);
    SDL_DestroyTexture(g->knightTexture);
    SDL_DestroyTexture(g->flameTexture);
    SDL_DestroyTexture(g->firePitTexture);
    SDL_DestroyTexture(g->glowTreeTexture);

    Mix_Quit();
    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    SDL_Quit();
}

void initColors(Game* g)
{
    SDL_PixelFormat* window_pixel_format = g->windowSurface->format;
    g->colors[Color_None] = 0;
    g->colors[Color_White] = SDL_MapRGB(window_pixel_format, 255, 255, 255);
    g->colors[Color_DarkGreen] = SDL_MapRGB(window_pixel_format, 37, 71, 0);
    g->colors[Color_Blue] = SDL_MapRGB(window_pixel_format, 0, 0, 255);
    g->colors[Color_Yellow] = SDL_MapRGB(window_pixel_format, 235, 245, 65);
    g->colors[Color_Brown] = SDL_MapRGB(window_pixel_format, 153, 102, 0);
    g->colors[Color_Rust] = SDL_MapRGB(window_pixel_format, 153, 70, 77);
    g->colors[Color_Magenta] = SDL_MapRGB(window_pixel_format, 255, 0, 255);
    g->colors[Color_Black] = SDL_MapRGB(window_pixel_format, 0, 0, 0);
    g->colors[Color_Red] = SDL_MapRGB(window_pixel_format, 255, 0, 0);
    g->colors[Color_Grey] = SDL_MapRGB(window_pixel_format, 135, 135, 135);
    g->colors[Color_DarkBlue] = SDL_MapRGB(window_pixel_format, 0, 51, 102);
    g->colors[Color_DarkOrange] = SDL_MapRGB(window_pixel_format, 255, 140, 0);
    g->colors[Color_BabyBlue] = SDL_MapRGB(window_pixel_format, 137, 207, 240);
    g->colors[Color_LimeGreen] = SDL_MapRGB(window_pixel_format, 106, 190, 48);
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

void initGame(Game* g, u32 width, u32 height)
{
    g->screenWidth = width;
    g->screenHeight = height;
    g->targetFps = 60;
    g->dt = (i32)((1.0f / (f32)g->targetFps) * 1000);
    g->targetMsPerFrame = (u32)(1000.0f / (f32)g->targetFps);

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

    g->window = SDL_CreateWindow("gamedev",
                                 30,
                                 50,
                                 g->screenWidth,
                                 g->screenHeight,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (g->window == NULL)
    {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        exit(1);
    }
    g->windowSurface = SDL_GetWindowSurface(g->window);

    g->renderer = SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED);

    if (g->renderer == NULL)
    {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        exit(1);
    }
    initColors(g);

    // Sounds
    g->mudSound.delay = 250;
    g->mudSound.chunk = loadWav("sounds/mud_walk.wav");

    g->initialized = true;
    g->running = true;
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
    g->mode = GAME_MODE_DIALOGUE;
    g->dialogue = dialogue;
}

void endDialogMode(Game *g)
{
    g->mode = GAME_MODE_PLAYING;
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
    g->mode = GAME_MODE_INVENTORY;
}

void endInventoryMode(Game *g)
{
    g->mode = GAME_MODE_PLAYING;
}

void updateInventoryMode(Game *g, Input *input)
{
    if (input->keyPressed[KEY_ESCAPE])
    {
        endInventoryMode(g);
    }
}

void drawDialogScreen(RenderGroup *group, Game *g, FontMetadata *fontMetadata)
{
    int thirdOfWidth = (int)(g->camera.viewport.w / 3);
    int fourthOfHeight = (int)(g->camera.viewport.h / 4);
    int dialogueBoxX = (int)(0.5 * (thirdOfWidth)) + g->camera.viewport.x;
    int dialogueBoxY = (int)((3 * (fourthOfHeight)) - 0.5 * fourthOfHeight) + g->camera.viewport.y;
    int dialogueBoxWidth = 2 * (thirdOfWidth);
    int dialogueBoxHeight = fourthOfHeight;
    SDL_Rect dialogueBoxDest = {dialogueBoxX,dialogueBoxY, dialogueBoxWidth, dialogueBoxHeight};
    pushFilledRect(group, dialogueBoxDest, g->colors[Color_BabyBlue]);
    drawText(group, fontMetadata, g->dialogue, dialogueBoxX, dialogueBoxY);
}

void drawInventoryScreen(RenderGroup *group, Game *g, Entity *h, FontMetadata *fontMetadata)
{
    int thirdOfWidth = (int)(g->camera.viewport.w / 3);
    int fourthOfHeight = (int)(g->camera.viewport.h / 4);
    int dialogueBoxX = (int)(0.5 * thirdOfWidth) + g->camera.viewport.x;
    int dialogueBoxY = (int)(0.5 * fourthOfHeight) + g->camera.viewport.y;
    int dialogueBoxWidth = 2 * (thirdOfWidth);
    int dialogueBoxHeight = fourthOfHeight;
    SDL_Rect dialogueBoxDest = {dialogueBoxX, dialogueBoxY, dialogueBoxWidth, dialogueBoxHeight};
    pushFilledRect(group, dialogueBoxDest, g->colors[Color_BabyBlue]);

    for (int inventoryIndex = 1; inventoryIndex < INV_COUNT; ++inventoryIndex)
    {
        char itemString[30] = {};
        switch ((InventoryItemType)inventoryIndex)
        {
        case INV_LEAVES:
            snprintf(itemString, 30, "Leaves: %d", h->inventory[INV_LEAVES]);
            break;
        case INV_GLOW:
            snprintf(itemString, 30, "Glow: %d", h->inventory[INV_GLOW]);
            break;
        default:
            assert(!"Wrong inventory type");
        }
        drawText(group, fontMetadata, itemString, dialogueBoxX, dialogueBoxY);
        // TODO(cjh): Figure out correct y offset for next line
        dialogueBoxY += 25;
    }
}

void drawHUD(RenderGroup *group, Game *g, Entity *h, FontMetadata *font)
{
    u8 beltSlots = 8;
    u8 slotSize = 40;
    i32 slotCenterX = g->camera.viewport.w / 2;
    i32 destX = slotCenterX - ((beltSlots / 2) * slotSize);
    i32 destY = g->camera.viewport.h + g->camera.viewport.y - slotSize;

    SDL_Rect backgroundDest = {destX, destY, beltSlots * slotSize, slotSize};

    // Draw transparent black background
    pushFilledRect(group, backgroundDest, g->colors[Color_Black], 128);
    // SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    // SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 128);
    // SDL_RenderFillRect(g->renderer, &backgroundDest);

    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
    for (int i = 0; i < beltSlots; ++i)
    {
        SDL_Rect dest = {destX + i*slotSize, destY, slotSize, slotSize};

        if ((u32)i < h->beltItemCount)
        {
            BeltItem *item = &h->beltItems[i];
            SDL_Texture *textureToDraw = NULL;
            SDL_Rect tileRect = {};

            switch (item->type)
            {
            case CRAFTABLE_TREE:
                textureToDraw = g->harvestableTreeTexture;
                // TODO(cjh): Get sprite width height
                tileRect.w = 64;
                tileRect.h = 64;
                break;
            case CRAFTABLE_GLOW_JUICE:
                textureToDraw = g->glowTreeTexture;
                // TODO(cjh): Get sprite width height
                tileRect.w = 80;
                tileRect.h = 80;
                break;
            default:
                break;
            }

            if (textureToDraw)
            {
                pushSprite(group, textureToDraw, tileRect, dest);
                // SDL_RenderCopy(g->renderer, textureToDraw, &tileRect, &dest);
            }
            // Draw inventory number
            assert(item->count <= 999);
            char numItems[4] = {};
            snprintf(numItems, 4, "%d", item->count);
            drawText(group, font, numItems, dest.x, dest.y);
        }
        pushRect(group, dest, g->colors[Color_White]);
        SDL_RenderDrawRect(g->renderer, &dest);
    }
}

void darkenBackground(Game *g)
{
    // TODO(cjh): Use RenderGroup
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 64);
    SDL_RenderFillRect(g->renderer, NULL);
}

void drawCircle(SDL_Renderer *renderer, i32 _x, i32 _y, i32 radius)
{
    i32 x = radius - 1;
    if (radius == 0)
    {
        return;
    }
    i32 y = 0;
    i32 tx = 1;
    i32 ty = 1;
    i32 err = tx - (2 * radius);
    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, _x + x, _y - y);
        SDL_RenderDrawPoint(renderer, _x + x, _y + y);
        SDL_RenderDrawPoint(renderer, _x - x, _y - y);
        SDL_RenderDrawPoint(renderer, _x - x, _y + y);
        SDL_RenderDrawPoint(renderer, _x + y, _y - x);
        SDL_RenderDrawPoint(renderer, _x + y, _y + x);
        SDL_RenderDrawPoint(renderer, _x - y, _y - x);
        SDL_RenderDrawPoint(renderer, _x - y, _y + x);

        if (err <= 0)
        {
            y++;
            err += ty;
            ty += 2;
        }
        if (err > 0)
        {
            x--;
            tx += 2;
            err += tx - (radius << 1);
        }
    }
}

void sleepIfAble(Game* g)
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
