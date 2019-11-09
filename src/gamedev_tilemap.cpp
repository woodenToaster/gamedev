#include "gamedev_tilemap.h"

void addTileFlags(Entity *e, u32 prop)
{
    e->tileFlags |= prop;
}

void removeTileFlags(Entity *e, u32 prop)
{
    addTileFlags(e, prop);
    e->tileFlags ^= prop;
}

b32 isTileFlagSet(Entity *e, TileProperty prop)
{
    b32 result = e->tileFlags & prop;
    return result;
}

void updateTiles(Game *g)
{
    Map *m = g->currentMap;
    for (size_t entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *e = &m->entities[entityIndex];
        if (e->type == ET_TILE)
        {
            if (e->animation.totalFrames > 0) {
                updateAnimation(&e->animation, g->dt, e->active);
            }
        }

        if (isTileFlagSet(e, TP_FLAME))
        {
            // Check all adjacent tiles for the TP_FLAMMABLE property.
            // If something is flammable, catch it on fire.
            for (int rowIndex = -1; rowIndex <= 1; ++rowIndex) {
                for (int colIndex = -1; colIndex <= 1; ++colIndex) {
                    if (rowIndex == 0 && colIndex == 0) {
                        continue;
                    }
                    Vec2 testPos = {e->position.x + colIndex*(int)m->tileWidth,
                                    e->position.y + rowIndex*(int)m->tileHeight};

                    Entity *testTile = getTileAtPosition(m, testPos);
                    if (testTile && isTileFlagSet(testTile, TP_FLAMMABLE) && testTile->fireState == FIRE_NONE)
                    {
                        testTile->fireState = FIRE_STARTED;
                        testTile->timeToCatchFire = 3000;
                    }
                }
            }
        }

        // Update Flammable tiles
        // TODO(cjh): Do burnt tiles become non-collidable and unharvestable?
        switch (e->fireState)
        {
            case FIRE_STARTED:
            {
                if	(e->timeToCatchFire < 0)
                {
                    addFlame(g, e->position);
                    e->fireState = FIRE_CAUGHT;
                    addTileFlags(e, TP_FLAME);
                    e->timeToCatchFire = 0;
                    e->timeSpentOnFire = 0;
                }
                else
                {
                    e->timeToCatchFire -= g->dt;
                }
            } break;
            case FIRE_CAUGHT:
            {
                e->timeSpentOnFire += g->dt;
                if (e->timeSpentOnFire > 3000)
                {
                    e->fireState = FIRE_BURNT;
                    e->timeSpentOnFire = 0;
                    removeTileFlags(e, TP_FLAME | TP_HARVEST | TP_SOLID);
                    removeEntity(m, e->position, TP_FLAME);
                    e->spriteRect.x = e->burntTileIndex*e->spriteSheet.spriteWidth;
                    e->collides = false;
                }
            } break;
            case FIRE_NONE:
                // Fall through
            case FIRE_BURNT:
                // Fall through
            default:
                break;
        }
    }
}

Rect getEntityRect(Entity *e)
{
    f32 yPercent = e->type == ET_TILE ? 0.5f : 1.0f;
    Rect result = {(int)(e->position.x - 0.5f*e->width), (int)(e->position.y - yPercent*e->height),
                   (int)e->width, (int)e->height};
    return result;
}

void drawTile(RenderGroup *group, Game *g, Entity *e, b32 isBeingPlaced)
{
    Rect tileRect = getEntityRect(e);

    if (e->spriteSheet.sheet.texture)
    {
        if (e->animation.totalFrames > 0)
        {
            e->spriteRect.x = e->spriteRect.w * e->animation.currentFrame;
        }
        pushSprite(group, e->spriteSheet.sheet, e->spriteRect, tileRect, RenderLayer_Entities);
    }
    else
    {
        u32 tileColor = e->color;
        pushFilledRect(group, tileRect, tileColor, RenderLayer_Ground);
    }

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

void drawTiles(RenderGroup *group, Game *g)
{
    Map* map = g->currentMap;
    for (u32 entityIndex = 0; entityIndex < map->entityCount; ++entityIndex)
    {
        Entity *e = &map->entities[entityIndex];
        if (e->type == ET_TILE)
        {
            drawTile(group, g, e);
        }
    }
}

void drawBackground(RenderGroup *group, Game *g)
{
    Camera *c = &g->camera;
    Rect backgroundDest = {c->viewport.x, c->viewport.y, c->viewport.w, c->viewport.h};
    pushFilledRect(group, backgroundDest, g->colors[Color_Grey], RenderLayer_Ground);
}
