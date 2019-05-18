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

internal void pushSprite(RenderGroup *group, SDL_Rect dest, SDL_Rect source, SDL_Texture *sheet)
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

internal void drawRenderGroup(SDL_Renderer *renderer, RenderGroup *group)
{
    for (u32 baseAddress = 0; baseAddress < group->bufferSize;)
    {
        RenderEntryHeader *header = (RenderEntryHeader*)(group->bufferBase + baseAddress);
        baseAddress += sizeof(*header);

        void *data = (u8*)header + sizeof(*header);
        switch (header->type)
        {
        case RenderEntryType_RenderEntryFilledRect:
        {
            RenderEntryFilledRect *entry = (RenderEntryFilledRect*)data;
            renderFilledRect(renderer, &entry->dest, entry->color, entry->alpha);
            baseAddress += sizeof(*entry);
        } break;

        case RenderEntryType_RenderEntrySprite:
        {
            RenderEntrySprite *entry = (RenderEntrySprite*)data;
            SDL_RenderCopy(renderer, entry->sheet, &entry->source, &entry->dest);
            baseAddress += sizeof(*entry);
        } break;

        default:
            InvalidCodePath;
        }
    }
}
