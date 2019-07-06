
// TODO(cjh): Account for wrapping off the viewport
void drawText(RenderGroup *group, FontMetadata *fontMetadata, char* text, i32 x=0, i32 y=0)
{
    // Leave a little padding in case the character extends left
    i32 xpos = 2 + x;
    i32 baseline = fontMetadata->baseline + y;
    u32 at = 0;
    while (text[at])
    {
        CodepointMetadata *cpm = &fontMetadata->codepointMetadata[text[at]];
        i32 width = cpm->x1 - cpm->x0;
        i32 height = cpm->y1 - cpm->y0;
        SDL_Rect source = {cpm->x0, cpm->y0, width, height};
        SDL_Rect dest = {xpos, baseline + cpm->y0, width, height};

	    if (text[at] != ' ')
        {
            SDL_Texture *t = fontMetadata->textures[text[at]];
            SDL_Rect fullTexture = {0, 0, 0, 0};
            pushSprite(group, t, fullTexture, dest, RenderLayer_HUD);
            xpos += (int)(cpm->advance * fontMetadata->scale);
        }
        else
        {
            // TODO(cjh): Don't hardcode space size. We'd like to get it from cpm->advance
            // but we don't store any metadata for a space
            xpos += (int)(750 * fontMetadata->scale);
        }

        if (text[at + 1])
        {
            // add kerning value specific to this character and the next
            int toAdvance = stbtt_GetCodepointKernAdvance(&fontMetadata->info, text[at], text[at + 1]);
            xpos += (int)(fontMetadata->scale * toAdvance);
        }
        ++at;
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
    pushFilledRect(group, dialogueBoxDest, g->colors[Color_BabyBlue], RenderLayer_HUD);
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
    pushFilledRect(group, dialogueBoxDest, g->colors[Color_BabyBlue], RenderLayer_HUD);

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

    // Transparent black background
    SDL_Rect backgroundDest = {destX, destY, beltSlots * slotSize, slotSize};
    pushFilledRect(group, backgroundDest, g->colors[Color_Black], RenderLayer_HUD, 128);

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
            case Craftable_Tree:
                textureToDraw = g->harvestableTreeTexture;
                // TODO(cjh): Get sprite width height
                tileRect.w = 64;
                tileRect.h = 64;
                break;
            case Craftable_Glow_Juice:
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
                pushSprite(group, textureToDraw, tileRect, dest, RenderLayer_HUD);
            }
            // Draw inventory number
            assert(item->count <= 999);
            char numItems[4] = {};
            snprintf(numItems, 4, "%d", item->count);
            drawText(group, font, numItems, dest.x, dest.y);
        }
        u8 alpha = i == h->activeBeltItemIndex ? 255 : 128;
        pushRect(group, dest, g->colors[Color_White], RenderLayer_HUD, alpha);
    }
}

void darkenBackground(RenderGroup *group, Game *g)
{
    SDL_Rect dest = {};
    pushFilledRect(group, dest, g->colors[Color_Black], RenderLayer_HUD, 64);
}

void renderRect(SDL_Renderer *renderer, SDL_Rect *dest, u32 color, u8 alpha=255)
{
    u8 r = (u8)((color & 0x00FF0000) >> 16);
    u8 g = (u8)((color & 0x0000FF00) >> 8);
    u8 b = (u8)((color & 0x000000FF) >> 0);

    SDL_BlendMode blendMode;
    SDL_GetRenderDrawBlendMode(renderer, &blendMode);
    SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawRect(renderer, dest);
    SDL_SetRenderDrawBlendMode(renderer, blendMode);
}

void renderFilledRect(SDL_Renderer* renderer, SDL_Rect* dest, u32 color, u8 alpha=255)
{
    // Color_None is 0xFFFFFFFF. Set a tile's background color to Color_None to avoid
    // extra rendering.
    if (color != 0xFFFFFFFF)
    {
        u8 r = (u8)((color & 0x00FF0000) >> 16);
        u8 g = (u8)((color & 0x0000FF00) >> 8);
        u8 b = (u8)((color & 0x000000FF) >> 0);

        SDL_BlendMode blendMode;
        SDL_GetRenderDrawBlendMode(renderer, &blendMode);
        SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, dest);
        SDL_SetRenderDrawBlendMode(renderer, blendMode);
    }
}

