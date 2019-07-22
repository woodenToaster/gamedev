#ifndef GD_TILE_MAP_H
#define GD_TILE_MAP_H

#include "gamedev_entity.h"
struct RenderGroup;

enum TileProperty
{
    TP_NONE = (0x0),
    TP_SOLID = (0x01 << 0),
    TP_QUICKSAND = (0x01 << 2),
    TP_CAMPFIRE = (0x01 << 6),
    TP_HARVEST = (0x01 << 7),
    TP_INTERACTIVE = (0x01 << 8),
    TP_FLAME = (0x01 << 9),
    TP_FLAMMABLE = (0x01 << 10)
};

struct Map
{
    u32 rows;
    u32 cols;
    u32 tileWidth;
    u32 tileHeight;
    int widthPixels;
    int heightPixels;
    TextureHandle texture;
    u32 entityCount;
    Entity entities[256];
};

void drawTile(RenderGroup *group, Game *g, Entity *e, b32 isBeingPlace=false);
Rect getEntityRect(Entity *e);
void addTileFlags(Entity *e, u32 prop);
b32 isTileFlagSet(Entity *e, TileProperty prop);
void removeTileFlags(Entity *e, u32 prop);
#endif
