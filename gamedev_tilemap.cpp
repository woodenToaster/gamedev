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

SDL_Rect getEntityRect(Entity *e)
{
    f32 yPercent = e->type == ET_TILE ? 0.5f : 1.0f;
    SDL_Rect result = {(int)(e->position.x - 0.5f*e->width), (int)(e->position.y - yPercent*e->height),
                       (int)e->width, (int)e->height};
    return result;
}

void drawTile(Game *g, Entity *e, bool32 isBeingPlaced)
{
    SDL_Rect tileRect = getEntityRect(e);

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
            // renderFilledRect(g->renderer, &tileRect, g->colors[COLOR_GREEN], 128);
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
