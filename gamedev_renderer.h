
enum RenderLayer
{
    RenderLayer_Ground,
    RenderLayer_Entities,
    RenderLayer_HUD
};

enum RenderEntryType
{
    RenderEntryType_RenderEntryRect,
    RenderEntryType_RenderEntryFilledRect,
    RenderEntryType_RenderEntrySprite
};

struct RenderEntryHeader
{
    RenderEntryType type;
};

// NOTE(cjh): An SDL_Rect of {0, 0, 0, 0} in RenderEntry objects implies the entire texture.
// i.e., pass NULL to the SDL_Render call.
struct RenderEntryRect
{
    u32 color;
    SDL_Rect dest;
    u8 alpha;
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

internal void pushRect(RenderGroup *group, SDL_Rect dest, u32 color, u8 alpha=255);
internal void pushFilledRect(RenderGroup *group, SDL_Rect dest, u32 color, u8 alpha=255);
internal void pushSprite(RenderGroup *group, SDL_Texture *sheet, SDL_Rect source, SDL_Rect dest);
