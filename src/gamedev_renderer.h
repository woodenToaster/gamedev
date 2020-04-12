#ifndef RENDERER_H
#define RENDERER_H

#include "gamedev.h"

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
    RenderEntryType_RenderEntryLoadedBitmap,
    RenderEntryType_RenderEntryTexture,

    RenderEntryType_Count
};

struct RenderEntryHeader
{
    RenderEntryType type;
};

// NOTE(cjh): A Rect of {0, 0, 0, 0} in RenderEntry objects implies the entire texture.
struct RenderEntryRect
{
    Vec4u8 color;
    Rect dest;
    RenderLayer layer;
};

struct RenderEntryFilledRect
{
    Vec4u8 color;
    Rect dest;
    RenderLayer layer;
};

struct RenderEntryLoadedBitmap
{
    LoadedBitmap bitmap;
    Rect source;
    Rect dest;
    RenderLayer layer;
};

struct RenderEntrySprite
{
    Rect source;
    Rect dest;
    TextureHandle sheet;
    RenderLayer layer;
};

struct RenderEntryTexture
{
    Vec2 position;
    Vec2 size;
    f32 spriteWidth;
    f32 spriteHeight;
    f32 sheetWidth;
    f32 sheetHeight;
    int currentFrame;
    Direction direction;
};

struct RenderGroup
{
    u32 maxBufferSize;
    u32 bufferSize;
    u8 *bufferBase;
};

internal void pushRect(RenderGroup *group, Rect dest, Vec4u8 color, RenderLayer layer);
internal void pushFilledRect(RenderGroup *group, Rect dest, Vec4u8 color, RenderLayer layer);
internal void pushSprite(RenderGroup *group, TextureHandle sheet, Rect source, Rect dest,
                         RenderLayer layer);
internal void pushBitmap(RenderGroup *group, LoadedBitmap bitmap, Rect source, Rect dest,
                         RenderLayer layer);
#endif
