
enum RenderEntryType
{
    RenderEntryType_RenderEntryFilledRect,
    RenderEntryType_RenderEntrySprite
};

struct RenderEntryHeader
{
    RenderEntryType type;
};

struct RenderEntryFilledRect
{
    u32 color;
    SDL_Rect dest;
    u8 alpha;
};

struct RenderEntrySprite
{
    SDL_Rect source;
    SDL_Rect dest;
    SDL_Texture *sheet;
};

struct RenderGroup
{
    u32 maxBufferSize;
    u32 bufferSize;
    u8 *bufferBase;
};

internal void pushFilledRect(RenderGroup *group, SDL_Rect dest, u32 color, u8 alpha);
internal void pushSprite(RenderGroup *group, SDL_Rect dest, SDL_Rect source, SDL_Texture *sheet);
