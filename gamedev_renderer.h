
enum RenderLayer
{
    RenderLayer_Ground,
    RenderLayer_Entities,
    RenderLayer_HUD,
    RenderLayer_Count
};

enum RenderEntryType
{
    RenderEntryType_RenderEntryRect,
    RenderEntryType_RenderEntryFilledRect,
    RenderEntryType_RenderEntrySprite,
    RenderEntryType_Count
};

struct TextureDims
{
    i32 width;
    i32 height;
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
    RenderLayer layer;
};

struct RenderEntryFilledRect
{
    u32 color;
    SDL_Rect dest;
    u8 alpha;
    RenderLayer layer;
};

struct RenderEntrySprite
{
    SDL_Rect source;
    SDL_Rect dest;
    TextureHandle sheet;
    RenderLayer layer;
};

struct RenderGroup
{
    u32 maxBufferSize;
    u32 bufferSize;
    u8 *bufferBase;
};

internal void pushRect(RenderGroup *group, Rect dest, u32 color, RenderLayer layer, u8 alpha=255);
internal void pushFilledRect(RenderGroup *group, Rect dest, u32 color, RenderLayer layer,  u8 alpha=255);
internal void pushSprite(RenderGroup *group, TextureHandle sheet, Rect source, Rect dest, RenderLayer layer);
