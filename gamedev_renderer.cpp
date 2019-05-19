
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
    // Color_None is 0. Set a tile's background color to Color_None to avoid
    // extra rendering.
    if (color)
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

internal void pushRect(RenderGroup *group, SDL_Rect dest, u32 color, u8 alpha)
{
    RenderEntryRect *rect = PushRenderElement(group, RenderEntryRect);
    if (rect)
    {
        rect->dest = dest;
        rect->color = color;
        rect->alpha = alpha;
    }
}

internal void pushFilledRect(RenderGroup *group, SDL_Rect dest, u32 color, u8 alpha)
{
    RenderEntryFilledRect *filledRect = PushRenderElement(group, RenderEntryFilledRect);
    if (filledRect)
    {
        filledRect->dest = dest;
        filledRect->color = color;
        filledRect->alpha = alpha;
    }
}

internal void pushSprite(RenderGroup *group, SDL_Texture *sheet, SDL_Rect source, SDL_Rect dest)
{
    RenderEntrySprite *sprite = PushRenderElement(group, RenderEntrySprite);
    if (sprite)
    {
        sprite->dest = dest;
        sprite->source = source;
        sprite->sheet = sheet;
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
            SDL_Rect *dest = checkForNullRect(&entry->dest);
            renderRect(renderer, dest, entry->color, entry->alpha);
            baseAddress += sizeof(*entry);
        } break;
        case RenderEntryType_RenderEntryFilledRect:
        {
            RenderEntryFilledRect *entry = (RenderEntryFilledRect*)data;
            SDL_Rect *dest = checkForNullRect(&entry->dest);
            renderFilledRect(renderer, dest, entry->color, entry->alpha);
            baseAddress += sizeof(*entry);
        } break;

        case RenderEntryType_RenderEntrySprite:
        {
            RenderEntrySprite *entry = (RenderEntrySprite*)data;
            SDL_Rect *source = checkForNullRect(&entry->source);
            SDL_Rect *dest = checkForNullRect(&entry->dest);
            SDL_RenderCopy(renderer, entry->sheet, source, dest);
            baseAddress += sizeof(*entry);
        } break;

        default:
            InvalidCodePath;
        }
    }
}

// internal void sortRenderGroup(RenderGroup *group)
// {
// }
