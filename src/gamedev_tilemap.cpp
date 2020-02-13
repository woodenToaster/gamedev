#include "gamedev_tilemap.h"

inline internal void addTileFlags(Entity *e, u32 prop)
{
    e->tileFlags |= prop;
}

inline internal void removeTileFlags(Entity *e, u32 prop)
{
    addTileFlags(e, prop);
    e->tileFlags ^= prop;
}

inline internal b32 isTileFlagSet(Entity *e, TileProperty prop)
{
    b32 result = e->tileFlags & prop;
    return result;
}

void updateTiles(Game *g, Input *input)
{
    i32 millisecondsToBurn = 3000;

    Map *m = g->currentMap;
    for (size_t entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *e = &m->entities[entityIndex];
        if (!e->active || e->type != EntityType_Tile)
        {
            continue;
        }

        if (e->animation.totalFrames > 0) {
            updateAnimation(&e->animation, input->dt, e->shouldAnimate);
        }

        // Update Flammable tiles
        // TODO(cjh): Do burnt tiles become non-collidable and unharvestable?
        switch (e->fireState)
        {
            case FireState_Started:
            {
                if	(e->timeToCatchFire < 0)
                {
                    addFlame(g, e->position);
                    e->fireState = FireState_Caught;
                    e->timeToCatchFire = 0;
                    e->timeSpentOnFire = 0;
                }
                else
                {
                    e->timeToCatchFire -= input->dt;
                }
            } break;
            case FireState_Caught:
            {
                e->timeSpentOnFire += input->dt;
                if (e->timeSpentOnFire > millisecondsToBurn)
                {
                    e->fireState = FireState_Burnt;
                    e->timeSpentOnFire = 0;
                    removeTileFlags(e, TileProperty_Harvest | TileProperty_Solid);
                    removeFlameFromTileAtPosition(m, e->position);
                    e->spriteRect.x = e->burntTileIndex*e->spriteSheet.spriteWidth;
                    e->collides = false;
                }
            } break;
            case FireState_None:
                // Fall through
            case FireState_Burnt:
                // Fall through
            default:
                break;
        }
    }
}

Rect getEntityRect(Entity *e)
{
    f32 yPercent = e->type == EntityType_Tile ? 0.5f : 1.0f;
    Rect result = {(int)(e->position.x - 0.5f*e->width), (int)(e->position.y - yPercent*e->height),
                   (int)e->width, (int)e->height};
    return result;
}

void drawTile(RenderGroup *group, Game *g, Entity *e, b32 isBeingPlaced)
{
    if (e->isVisible && e->active)
    {
        Rect tileRect = getEntityRect(e);
        if (e->spriteSheet.sheet.texture)
        {
            if (e->animation.totalFrames > 0)
            {
                e->spriteRect.x = e->spriteRect.w * e->animation.currentFrame;
            }
            pushSprite(group, e->spriteSheet.sheet, e->spriteRect, tileRect, RenderLayer_Ground);
        }
        else
        {
            u32 tileColor = e->color;
            pushFilledRect(group, tileRect, tileColor, RenderLayer_Ground);
        }

        // Draw position
        // pushFilledRect(group, {(int)e->position.x, (int)e->position.y, 2, 2}, g->colors[Color_Red],
        //                RenderLayer_Entities);

        if (isBeingPlaced)
        {
            if (e->validPlacement)
            {
                // TODO(cjh): Draw green filter?
                // pushFilledRect(group, tileRect, g->colors[Color_Green], 128);
            }
            else
            {
                // Draw red filter on top
                pushFilledRect(group, tileRect, g->colors[Color_Red], RenderLayer_Entities, 128);
            }
        }
    }
}

void drawTiles(RenderGroup *group, Game *g)
{
    Map* map = g->currentMap;
    for (u32 entityIndex = 0; entityIndex < map->entityCount; ++entityIndex)
    {
        Entity *e = &map->entities[entityIndex];
        if (!e->active)
        {
            continue;
        }
        if (e->type == EntityType_Tile)
        {
            drawTile(group, g, e);
        }
    }
}

void drawBackground(RenderGroup *group, Game *g)
{
    Camera *c = &g->camera;
    Rect backgroundDest = {c->viewport.x, c->viewport.y, c->viewport.w, c->viewport.h};
    pushFilledRect(group, backgroundDest, vec3(128, 128, 128), RenderLayer_Ground);
    // pushFilledRect(group, backgroundDest, g->colors[Color_Grey], RenderLayer_Ground);
}