#define PushRenderElement(group, type) (type*)pushRenderElement_(group, sizeof(type), RenderEntryType_##type)

internal void *pushRenderElement_(RenderGroup *group, u32 size, RenderEntryType type)
{
    void *result = 0;

    size += sizeof(RenderEntryHeader);

    if (group->bufferSize + size < group->maxBufferSize)
    {
        RenderEntryHeader *header = (RenderEntryHeader*)(group->bufferBase + group->bufferSize);
        header->type = type;
        result = (u8*)header + sizeof(*header);
        group->bufferSize += size;
    }
    else
    {
        InvalidCodePath;
    }

    return result;
}

internal void pushRect(RenderGroup *group, SDL_Rect dest, u32 color, RenderLayer layer, u8 alpha)
{
    RenderEntryRect *rect = PushRenderElement(group, RenderEntryRect);
    if (rect)
    {
        rect->dest = dest;
        rect->color = color;
        rect->alpha = alpha;
        rect->layer = layer;
    }
}

internal void pushFilledRect(RenderGroup *group, SDL_Rect dest, u32 color, RenderLayer layer, u8 alpha)
{
    RenderEntryFilledRect *filledRect = PushRenderElement(group, RenderEntryFilledRect);
    if (filledRect)
    {
        filledRect->dest = dest;
        filledRect->color = color;
        filledRect->alpha = alpha;
        filledRect->layer = layer;
    }
}

internal void pushSprite(RenderGroup *group, SDL_Texture *sheet, SDL_Rect source, SDL_Rect dest, RenderLayer layer)
{
    RenderEntrySprite *sprite = PushRenderElement(group, RenderEntrySprite);
    if (sprite)
    {
        sprite->dest = dest;
        sprite->source = source;
        sprite->sheet = sheet;
        sprite->layer = layer;
    }
}

internal RenderGroup *allocateRenderGroup(Arena *arena, u32 maxSize)
{
    RenderGroup *result = PushStruct(arena, RenderGroup);
    result->bufferBase = PushSize(arena, maxSize);
    result->maxBufferSize = maxSize;
    result->bufferSize = 0;

    return result;
}

internal SDL_Rect *checkForNullRect(SDL_Rect *rect)
{
    if (rect->x == 0 && rect->y == 0 && rect->w == 0 && rect->h == 0)
    {
        return NULL;
    }
    return rect;
}

internal void drawRenderGroup(SDL_Renderer *renderer, RenderGroup *group)
{
    for (int layerIndex = 0; layerIndex < RenderLayer_Count; ++layerIndex)
    {
        for (u32 baseAddress = 0; baseAddress < group->bufferSize;)
        {
            RenderEntryHeader *header = (RenderEntryHeader*)(group->bufferBase + baseAddress);
            baseAddress += sizeof(*header);

            void *data = (u8*)header + sizeof(*header);
            switch (header->type)
            {
            case RenderEntryType_RenderEntryRect:
            {
                RenderEntryRect *entry = (RenderEntryRect*)data;
                if (entry->layer == layerIndex)
                {
                    SDL_Rect *dest = checkForNullRect(&entry->dest);
                    renderRect(renderer, dest, entry->color, entry->alpha);
                }
                baseAddress += sizeof(*entry);
            } break;
            case RenderEntryType_RenderEntryFilledRect:
            {
                RenderEntryFilledRect *entry = (RenderEntryFilledRect*)data;
                if (entry->layer == layerIndex)
                {
                    SDL_Rect *dest = checkForNullRect(&entry->dest);
                    renderFilledRect(renderer, dest, entry->color, entry->alpha);
                }
                baseAddress += sizeof(*entry);
            } break;

            case RenderEntryType_RenderEntrySprite:
            {
                RenderEntrySprite *entry = (RenderEntrySprite*)data;
                if (entry->layer == layerIndex)
                {
                    SDL_Rect *source = checkForNullRect(&entry->source);
                    SDL_Rect *dest = checkForNullRect(&entry->dest);
                    SDL_RenderCopy(renderer, entry->sheet, source, dest);
                }
                baseAddress += sizeof(*entry);
            } break;

            default:
                InvalidCodePath;
            }
        }
    }
}

#if 0
internal void sortRenderGroup(RenderGroup *group)
{
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
#endif