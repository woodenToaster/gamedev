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

void updateAnimatedTiles(Map *m, u32 dt)
{
    for (size_t entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *e = &m->entities[entityIndex];
        if (e->type == ET_TILE && e->animation.totalFrames > 0)
        {
            updateAnimation(&e->animation, dt, e->active);
        }
    }
}

SDL_Rect getTileRect(Entity *tile)
{
    SDL_Rect result = {(int)(tile->position.x - 0.5f*tile->width), (int)(tile->position.y - 0.5f*tile->height),
                       (int)tile->width, (int)tile->height};
    return result;
}

void drawTile(Game *g, Entity *e, bool32 isBeingPlaced)
{
    SDL_Rect tileRect = getTileRect(e);

    if (e->spriteSheet.sheet)
    {
        if (e->animation.totalFrames > 0)
        {
            e->spriteRect.x = e->spriteRect.w * e->animation.currentFrame;
        }
        SDL_RenderCopy(g->renderer, e->spriteSheet.sheet, &e->spriteRect, &tileRect);
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
    Map* map = g->currentMap;
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

void destroyMap(Map* m)
{
    SDL_DestroyTexture(m->texture);
}
