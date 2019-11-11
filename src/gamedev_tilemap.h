#ifndef GD_TILE_MAP_H
#define GD_TILE_MAP_H

#include "gamedev_entity.h"
struct RenderGroup;

enum TileProperty
{
    TileProperty_None = (0x0),
    TileProperty_Solid = (0x01 << 0),
    TileProperty_Quicksand = (0x01 << 1),
    TileProperty_Campfire = (0x01 << 2),
    TileProperty_Harvest = (0x01 << 3),
    TileProperty_Interactive = (0x01 << 4),
    TileProperty_Flame = (0x01 << 5),
    TileProperty_Flammable = (0x01 << 6)
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
void drawTiles(RenderGroup *group, Game *g);
void updateTiles(Game *g);
void drawBackground(RenderGroup *group, Game *g);
Rect getEntityRect(Entity *e);
inline internal void addTileFlags(Entity *e, u32 prop);
inline internal b32 isTileFlagSet(Entity *e, TileProperty prop);
inline internal void removeTileFlags(Entity *e, u32 prop);
#endif
