#ifndef GD_TILE_MAP_H
#define GD_TILE_MAP_H

#include "gamedev_entity.h"

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
    TP_LAST
};

static u32 tile_properties[TP_LAST] = {
    // Designated initializer syntax only available in C
    /* [TP_NONE] = 0x0, */
    /* [TP_SOLID] = 0x01 << 0, */
    /* [TP_WATER] = 0x01 << 1, */
    /* [TP_QUICKSAND] = 0x01 << 2, */
    /* [TP_STICKY] = 0x01 << 3, */
    /* [TP_REVERSE] = 0x01 << 4, */
    /* [TP_WARP] = 0x01 << 5, */
    /* [TP_FIRE] = 0x01 << 6, */
    0x0,
    0x01 << 0,
    0x01 << 1,
    0x01 << 2,
    0x01 << 3,
    0x01 << 4,
    0x01 << 5,
    0x01 << 6,
};

struct Tile
{
    u32 tile_width;
    u32 tile_height;
    u32 flags;
    u32 color;
    SDL_Surface* sprite;
    SDL_Rect sprite_rect;
    Animation animation;
    u8 active;
    u8 has_animation;
    unsigned char* img_data;
    u32 destination_map;
};

struct TileList
{
    Tile** tiles;
    u32 count;
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
    SDL_Surface* surface;
    EntityList active_entities;
    TileList active_tiles;
};

struct MapList
{
    Map** maps;
    u32 count;
};

#endif
