#include "gamedev_tilemap.h"

void initTile(Tile* t, u32 flags, u32 color, SDL_Renderer* renderer, const char* sprite_path)
{
    t->flags = flags;
    t->color = color;
    t->sprite_rect = {};

    if (sprite_path)
    {
        t->sprite = create_texture_from_png(sprite_path, renderer);

        if (!t->sprite)
        {
            fprintf(stderr, "Could not create texture from surface: %s\n", SDL_GetError());
            exit(1);
        }
    }
}

void setTileSpriteSize(Tile* t, int width, int height)
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

bool32 isSolidTile(Tile* t)
{
    return tile_get_flags(t) & tile_properties[TP_SOLID];
}

bool32 isSlowTile(Tile* t)
{
    return (u8)(tile_get_flags(t) & tile_properties[TP_QUICKSAND]);
}

bool32 isTileFlagSet(Entity *e, TileProperty prop)
{
    bool32 result = e->tileFlags & tile_properties[prop];
    return result;
}

bool32 isWarpTile(Tile* t)
{
    return (u8)(tile_get_flags(t) & tile_properties[TP_WARP]);
}

bool32 tile_is_interactive(Tile *t)
{
    return tile_get_flags(t) & tile_properties[TP_INTERACTIVE];
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

        if (t->has_animation && t->animation_is_active)
        {
            t->sprite_rect.x = t->sprite_rect.w * t->animation.current_frame;
        }
        SDL_RenderCopy(renderer, t->sprite, &t->sprite_rect, &dest);
    }
}

void lightFire(Tile *t, Hero *h)
{
    (void)h;
    t->animation_is_active = true;
}

void destroyTile(Tile* t)
{
    if (t->sprite)
    {
        SDL_DestroyTexture(t->sprite);
        t->sprite = NULL;
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
}

void initMap(Map* m, u32 cols, u32 rows, Tile** tiles, SDL_Renderer* renderer)
{
    m->cols = cols;
    m->rows = rows;
    m->tile_width = tiles[0][0].width;
    m->tile_height = tiles[0][0].height;
    m->tiles = tiles;
    m->width_pixels = cols * tiles[0]->width;
    m->height_pixels = rows * tiles[0]->height;

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

Tile *getTileAtPosition(Map *m, Vec2 pos)
{
    Tile *result = 0;
    if (m)
    {
        SDL_Rect tile_at_point = {
            (int)((pos.x / m->tile_width) * m->tile_width),
            (int)((pos.y / m->tile_height) * m->tile_height)
        };
        int map_coord_x = tile_at_point.y / m->tile_height;
        int map_coord_y = tile_at_point.x / m->tile_width;
        int tile_index = map_coord_x * m->cols + map_coord_y;

        if (tile_index >= 0 && tile_index < (int)(m->rows * m->cols))
        {
            result = m->tiles[tile_index];
        }
    }
    return result;
}

Tile *map_get_tile_at_point(Map *m, Point p)
{
    Tile *result = 0;
    if (m && p.x >= 0 && p.y >= 0)
    {
        SDL_Rect tile_at_point = {
            (int)((p.x / m->tile_width) * m->tile_width),
            (int)((p.y / m->tile_height) * m->tile_height)
        };
        int map_coord_x = tile_at_point.y / m->tile_height;
        int map_coord_y = tile_at_point.x / m->tile_width;
        int tile_index = map_coord_x * m->cols + map_coord_y;

        if (tile_index >= 0 && tile_index < (int)(m->rows * m->cols))
        {
            result = m->tiles[tile_index];
        }
    }
    return result;
}

#if 0
void updateMap(Game* g)
{
    Map* m = g->current_map;
    for (size_t i = 0; i < m->rows * m->cols; ++i)
    {
        Tile* tp = m->tiles[i];
        updateAnimation(&tp->animation, g->dt, tp->active);
    }

    updateEntityList(g);
}
#endif

void drawMap(Game* g)
{
    Map* map = g->current_map;
    Camera* c = &g->camera;

    // Draw background
    SDL_Rect dest = {c->viewport.x, c->viewport.y, c->viewport.w, c->viewport.h};
    renderFilledRect(g->renderer, &dest, g->colors[COLOR_BLUE]);

    // Draw tile entities
    for (u32 entityIndex = 0; entityIndex < map->entityCount; ++entityIndex)
    {
        Entity *e = &map->entities[entityIndex];
        if (e->type == ET_TILE)
        {
            SDL_Rect tileRect = {(int)(e->position.x - 0.5f*e->width),
                                 (int)(e->position.y - 0.5f*e->height),
                                 (int)e->width, (int)e->height};
            u32 tileColor = e->color;
            renderFilledRect(g->renderer, &tileRect, tileColor);
        }
    }
    drawEntityList(g);
}

void map_activate_entities(Map* m)
{
    for (u32 i = 0; i < m->active_entities.count; ++i)
    {
        m->active_entities.entities[i]->active = true;
    }
}

void map_deactivate_entities(Map* m)
{
    for (u32 i = 0; i < m->active_entities.count; ++i)
    {
        m->active_entities.entities[i]->active = false;
    }
}

void map_activate_tiles(Map* m)
{
    for (u32 i = 0; i < m->active_tiles.count; ++i)
    {
        m->active_tiles.tiles[i]->active = true;
    }
}

void map_deactivate_tiles(Map* m)
{
    for (u32 i = 0; i < m->active_tiles.count; ++i)
    {
        m->active_tiles.tiles[i]->active = false;
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
        map1->current = false;
        map2->current = true;
        map_deactivate_entities(map1);
        map_deactivate_tiles(map1);
        map_activate_entities(map2);
        map_activate_tiles(map2);
    }
    else if (map2->current)
    {
        // Transitioning to map3
        g->current_map = map3;
        map2->current = false;
        map3->current = true;
        map_deactivate_entities(map2);
        map_activate_entities(map3);
        map_deactivate_tiles(map2);
        map_activate_tiles(map3);
    }
    else if (map3->current)
    {
        // Transitioning to map1
        g->current_map = map1;
        map1->current = true;
        map3->current = false;
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

