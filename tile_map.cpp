#include "tile_map.h"

Uint64 Tile::as_u64() {
    return (Uint64)flags << 32 | color;
}

bool Tile::is_solid() {
    Uint32 f = (Uint32)(this->as_u64() >> 32);
    return f & SOLID;
}

bool Tile::is_slow() {
    Uint32 f = (Uint32)(this->as_u64() >> 32);
    return f & QUICKSAND;
}

bool Tile::is_warp() {
    Uint32 f = (Uint32)(this->as_u64() >> 32);
    return f & WARP;
}

Uint32 Tile::get_color() {
    return (Uint32)(this->as_u64() & 0xFFFFFFFF);
}
