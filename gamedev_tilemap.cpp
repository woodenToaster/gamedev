#include "gamedev_tilemap.h"

void addTileFlags(Entity *e, u32 prop)
{
    e->tileFlags |= prop;
}

bool32 isTileFlagSet(Entity *e, TileProperty prop)
{
    bool32 result = e->tileFlags & prop;
    return result;
}

// void lightFire(Tile *t, Hero *h)
// {
//     (void)h;
//     t->animation_is_active = true;
// }

// void initMap(Map* m, u32 cols, u32 rows, Tile** tiles, SDL_Renderer* renderer)
// {
//     m->cols = cols;
//     m->rows = rows;
//     m->tile_width = tiles[0][0].width;
//     m->tile_height = tiles[0][0].height;
//     m->tiles = tiles;
//     m->widthPixels = cols * tiles[0]->width;
//     m->heightPixels = rows * tiles[0]->height;

//     m->texture = SDL_CreateTexture(
//         renderer,
//         SDL_PIXELFORMAT_RGB888,
//         SDL_TEXTUREACCESS_TARGET,
//         m->widthPixels,
//         m->heightPixels
//     );

//     if (!m->texture)
//     {
//         printf("Failed to create surface: %s\n", SDL_GetError());
//         exit(1);
//     }
// }

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

SDL_Rect getTileRect(Entity *tile)
{
    SDL_Rect result = {(int)(tile->position.x - 0.5f*tile->width), (int)(tile->position.y - 0.5f*tile->height),
                       (int)tile->width, (int)tile->height};
    return result;
}

void drawTile(Game *g, Entity *e, bool32 isBeingPlaced)
{
    SDL_Rect tileRect = getTileRect(e);

    if (e->sprite_sheet.sheet)
    {
        // TODO(chj): Animated tiles
        // if (t->has_animation && t->animation_is_active)
        // {
        //     t->sprite_rect.x = t->sprite_rect.w * t->animation.current_frame;
        // }
        SDL_RenderCopy(g->renderer, e->sprite_sheet.sheet, NULL, &tileRect);
    }
    else
    {
        u32 tileColor = e->color;
        renderFilledRect(g->renderer, &tileRect, tileColor);
    }

    if (isBeingPlaced)
    {
        if (e->validPlacement)
        {
            // TODO(chj): Draw green filter?
        }
        else
        {
            // Draw red filter on top
            renderFilledRect(g->renderer, &tileRect, g->colors[COLOR_RED], 128);
        }
    }
}

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
            drawTile(g, e);
        }
    }
    drawEntities(g);
}

#if 0
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
#endif

void map_destroy(Map* m)
{
    SDL_DestroyTexture(m->texture);
}
