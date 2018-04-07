#include "tile_map.h"

void Animation::update(Uint32 elapsed_last_frame)
{
    elapsed += elapsed_last_frame;
    if (elapsed > delay) {
        current_frame++;
        if (current_frame > total_frames - 1) {
            current_frame = 0;
        }
        elapsed = 0;
    }
}

Tile::Tile(Uint32 flags, Uint32 color, const char* sprite_path):
    flags(flags),
    color(color),
    sprite_rect(),
    active(false)
{
    if (sprite_path)
    {
        sprite = IMG_Load(sprite_path);

        if (sprite == NULL)
        {
            printf("Couldn't load %s: %s\n", sprite_path, SDL_GetError());
            exit(1);
        }
    }
    else
    {
        sprite = NULL;
    }
}

Tile::~Tile()
{
    // TODO: Figure out crash 
    // if (sprite)
    // {
    //     SDL_FreeSurface(sprite);
    //     sprite = NULL;
    // }
}

void Tile::set_sprite_size(int width, int height)
{
    sprite_rect.w = width;
    sprite_rect.h = height;
}

Uint64 Tile::as_u64()
{
    return (Uint64)flags << 32 | color;
}

bool Tile::is_fire()
{
    return get_flags() & FIRE;
}

bool Tile::is_solid()
{
    return get_flags() & SOLID;
}

bool Tile::is_slow()
{
    return get_flags() & QUICKSAND;
}

bool Tile::is_warp()
{
    return get_flags() & WARP;
}

Uint32 Tile::get_flags()
{
    return (Uint32)(this->as_u64() >> 32);
}

Uint32 Tile::get_color()
{
    return (Uint32)(this->as_u64() & 0xFFFFFFFF);
}

void Tile::draw(SDL_Surface* map_surface, SDL_Rect* tile_rect)
{
    Uint32 fill_color = get_color();
    SDL_FillRect(map_surface, tile_rect, fill_color);

    if (sprite && active)
    {
        SDL_Rect dest = {tile_rect->x, tile_rect->y, 64, 64};
        // TODO: Hard coded tile dimensions
        dest.x += (80 - sprite_rect.w) / 2;
        dest.y += (80 - sprite_rect.h) / 2;

        sprite_rect.x = sprite_rect.w * animation.current_frame;
        SDL_BlitSurface(sprite, &sprite_rect, map_surface, &dest);
    }
}


Map::Map(int rows, int cols, Tile** tiles):
    rows(rows),
    cols(cols),
    tiles(tiles),
    is_active(false)
{
    width_pixels = cols * Tile::tile_width;
    height_pixels = rows * Tile::tile_height;
}
