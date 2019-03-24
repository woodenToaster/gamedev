#ifndef GD_TILE_MAP_H
#define GD_TILE_MAP_H

#include "gamedev_entity.h"

struct Tile;

enum TileProperty
{
    TP_NONE,
    TP_SOLID,
    TP_WATER,
    TP_QUICKSAND,
    TP_STICKY,
    TP_REVERSE,
    TP_WARP,
    TP_FIRE,
    TP_HARVEST,
    TP_INTERACTIVE,
    TP_COUNT
};

// TODO(chj): Set values here on enum above
static u32 tile_properties[TP_COUNT] = {
    // Designated initializer syntax only available in C
    /* [TP_NONE] = 0x0, */
    /* [TP_SOLID] = 0x01 << 0, */
    /* [TP_WATER] = 0x01 << 1, */
    /* [TP_QUICKSAND] = 0x01 << 2, */
    /* [TP_STICKY] = 0x01 << 3, */
    /* [TP_REVERSE] = 0x01 << 4, */
    /* [TP_WARP] = 0x01 << 5, */
    /* [TP_FIRE] = 0x01 << 6, */
    /* [TP_HARVEST] = 0x01 << 7, */
    /* [TP_INTERACTIVE] = 0x01 << 8, */
    0x0,
    0x01 << 0,
    0x01 << 1,
    0x01 << 2,
    0x01 << 3,
    0x01 << 4,
    0x01 << 5,
    0x01 << 6,
    0x01 << 7,
    0x01 << 8,
};

typedef void (*heroTileInteractionFunc)(Tile *t, Hero *h);

struct Tile
{
    u32 tile_width;
    u32 tile_height;
    u32 flags;
    u32 color;
    SDL_Texture* sprite;
    SDL_Rect sprite_rect;
    Animation animation;
    bool32 active;
    bool32 has_animation;
    bool32 animation_is_active;
    u32 destination_map;
    bool32 is_harvestable;
    bool32 harvested;
    InventoryItemType harvestedItem;
    heroTileInteractionFunc onHeroInteract;
};

struct TileList
{
    Tile** tiles;
    u32 count;
};

struct Tileset
{
    Tile tiles[462];
    SDL_Texture* texture;
};

struct Map
{
    u32 rows;
    u32 cols;
    u32 tile_width;
    u32 tile_height;
    int width_pixels;
    int height_pixels;
    u64 tile_rows_per_screen;
    u64 tile_cols_per_screen;
    u8 current;
    Tile** tiles;
    SDL_Texture* texture;
    EntityList active_entities;
    TileList active_tiles;
};

struct MapList
{
    Map** maps;
    u32 count;
};

void initTile(Tile* t, u32 flags, u32 color, SDL_Renderer* renderer, const char* sprite_path=NULL);
void destroyTile(Tile* t);
void setTileSpriteSize(Tile* t, int width, int height);
bool32 tile_is_interactive(Tile *t);
Tile *getTileAtPosition(Map *m, Vec2 pos);
Tile *map_get_tile_at_point(Map *m, Point p);
bool32 isSolidTile(Tile *t);
bool32 isSlowTile(Tile* t);
bool32 isWarpTile(Tile* t);
void map_do_warp(Game* game);
#endif
