#include "gamedev_tilemap.h"


Tile::Tile(Uint32 flags, Uint32 color, const char* sprite_path):
    flags(flags),
    color(color),
    sprite_rect(),
    active(false),
    img_data(NULL)
{
    if (sprite_path)
    {
        sprite = create_surface_from_png(&img_data, sprite_path);
    }
    else
    {
        sprite = NULL;
    }
}

Tile::~Tile()
{
    if (sprite)
    {
        SDL_FreeSurface(sprite);
        sprite = NULL;
        stbi_image_free(img_data);
        img_data = NULL;
    }
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

void map_init(Map* m, u32 cols, u32 rows, Tile** tiles)
{
    m->cols = cols;
    m->rows = rows;
    m->tiles = tiles;
    m->width_pixels = cols * Tile::tile_width;
    m->height_pixels = rows * Tile::tile_height;
    m->surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        m->width_pixels,
        m->height_pixels,
        32,
        SDL_PIXELFORMAT_RGB888
    );
    if (!m->surface)
    {
        printf("Failed to create surface: %s\n", SDL_GetError());
        exit(1);
    }
}

void map_update_tiles(Map* m, u32 last_frame_duration)
{
    for (size_t i = 0; i < m->rows * m->cols; ++i)
    {
        Tile* tp = m->tiles[i];
        animation_update(&tp->animation, last_frame_duration);
    }
}

void map_destroy(Map* m)
{
    SDL_FreeSurface(m->surface);
}

void map_list_destroy(MapList* ml)
{
    for (u32 i = 0; i < ml->count; ++i)
    {
        map_destroy(ml->maps[i]);
    }
}
