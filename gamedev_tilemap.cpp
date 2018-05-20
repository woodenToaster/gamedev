#include "gamedev_tilemap.h"

void tile_init(Tile* t, u32 flags, u32 color, const char* sprite_path=NULL)
{
    t->flags = flags;
    t->color = color;
    t->sprite_rect = {};
    t->active = GD_FALSE;
    t->img_data = NULL;

    if (sprite_path)
    {
        t->sprite = create_surface_from_png(&t->img_data, sprite_path);
    }
    else
    {
        t->sprite = NULL;
    }
}

void tile_set_sprite_size(Tile* t, int width, int height)
{
    t->sprite_rect.w = width;
    t->sprite_rect.h = height;
}

u64 tile_as_u64(Tile* t)
{
    return (u64)t->flags << 32 | t->color;
}

u32 tile_get_flags(Tile* t)
{
    return (u32)(tile_as_u64(t) >> 32);
}

u8 tile_is_fire(Tile* t)
{
    return (u8)(tile_get_flags(t) & tile_properties[TP_FIRE]);
}

u8 tile_is_solid(Tile* t)
{
    return (u8)(tile_get_flags(t) & tile_properties[TP_SOLID]);
}

u8 tile_is_slow(Tile* t)
{
    return (u8)(tile_get_flags(t) & tile_properties[TP_QUICKSAND]);
}

u8 tile_is_warp(Tile* t)
{
    return (u8)(tile_get_flags(t) & tile_properties[TP_WARP]);
}


u32 tile_get_color(Tile* t)
{
    return (u32)(tile_as_u64(t) & 0xFFFFFFFF);
}

void tile_draw(Tile* t, SDL_Surface* map_surface, SDL_Rect* tile_rect)
{
    u32 fill_color = tile_get_color(t);
    SDL_FillRect(map_surface, tile_rect, fill_color);

    if (t->sprite && t->active)
    {
        SDL_Rect dest = *tile_rect;
        dest.x += (t->tile_width - t->sprite_rect.w) / 2;
        dest.y += (t->tile_height - t->sprite_rect.h) / 2;

        if (t->has_animation)
        {
            t->sprite_rect.x = t->sprite_rect.w * t->animation.current_frame;
        }
        SDL_BlitSurface(t->sprite, &t->sprite_rect, map_surface, &dest);
    }
}

void tile_destroy(Tile* t)
{
    if (t->sprite)
    {
        SDL_FreeSurface(t->sprite);
        t->sprite = NULL;
        stbi_image_free(t->img_data);
        t->img_data = NULL;
    }
}

void tile_list_destroy(TileList* tl)
{
    for (u32 i = 0; i < tl->count; ++i)
    {
        tile_destroy(tl->tiles[i]);
    }
}

void map_init(Map* m, u32 cols, u32 rows, Tile** tiles)
{
    m->cols = cols;
    m->rows = rows;
    m->tile_width = tiles[0][0].tile_width;
    m->tile_height = tiles[0][0].tile_height;
    m->tiles = tiles;
    m->width_pixels = cols * tiles[0]->tile_width;
    m->height_pixels = rows * tiles[0]->tile_height;
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
        animation_update(&tp->animation, last_frame_duration, tp->active);
    }
}

void map_draw(Map* m, Camera* c)
{
    // TODO: Only draw tiles that have changed
    size_t first_row = c->viewport.y / m->tile_height;
    size_t last_row = first_row + c->viewport.h / m->tile_height + 1;
    size_t first_col = c->viewport.x / m->tile_width;
    size_t last_col = first_col + c->viewport.w / m->tile_width + 1;

    clamp_size_t(&last_row, last_row, m->rows);
    clamp_size_t(&last_col, last_col, m->cols);

    SDL_Rect tile_rect = {};
    tile_rect.w = m->tile_width;
    tile_rect.h = m->tile_height;
    for (size_t row = first_row; row < last_row; ++row)
    {
        for (size_t col = first_col; col < last_col; ++col)
        {
            tile_rect.x = (int)col * m->tile_width;
            tile_rect.y = (int)row * m->tile_height;
            Tile* tp = m->tiles[row * m->cols + col];
            tile_draw(tp, m->surface, &tile_rect);
        }
    }

    entity_list_draw(&m->active_entities, m->surface);
}

void map_activate_entities(Map* m)
{
    for (u32 i = 0; i < m->active_entities.count; ++i)
    {
        m->active_entities.entities[i]->active = GD_TRUE;
    }
}

void map_deactivate_entities(Map* m)
{
    for (u32 i = 0; i < m->active_entities.count; ++i)
    {
        m->active_entities.entities[i]->active = GD_FALSE;
    }
}

void map_do_warp(MapList* ml, Map** current_map)
{
    // TODO: Make this more generic
    Map* map1 = ml->maps[0];
    Map* map2 = ml->maps[1];
    Map* map3 = ml->maps[2];

    if (map1->current)
    {
        // Transitioning to map2
        *current_map = map2;
        map1->current = GD_FALSE;
        map2->current = GD_TRUE;
        // TODO: Need list of active tiles
        // fire.active = GD_FALSE;

        // TODO: Deactivate all active entties of previous map
        map_deactivate_entities(map1);
        map_activate_entities(map2);
    }
    else if (map2->current)
    {
        // Transitioning to map3
        *current_map = map3;
        map2->current = GD_FALSE;
        map3->current = GD_TRUE;
        // grass->active = GD_TRUE;
        map_deactivate_entities(map2);
        map_activate_entities(map3);
    }
    else if (map3->current)
    {
        // Transitioning to map1
        *current_map = map1;
        map1->current = GD_TRUE;
        map3->current = GD_FALSE;
        // fire.active = GD_TRUE;
        // grass->active = GD_FALSE;
        map_deactivate_entities(map3);
        map_activate_entities(map1);
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
