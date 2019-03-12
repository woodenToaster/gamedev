#include "gamedev_tilemap.h"

void initTile(Tile* t, u32 flags, u32 color, SDL_Renderer* renderer, const char* sprite_path)
{
    t->flags = flags;
    t->color = color;
    t->sprite_rect = {};
    t->active = GD_FALSE;
    t->img_data = NULL;

    if (sprite_path)
    {
        SDL_Surface* sprite_surface = create_surface_from_png(&t->img_data, sprite_path);
        t->sprite = SDL_CreateTextureFromSurface(renderer, sprite_surface);

        if (!t->sprite)
        {
            fprintf(stderr, "Could not create texture from surface: %s\n", SDL_GetError());
            exit(1);
        }

        SDL_FreeSurface(sprite_surface);
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

void tile_fill(Tile* t, SDL_Renderer* renderer, SDL_Rect* tile_rect)
{
    u32 fill_color = tile_get_color(t);
    renderFilledRect(renderer, tile_rect, fill_color);
}

void drawTile(Tile* t, SDL_Renderer* renderer, SDL_Rect* tile_rect)
{
    tile_fill(t, renderer, tile_rect);

    if (t->sprite && t->active)
    {
        SDL_Rect dest = *tile_rect;
        // dest.x += (t->tile_width - t->sprite_rect.w) / 2;
        // dest.y += (t->tile_height - t->sprite_rect.h) / 2;

        if (t->has_animation)
        {
            t->sprite_rect.x = t->sprite_rect.w * t->animation.current_frame;
        }
        SDL_RenderCopy(renderer, t->sprite, &t->sprite_rect, &dest);
    }
}

void destroyTile(Tile* t)
{
    if (t->sprite)
    {
        SDL_DestroyTexture(t->sprite);
        t->sprite = NULL;
        stbi_image_free(t->img_data);
        t->img_data = NULL;
    }
}

void destroyTileList(TileList* tl)
{
    for (u32 i = 0; i < tl->count; ++i)
    {
        destroyTile(tl->tiles[i]);
    }
}

void destroyTileset(Tileset* ts)
{
    SDL_DestroyTexture(ts->texture);
    stbi_image_free(ts->img_data);
}

void initMap(Map* m, u32 cols, u32 rows, Tile** tiles, SDL_Renderer* renderer)
{
    m->cols = cols;
    m->rows = rows;
    m->tile_width = tiles[0][0].tile_width;
    m->tile_height = tiles[0][0].tile_height;
    m->tiles = tiles;
    m->width_pixels = cols * tiles[0]->tile_width;
    m->height_pixels = rows * tiles[0]->tile_height;

    m->texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_TARGET,
        m->width_pixels,
        m->height_pixels
    );

    if (!m->texture)
    {
        printf("Failed to create surface: %s\n", SDL_GetError());
        exit(1);
    }
}

Tile *map_get_tile_at_point(Map *m, Point p)
{
    SDL_Rect tile_at_point = {
        (int)((p.x / m->tile_width) * m->tile_width),
        (int)((p.y / m->tile_height) * m->tile_height)
    };
    int map_coord_x = tile_at_point.y / m->tile_height;
    int map_coord_y = tile_at_point.x / m->tile_width;
    int tile_index = map_coord_x * m->cols + map_coord_y;

    return m->tiles[tile_index];
}

void map_update_tiles(Game* g)
{
    Map* m = g->current_map;
    for (size_t i = 0; i < m->rows * m->cols; ++i)
    {
        Tile* tp = m->tiles[i];
        updateAnimation(&tp->animation, g->dt, tp->active);
    }
}

void drawMap(Game* g)
{
    Map* m = g->current_map;
    Camera* c = &g->camera;

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
            drawTile(tp, g->renderer, &tile_rect);
        }
    }

    drawEntityList(&m->active_entities, g);
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

void map_activate_tiles(Map* m)
{
    for (u32 i = 0; i < m->active_tiles.count; ++i)
    {
        m->active_tiles.tiles[i]->active = GD_TRUE;
    }
}

void map_deactivate_tiles(Map* m)
{
    for (u32 i = 0; i < m->active_tiles.count; ++i)
    {
        m->active_tiles.tiles[i]->active = GD_FALSE;
    }
}

void map_do_warp(Game* g)
{
    // TODO: Make this more generic
    Map* map1 = g->maps->maps[0];
    Map* map2 = g->maps->maps[1];
    Map* map3 = g->maps->maps[2];

    if (map1->current)
    {
        // Transitioning to map2
        g->current_map = map2;
        map1->current = GD_FALSE;
        map2->current = GD_TRUE;
        map_deactivate_entities(map1);
        map_deactivate_tiles(map1);
        map_activate_entities(map2);
        map_activate_tiles(map2);
    }
    else if (map2->current)
    {
        // Transitioning to map3
        g->current_map = map3;
        map2->current = GD_FALSE;
        map3->current = GD_TRUE;
        map_deactivate_entities(map2);
        map_activate_entities(map3);
        map_deactivate_tiles(map2);
        map_activate_tiles(map3);
    }
    else if (map3->current)
    {
        // Transitioning to map1
        g->current_map = map1;
        map1->current = GD_TRUE;
        map3->current = GD_FALSE;
        map_deactivate_entities(map3);
        map_activate_entities(map1);
        map_deactivate_tiles(map3);
        map_activate_tiles(map1);
    }
}

void map_destroy(Map* m)
{
    SDL_DestroyTexture(m->texture);
}

void map_list_destroy(MapList* ml)
{
    for (u32 i = 0; i < ml->count; ++i)
    {
        map_destroy(ml->maps[i]);
    }
}

