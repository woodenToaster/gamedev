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
                    if (testTile && isTileFlagSet(testTile, TP_FLAMMABLE) && !testTile->onFire)
                    {
                        // TODO(chj): Check e, not testTile for burning out
                        // TODO(chj): Need to associate flame entity with tile under it
                        if (testTile->fireState == FIRE_NONE)
                        {
                            testTile->fireState = FIRE_STARTED;
                            testTile->timeToCatchFire = 3000;

                        }
                        else if (testTile->fireState == FIRE_STARTED)
                        {
                            if (testTile->timeToCatchFire < 0)
                            {
                                addFlame(g, testPos);
                                testTile->onFire = true;
                                testTile->fireState = FIRE_CAUGHT;
                                testTile->timeToCatchFire = 0;
                                testTile->timeSpentOnFire = 0;
                            }
                            else
                            {
                                testTile->timeToCatchFire -= g->dt;
                            }
                        }
                        else if (testTile->fireState == FIRE_CAUGHT)
                        {
                            testTile->timeSpentOnFire += g->dt;
                            if (testTile->timeSpentOnFire > 3000)
                            {
                                testTile->fireState = FIRE_BURNT;
                                testTile->timeSpentOnFire = 0;
                                // TODO(chj): Remove flame
                                testTile->spriteRect.x += testTile->width;
                            }
                        }


                    }
                }
            }
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
    renderFilledRect(g->renderer, &dest, g->colors[COLOR_LIME_GREEN]);

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
