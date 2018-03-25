#ifndef TILE_MAP_H
#define TILE_MAP_H

struct Tile {
    static const Uint32 NONE = 0x0;
    static const Uint32 SOLID = 0x01;
    static const Uint32 WATER = 0x01 << 1;
    static const Uint32 QUICKSAND = 0x01 << 2;
    static const Uint32 STICKY = 0x01 << 3;
    static const Uint32 REVERSE = 0x01 << 4;
    static const Uint32 WARP = 0x01 << 5;

    Uint32 flags;
    Uint32 color;
    SDL_Surface* sprite;

    bool is_solid();
    bool is_slow();
    bool is_warp();
    Uint32 get_color();
    Uint64 as_u64();
};

struct TileMap {

};

#endif
