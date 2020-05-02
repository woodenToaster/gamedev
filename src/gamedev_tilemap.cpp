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

void drawTile(RenderCommands *commands, Game *g, Entity *e, b32 isBeingPlaced)
{
    if (e->isVisible && e->active)
    {
        Rect tileRect = getEntityRect(e);
        if (e->texture.id)
        {
            if (e->animation.totalFrames > 0)
            {
                e->spriteRect.x = e->spriteRect.w * e->animation.currentFrame;
            }
            // pushSprite(commands, e->spriteSheet.sheet, e->spriteRect, tileRect, RenderLayer_Ground);
            pushTexture(commands, e);
        }
        else
        {
            if (e->color.a)
            {
                Rect2 dest = {};
                dest.minP = e->position;
                dest.maxP = dest.minP + e->size;
                pushFilledRect(commands, dest, e->color, RenderLayer_Ground);
            }
        }

        // Draw position
        // pushFilledRect(commands, {(int)e->position.x, (int)e->position.y, 2, 2}, g->colors[Color_Red],
        //                RenderLayer_Entities);

        if (isBeingPlaced)
        {
            if (e->validPlacement)
            {
                // TODO(cjh): Draw green filter?
                // pushFilledRect(commands, tileRect, g->colors[Color_Green], 128);
            }
            else
            {
                // Draw red filter on top
                Vec4u8 transparentRed = g->colors[Color_Red];
                transparentRed.a = 128;
                pushFilledRect(commands, tileRect, transparentRed, RenderLayer_Entities);
            }
        }
    }
}

void drawTiles(RenderCommands *commands, Game *g)
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
            drawTile(commands, g, e);
        }
    }
}

void drawBackground(RenderCommands *commands, Game *g)
{
    Rect2 floor =  {};
    floor.minP = vec2(0, 0);
    floor.maxP = vec2((f32)g->currentMap->cols - 1, (f32)g->currentMap->rows - 1);
    pushFilledRect(commands, floor, g->colors[Color_Grey], RenderLayer_Ground);
}
