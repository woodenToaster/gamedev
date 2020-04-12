
// TODO(cjh): Account for wrapping off the viewport
void drawText(RenderCommands *commands, FontMetadata *fontMetadata, char* text, i32 x=0, i32 y=0)
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
        Rect source = {cpm->x0, cpm->y0, width, height};
        Rect dest = {xpos, baseline + cpm->y0, width, height};

	    if (text[at] != ' ')
        {
            TextureHandle t = {};
            t.texture = fontMetadata->textures[text[at]].texture;
            Rect fullTexture = {0, 0, 0, 0};
            pushSprite(commands, t, fullTexture, dest, RenderLayer_HUD);
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
            int toAdvance = fontAPI.getKernAdvancement(fontMetadata->info, text[at], text[at + 1]);
            xpos += (int)(fontMetadata->scale * toAdvance);
        }
        ++at;
    }
}

#if 0
void drawDialogScreen(RenderCommands *commands, Game *g, FontMetadata *fontMetadata)
{
    int thirdOfWidth = (int)(g->camera.viewport.w / 3);
    int fourthOfHeight = (int)(g->camera.viewport.h / 4);
    int dialogueBoxX = (int)(0.5 * (thirdOfWidth)) + g->camera.viewport.x;
    int dialogueBoxY = (int)((3 * (fourthOfHeight)) - 0.5 * fourthOfHeight) + g->camera.viewport.y;
    int dialogueBoxWidth = 2 * (thirdOfWidth);
    int dialogueBoxHeight = fourthOfHeight;
    Rect dialogueBoxDest = {dialogueBoxX,dialogueBoxY, dialogueBoxWidth, dialogueBoxHeight};
    pushFilledRect(commands, dialogueBoxDest, g->colors[Color_BabyBlue], RenderLayer_HUD);
    drawText(commands, fontMetadata, g->dialogue, dialogueBoxX, dialogueBoxY);
}

void drawInventoryScreen(RenderCommands*commands, Game *g, Entity *h, FontMetadata *fontMetadata)
{
    int thirdOfWidth = (int)(g->camera.viewport.w / 3);
    int fourthOfHeight = (int)(g->camera.viewport.h / 4);
    int dialogueBoxX = (int)(0.5 * thirdOfWidth) + g->camera.viewport.x;
    int dialogueBoxY = (int)(0.5 * fourthOfHeight) + g->camera.viewport.y;
    int dialogueBoxWidth = 2 * (thirdOfWidth);
    int dialogueBoxHeight = fourthOfHeight;
    Rect dialogueBoxDest = {dialogueBoxX, dialogueBoxY, dialogueBoxWidth, dialogueBoxHeight};
    pushFilledRect(commands, dialogueBoxDest, g->colors[Color_BabyBlue], RenderLayer_HUD);

    for (int inventoryIndex = 1; inventoryIndex < InventoryItemType_Count; ++inventoryIndex)
    {
        char itemString[30] = {};
        switch ((InventoryItemType)inventoryIndex)
        {
            case InventoryItemType_Leaves:
                snprintf(itemString, 30, "Leaves: %d", h->inventory[InventoryItemType_Leaves]);
                break;
            case InventoryItemType_Glow:
                snprintf(itemString, 30, "Glow: %d", h->inventory[InventoryItemType_Glow]);
                break;
            default:
                assert(!"Wrong inventory type");
        }
        drawText(commands, fontMetadata, itemString, dialogueBoxX, dialogueBoxY);
        // TODO(cjh): Figure out correct y offset for next line
        dialogueBoxY += 25;
    }
}

void drawHUD(RenderCommands *commands, Game *g, Entity *h, FontMetadata *font)
{
    u8 beltSlots = 8;
    u8 slotSize = 40;
    i32 slotCenterX = g->camera.viewport.w / 2;
    i32 destX = slotCenterX - ((beltSlots / 2) * slotSize);
    i32 destY = g->camera.viewport.h + g->camera.viewport.y - slotSize;

    // Transparent black background
    Rect backgroundDest = {destX, destY, beltSlots * slotSize, slotSize};
    Vec4u8 transparentBlack = g->colors[Color_Black];
    transparentBlack.a = 128;
    pushFilledRect(commands, backgroundDest, transparentBlack, RenderLayer_HUD);

    for (int i = 0; i < beltSlots; ++i)
    {
        Rect dest = {destX + i*slotSize, destY, slotSize, slotSize};
        if ((u32)i < h->beltItemCount)
        {
            BeltItem *item = &h->beltItems[i];
            TextureHandle textureToDraw = {};
            Rect tileRect = {};

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

            if (textureToDraw.texture)
            {
                pushSprite(commands, textureToDraw, tileRect, dest, RenderLayer_HUD);
            }
            // Draw inventory number
            assert(item->count <= 999);
            char numItems[4] = {};
            snprintf(numItems, 4, "%d", item->count);
            drawText(commands, font, numItems, dest.x, dest.y);
        }
        Vec4u8 inventoryOutlineColor = g->colors[Color_White];
        inventoryOutlineColor.a = i == h->activeBeltItemIndex ? 255 : 12;
        pushRect(commands, dest, g->colors[Color_White], RenderLayer_HUD);
    }
}
#endif

