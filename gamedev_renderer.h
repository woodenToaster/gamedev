
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

// NOTE(cjh): A Rect of {0, 0, 0, 0} in RenderEntry objects implies the entire texture.
struct RenderEntryRect
{
    u32 color;
    Rect dest;
    u8 alpha;
    RenderLayer layer;
};

struct RenderEntryFilledRect
{
    u32 color;
    Rect dest;
    u8 alpha;
    RenderLayer layer;
};

struct RenderEntrySprite
{
    Rect source;
    Rect dest;
    TextureHandle sheet;
    RenderLayer layer;
};

struct RenderGroup
{
    u32 maxBufferSize;
    u32 bufferSize;
    u8 *bufferBase;
};

struct RendererHandle
{
    void *renderer;
};

typedef TextureDims (GetTextureDims)(TextureHandle texture);
typedef void (DestroyTexture)(TextureHandle t);
typedef void (SetRenderDrawColor)(RendererHandle renderer, u32 color);
typedef void (RenderRect)(RendererHandle renderer, Rect dest, u32 color, u8 alpha);
typedef void (RenderFilledRect)(RendererHandle renderer, Rect dest, u32 color, u8 alpha);
typedef void (RenderSprite)(RendererHandle renderer, TextureHandle texture, Rect source, Rect dest);

struct RendererAPI
{
    GetTextureDims *getTextureDims;
    DestroyTexture *destroyTexture;
    SetRenderDrawColor *setRenderDrawColor;
    RenderRect *renderRect;
    RenderFilledRect *renderFilledRect;
    RenderSprite *renderSprite;
};

internal void pushRect(RenderGroup *group, Rect dest, u32 color, RenderLayer layer, u8 alpha=255);
internal void pushFilledRect(RenderGroup *group, Rect dest, u32 color, RenderLayer layer,  u8 alpha=255);
internal void pushSprite(RenderGroup *group, TextureHandle sheet, Rect source, Rect dest, RenderLayer layer);