void darkenBackground(RenderCommands*commands, Game *g)
{
    Rect dest = {};
    Vec4u8 transparentBlack = g->colors[Color_Black];
    transparentBlack.a = 64;
    pushFilledRect(commands, dest, transparentBlack, RenderLayer_HUD);
}

#define PushRenderElement(commands, type) (type*)pushRenderElement_(commands, sizeof(type), \
    RenderEntryType_##type)

internal void *pushRenderElement_(RenderCommands *commands, u32 size, RenderEntryType type)
{
    void *result = 0;

    size += sizeof(RenderEntryHeader);

    if (commands->bufferSize + size < commands->maxBufferSize)
    {
        RenderEntryHeader *header = (RenderEntryHeader*)(commands->bufferBase + commands->bufferSize);
        header->type = type;
        result = (u8*)header + sizeof(*header);
        commands->bufferSize += size;
    }
    else
    {
        InvalidCodePath;
    }

    return result;
}

internal void pushRect(RenderCommands *commands, Rect dest, Vec4u8 color, RenderLayer layer)
{
    RenderEntryRect *rect = PushRenderElement(commands, RenderEntryRect);
    if (rect)
    {
        Rect sdlDest = {dest.x, dest.y, dest.w, dest.h};
        rect->dest = sdlDest;
        rect->color = color;
        rect->layer = layer;
    }
}

internal void pushFilledRect(RenderCommands *commands, Rect dest, Vec4u8 color, RenderLayer layer)
{
    RenderEntryFilledRect *filledRect = PushRenderElement(commands, RenderEntryFilledRect);
    if (filledRect)
    {
        filledRect->dest = {vec2((f32)dest.x, (f32)dest.y),
                            vec2((f32)dest.x + dest.w, (f32)dest.y + dest.h)};
        filledRect->color = color;
        filledRect->layer = layer;
    }
}

internal void pushFilledRect(RenderCommands *commands, Rect2 dest, Vec4u8 color, RenderLayer layer)
{
    RenderEntryFilledRect *filledRect = PushRenderElement(commands, RenderEntryFilledRect);
    if (filledRect)
    {
        filledRect->dest = dest;
        filledRect->color = color;
        filledRect->layer = layer;
    }
}

internal void pushSprite(RenderCommands *commands, TextureHandle sheet, Rect source, Rect dest, RenderLayer layer)
{
    RenderEntrySprite *sprite = PushRenderElement(commands, RenderEntrySprite);
    if (sprite)
    {
        Rect sdlDest = {dest.x, dest.y, dest.w, dest.h};
        Rect sdlSource = {source.x, source.y, source.w, source.h};
        sprite->dest = sdlDest;
        sprite->source = sdlSource;
        sprite->sheet = sheet;
        sprite->layer = layer;
    }
}

#if 0
internal void pushBitmap(RenderCommands *commands, LoadedBitmap bitmap, Rect source, Rect dest,
                         RenderLayer layer)
{
    RenderEntryLoadedBitmap *bitmap_entry = PushRenderElement(commands, RenderEntryLoadedBitmap);
    if (bitmap_entry)
    {
        bitmap_entry->bitmap = bitmap;
        bitmap_entry->source = source;
        bitmap_entry->dest = dest;
        bitmap_entry->layer = layer;
    }
}

internal void pushSprite(RenderCommands *commands, Sprite *sprite, Rect dest, RenderLayer layer)
{
    RenderEntrySprite *entry = PushRenderElement(commands, RenderEntrySprite);
    if (entry && sprite)
    {
        Rect sdlDest = {dest.x, dest.y, dest.w, dest.h};
        Rect sdlSource = {sprite->x, sprite->y, sprite->width, sprite->height};
        entry->dest = sdlDest;
        entry->source = sdlSource;
        entry->sheet = sprite->sheet.sheet;
        entry->layer = layer;
    }
}
#endif

#if 0
internal RenderCommands *allocateRenderGroup(Arena *arena, u32 maxSize)
{
    RenderCommands *result = PUSH_STRUCT(arena, RenderCommands);
    result->bufferBase = pushSize(arena, maxSize);
    result->maxBufferSize = maxSize;
    result->bufferSize = 0;

    return result;
}

internal void drawRenderGroup(void *renderer, RenderCommands *group)
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
                        rendererAPI.renderRect(renderer, entry->dest, entry->color);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                case RenderEntryType_RenderEntryFilledRect:
                {
                    RenderEntryFilledRect *entry = (RenderEntryFilledRect*)data;
                    if (entry->layer == layerIndex)
                    {
                        rendererAPI.renderFilledRect(renderer, entry->dest, entry->color);
                    }
                    baseAddress += sizeof(*entry);
                } break;

                case RenderEntryType_RenderEntrySprite:
                {
                    RenderEntrySprite *entry = (RenderEntrySprite*)data;
                    if (entry->layer == layerIndex)
                    {
                        rendererAPI.renderSprite(renderer, entry->sheet, entry->source, entry->dest);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                case RenderEntryType_RenderEntryLoadedBitmap:
                {
                    RenderEntryLoadedBitmap *entry = (RenderEntryLoadedBitmap *)data;
                    if (entry->layer == layerIndex)
                    {
                        rendererAPI.renderBitmap(renderer, entry->bitmap, entry->source, entry->dest);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                default:
                    InvalidCodePath;
            }
        }
    }
}

internal void sortRenderGroup(RenderCommands *group)
{
}
#endif
