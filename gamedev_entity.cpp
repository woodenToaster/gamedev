void initEntitySpriteSheet(Entity* e, SDL_Texture *texture, int num_x, int num_y)
{
    initSpriteSheet(&e->spriteSheet, texture, num_x, num_y);
    e->spriteRect.w = e->spriteSheet.spriteWidth;
    e->spriteRect.h = e->spriteSheet.spriteHeight;
    e->spriteSheet.scale = 1;
}

bool32 isEntity(Entity *e)
{
    bool32 result = 0;
    if (e)
    {
        result = e->type == ET_HERO;
    }
    return result;
}

void setRenderDrawColor(SDL_Renderer *renderer, u32 color)
{
    SDL_SetRenderDrawColor(renderer, getRedFromU32(color), getGreenFromU32(color),
                           getBlueFromU32(color), 255);
}

void drawEntity(Entity* e, Game* g)
{
    if (e->active && e->type != ET_TILE)
    {
        SDL_Rect dest = {};
        e->spriteRect.x = e->spriteRect.w * e->animation.currentFrame;
        // Draw collision box
        setRenderDrawColor(g->renderer, g->colors[COLOR_YELLOW]);
        SDL_Rect collisionRect = {(int)(e->position.x - 0.5f * e->width), (int)(e->position.y - e->height),
                                e->width, e->height};
        SDL_RenderFillRect(g->renderer, &collisionRect);

        // Draw position point
        // setRenderDrawColor(g->renderer, g->colors[COLOR_BLACK]);
        // SDL_RenderDrawPoint(g->renderer, (int)e->position.x, (int)e->position.y);

        i32 width = e->spriteDims.x;
        i32 height = e->spriteDims.y;
        dest = {(int)(e->position.x - 0.5f*width), (int)(e->position.y - height), width, height};
        SDL_RenderCopy(g->renderer, e->spriteSheet.sheet, &e->spriteRect, &dest);

        // Entity interactionRect
        // TODO(chj): This is visible when placing a tile. Push buffer rendering will fix that
        if (e->harvesting)
        {
            renderFilledRect(g->renderer, &e->heroInteractionRect, g->colors[COLOR_DARK_ORANGE]);
        }
    }
}

void drawPlacingTile(Game *g, Entity *h)
{
    if (h->tileToPlace && h->placingItem)
    {
        drawTile(g, h->tileToPlace, true);
    }
}

void reverseDirection(Entity* e)
{
    switch (e->direction)
    {
    case DIR_UP:
        e->direction = DIR_DOWN;
        break;
    case DIR_DOWN:
        e->direction = DIR_UP;
        break;
    case DIR_RIGHT:
        e->direction = DIR_LEFT;
        break;
    case DIR_LEFT:
        e->direction = DIR_RIGHT;
        break;
    }
}

void drawEntities(Game* g)
{
    Map *m = g->currentMap;
    for (u32 entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *e = &m->entities[entityIndex];
        drawEntity(e, g);
    }
}

void clampEntityToMap(Entity* h, Map* map)
{
    h->position.x = clampFloat(h->position.x, 0, (f32)map->widthPixels - 0.5f*h->width);
    h->position.y = clampFloat(h->position.y, 0, (f32)map->heightPixels - 0.5f*h->height);
}

void heroInteract(Entity *h, Game *g)
{
    (void)g;
    // Check for harvestable tile
    Point pointToHarvest = {(i32)h->position.x, (i32)h->position.y};
    i32 interactionRectWidth = h->width;
    i32 interactionRectHeight = h->height;

    switch (h->direction)
    {
    case DIR_UP:
        interactionRectWidth = h->height;
        interactionRectHeight = h->width;
        pointToHarvest.x -= (i32)(0.5f*interactionRectWidth);
        pointToHarvest.y -= (interactionRectHeight + interactionRectWidth);
        break;
    case DIR_DOWN:
        interactionRectWidth = h->height;
        interactionRectHeight = h->width;
        pointToHarvest.x -= (i32)(0.5f*interactionRectWidth);
        break;
    case DIR_LEFT:
        pointToHarvest.x -= (i32)(1.5f*h->width);
        pointToHarvest.y -= h->height;
        break;
    case DIR_RIGHT:
        pointToHarvest.x += (i32)(0.5f*h->width);
        pointToHarvest.y -= h->height;
        break;
    default:
        break;
    }

    h->heroInteractionRect = {pointToHarvest.x, pointToHarvest.y, interactionRectWidth, interactionRectHeight};

#if 0
    // Check for entities to interact with in a circle
    Vec2 heroCenter = {};
    heroCenter.x = h->position.x + (0.5f * h->width);
    heroCenter.y = h->position.y + (0.5f * h->height);
    heroInteractionRegion.center = heroCenter;
    heroInteractionRegion.radius = maxFloat32((f32)h->width, (f32)h->height) * 0.666666f;

    // See if there is an entity there
    for (u32 entityIndex = 0; entityIndex < g->currentMap->active_entities.count; ++entityIndex)
    {
        Entity *e = g->currentMap->active_entities.entities[entityIndex];
        if (entityIsHarlod(e))
        {
            if(circleOverlapsRect(&heroInteractionRegion, &e->bounding_box))
            {
                // TODO(chj): Find a cleaner way to do this
                interactWithEntity *f = (interactWithEntity*)((u8*)e + offsetof(Harlod, onEntityInteract));
                (*f)(e, h, g);
            }
        }
    }
#endif
}

Entity *addEntity(Map *m)
{
    assert(m->entityCount < ArrayCount(m->entities));
    Entity *result = &m->entities[m->entityCount++];

    return result;
}

internal CraftableItemType *findItemInBelt(Entity *h, CraftableItemType item)
{
    CraftableItemType *result = 0;
    for (u32 itemIndex = 0; itemIndex < h->beltItemCount; ++itemIndex)
    {
        if (h->beltItems[itemIndex] == item)
        {
            result = &h->beltItems[itemIndex];
        }
    }
    return result;
}

internal void craftItem(Entity *h, CraftableItemType item)
{
    u32 numRequiredItems = 0;
    InventoryItemType requiredItem = INV_NONE;

    switch (item)
    {
        case CRAFTABLE_TREE:
            numRequiredItems = 2;
            requiredItem = INV_LEAVES;
            break;
        case CRAFTABLE_GLOW_JUICE:
            numRequiredItems = 1;
            requiredItem = INV_GLOW;
            break;
        default:
            break;
    }

    if (h->inventory[requiredItem] >= numRequiredItems)
    {
        h->inventory[requiredItem] -= numRequiredItems;
        ++h->inventory[item];

        // Add to belt if it's not already there
        if (!findItemInBelt(h, item))
        {
            assert(requiredItem != INV_NONE);
            assert(h->beltItemCount < ArrayCount(h->beltItems));
            CraftableItemType *beltItem = &h->beltItems[h->beltItemCount++];
            *beltItem = item;
        }
    }
}

internal void placeItem(Map *m, Entity *h)
{
    if (h->tileToPlace && h->tileToPlace->validPlacement)
    {
        switch (h->tileToPlace->craftableItem)
        {
            case CRAFTABLE_TREE:
            {
                Entity *tile = h->tileToPlace;
                tile->collides = true;
                tile->active = true;
                tile->isHarvestable = true;

                h->inventory[INV_TREES]--;

                if (h->inventory[INV_TREES] == 0)
                {
                    // Remove from belt if it's there
                    CraftableItemType *item = findItemInBelt(h, CRAFTABLE_TREE);
                    if (item)
                    {
                        *item = h->beltItems[--h->beltItemCount];
                    }
                }

                if (tile->deleteAfterPlacement)
                {
                    u64 indexToRemove = tile->deleteAfterPlacement - m->entities;
                    m->entities[indexToRemove] = m->entities[--m->entityCount];
                    tile->deleteAfterPlacement = NULL;
                }
            } break;
            default:
                break;
        }
        h->placingItem = false;
        h->tileToPlace = NULL;
    }
}

#if 0
internal bool32 isInMap(Game *g, Vec2 pos)
{
    bool32 result = true;
    if (pos.x < 0 || pos.x > g->currentMap->widthPixels - 1 ||
        pos.y < 0 || pos.y > g->currentMap->heightPixels - 1)
    {
        result = false;
    }
    return result;
}
#endif

internal bool32 testWall(f32 wall, f32 relX, f32 relY, f32 playerDeltaX, f32 playerDeltaY,
                         f32 *tMin, f32 minY, f32 maxY)
{
    bool32 hit = false;

    f32 tEpsilon = 0.001f;
    if (playerDeltaX != 0.0f)
    {
        f32 tResult = (wall - relX) / playerDeltaX;
        f32 y = relY + tResult * playerDeltaY;
        if ((tResult >= 0.0f) && (*tMin > tResult))
        {
            if ((y >= minY) && (y <= maxY))
            {
                *tMin = maxFloat32(0.0f, tResult - tEpsilon);
                hit = true;
            }
        }
    }

    return hit;
}

internal Vec2 getTilePlacementPosition(Game *g, Entity *h)
{
    Vec2 result = {};

    Entity *tile = h->tileToPlace;
    if (tile)
    {
        // Calculate position to place new tile from hero's position
        u32 col = (u32)(h->position.x / tile->width);
        u32 row = (u32)(h->position.y / tile->height);

        switch (h->direction)
        {
        case DIR_UP:
            row--;
            break;
        case DIR_DOWN:
            row++;
            break;
        case DIR_LEFT:
            col--;
            break;
        case DIR_RIGHT:
            col++;
            break;
        default:
            break;
        }

        clampU32(row, 0, g->currentMap->rows - 1);
        clampU32(col, 0, g->currentMap->cols - 1);
        result = {col*tile->width + 0.5f*tile->width, row*tile->height + 0.5f*tile->height};
    }

    return result;
}

internal bool32 isValidTilePlacment(Map *m, Entity *tileToPlace)
{
    bool32 result = true;
    tileToPlace->deleteAfterPlacement = NULL;

    for (u32 entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *testEntity = &m->entities[entityIndex];

        if (testEntity != tileToPlace)
        {
            SDL_Rect tileRect = getTileRect(tileToPlace);
            SDL_Rect testRect = getTileRect(testEntity);

            if (rectsOverlap(&tileRect, &testRect))
            {
                if (testEntity->collides)
                {
                    result = false;
                    break;
                }
                else
                {
                    // A tile can be placed here because this is not a colliding tile.
                    // However, we need to remember it so we can delete it after we
                    // place a tile here.
                    tileToPlace->deleteAfterPlacement = testEntity;
                }
            }
        }
    }

    return result;
}

internal void updateHero(Entity* h, Input* input, Game* g)
{
    Map *map = g->currentMap;
    // TODO(chj): Handle joystick and keyboard on separate paths
    Vec2 acceleration = {};
    acceleration.x = input->stickX;
    acceleration.y = input->stickY;

    if (input->keyDown[KEY_RIGHT])
    {
        acceleration.x = 1.0f;
    }
    if (input->keyDown[KEY_LEFT])
    {
        acceleration.x = -1.0f;
    }
    if (input->keyDown[KEY_UP])
    {
        acceleration.y = -1.0f;
    }
    if (input->keyDown[KEY_DOWN])
    {
        acceleration.y = 1.0f;
    }

    if (acceleration.x == 0.0f && acceleration.y == 0.0f)
    {
        // TODO(chj): should this really be false when skidding to a stop?
        h->isMoving = false;
    }
    else
    {
        u32 spriteHeight = h->spriteSheet.spriteHeight;
        if (acceleration.x > 0)
        {
            h->spriteRect.y = 0 * spriteHeight;
            h->direction = DIR_RIGHT;
        }
        if (acceleration.x < 0)
        {
            h->spriteRect.y = 3 * spriteHeight;
            h->direction = DIR_LEFT;
        }
        if (acceleration.y < 0)
        {
            h->spriteRect.y = 1 * spriteHeight;
            h->direction = DIR_UP;
        }
        if (acceleration.y > 0)
        {
            h->spriteRect.y = 4 * spriteHeight;
            h->direction = DIR_DOWN;
        }

        h->isMoving = true;
    }

    // Diagonal movement
    if (acceleration.x != 0.0f && acceleration.y != 0.0f)
    {
        acceleration *= 0.707186781187f;
    }

    acceleration *= h->speed;
    // Friction
    acceleration -= 8 * h->velocity;

    f32 dt = (f32)g->dt / 1000.0f;
    Vec2 oldPosition = h->position;
    Vec2 playerDelta = (0.5 * acceleration * square(dt)) + (h->velocity * dt);
    Vec2 newPosition =  playerDelta + h->position;
    h->velocity = (acceleration * dt) + h->velocity;

    if (!h->isMoving)
    {
        h->animation.currentFrame = 0;
        h->spriteRect.x = 0;
    }

    for (int iter = 0; iter < 4; ++iter)
    {
        f32 tMin = 1.0f;
        Vec2 wallNormal = {};
        bool32 hit = false;
        Vec2 desiredPosition = h->position + playerDelta;

        for (u32 entityIndex = 0; entityIndex < map->entityCount; ++entityIndex)
        {
            Entity *testEntity = &map->entities[entityIndex];

            f32 heightOffset = 0.5f*h->height;
            if (testEntity->type == ET_TILE)
            {
                heightOffset = 0.0f;
            }
            f32 minX = maxFloat32(0.0f, testEntity->position.x - 0.5f*testEntity->width - 0.5f*h->width);
            f32 minY = maxFloat32(0.0f, testEntity->position.y - 0.5f*testEntity->height - heightOffset);
            Vec2 minCorner = {minX, minY};
            f32 maxX = minFloat32((f32)map->widthPixels,
                                  testEntity->position.x + 0.5f*testEntity->width + 0.5f*h->width);
            f32 maxY = minFloat32((f32)map->heightPixels,
                                  testEntity->position.y + 0.5f*testEntity->height + h->height);
            Vec2 maxCorner = {maxX, maxY};


            if (isTileFlagSet(testEntity, TP_QUICKSAND))
            {
                bool32 inQuicksand = false;
                if (testWall(minCorner.x, oldPosition.x, oldPosition.y, playerDelta.x, playerDelta.y,
                             &tMin, minCorner.y, maxCorner.y))
                {
                    inQuicksand = true;
                }
                if (testWall(maxCorner.x, oldPosition.x, oldPosition.y, playerDelta.x, playerDelta.y,
                             &tMin, minCorner.y, maxCorner.y))
                {
                    inQuicksand = true;
                }
                if (testWall(minCorner.y, oldPosition.y, oldPosition.x, playerDelta.y, playerDelta.x,
                             &tMin, minCorner.x, maxCorner.x))
                {
                    inQuicksand = true;
                }
                if (testWall(maxCorner.y, oldPosition.y, oldPosition.x, playerDelta.y, playerDelta.x,
                             &tMin, minCorner.x, maxCorner.x))
                {
                    inQuicksand = true;
                }

                if (inQuicksand)
                {
                    // TODO(chj): Slow hero down
                    // TODO(chj): Need constant sound
                    // TODO(chj): Move to harvesting loop below (a collision loop)
                    if (h->isMoving)
                    {
                        queueSound(&g->sounds, &g->mudSound);
                    }
                }
            }

            if (testEntity->collides)
            {
                if (testWall(minCorner.x, oldPosition.x, oldPosition.y, playerDelta.x, playerDelta.y,
                            &tMin, minCorner.y, maxCorner.y))
                {
                    wallNormal = {-1, 0};
                    hit = true;
                }
                if (testWall(maxCorner.x, oldPosition.x, oldPosition.y, playerDelta.x, playerDelta.y,
                            &tMin, minCorner.y, maxCorner.y))
                {
                    wallNormal = {1, 0};
                    hit = true;
                }
                if (testWall(minCorner.y, oldPosition.y, oldPosition.x, playerDelta.y, playerDelta.x,
                            &tMin, minCorner.x, maxCorner.x))
                {
                    wallNormal = {0, -1};
                    hit = true;
                }
                if (testWall(maxCorner.y, oldPosition.y, oldPosition.x, playerDelta.y, playerDelta.x,
                            &tMin, minCorner.x, maxCorner.x))
                {
                    wallNormal = {0, 1};
                    hit = true;
                }
            }
        }

        h->position += (tMin * playerDelta);
        h->velocity -= 1 * vec2_dot(&h->velocity, &wallNormal) * wallNormal;
        playerDelta = desiredPosition - h->position;
        playerDelta -= 1 * vec2_dot(&playerDelta, &wallNormal) * wallNormal;
    }

    clampEntityToMap(h, map);

    // Actions
    h->harvesting = input->keyPressed[KEY_SPACE] || input->buttonPressed[BUTTON_A];
    h->craft = input->keyPressed[KEY_C];
    if (input->keyPressed[KEY_P])
    {
        if (h->inventory[INV_TREES] > 0)
        {
            h->placingItem = true;
        }
    }

    if (h->harvesting && !h->placingItem)
    {
        heroInteract(h, g);

        for (u32 entityIndex = 0; entityIndex < map->entityCount; ++entityIndex)
        {
            Entity *testEntity = &map->entities[entityIndex];

            switch (testEntity->type)
            {
                case ET_TILE:
                {
                    // Tile position is at center of tile
                    SDL_Rect tileBoundingBox = {(i32)testEntity->position.x - (i32)(0.5f*testEntity->width),
                                                (i32)testEntity->position.y - (i32)(0.5f*testEntity->height),
                                                testEntity->width, testEntity->height};

                    if (rectsOverlap(&h->heroInteractionRect, &tileBoundingBox))
                    {
                        // TODO(chj): Use tileFlags
                        if (testEntity->isHarvestable)
                        {
                            testEntity->isHarvestable = false;
                            testEntity->collides = false;
                            if (testEntity->harvestedSprite)
                            {
                                initSpriteSheet(&testEntity->spriteSheet, testEntity->harvestedSprite, 1, 1);
                            }
                            else
                            {
                                testEntity->spriteRect.x += map->tileWidth;
                            }
                            h->inventory[testEntity->harvestedItem]++;
                        }

                        if (isTileFlagSet(testEntity, TP_INTERACTIVE))
                        {
                            if (isTileFlagSet(testEntity, TP_FIRE))
                            {
                                testEntity->active = !testEntity->active;
                            }
                        }
                    }
                    break;
                }
                case ET_HARLOD:
                {
                    SDL_Rect harlodCollisionRegion = {(i32)(testEntity->position.x - 0.5f*testEntity->width),
                                                      (i32)(testEntity->position.y - testEntity->height),
                                                      testEntity->width, testEntity->height};

                    if (rectsOverlap(&h->heroInteractionRect, &harlodCollisionRegion))
                    {
                        if (!testEntity->dialogueFile.contents)
                        {
                            testEntity->dialogueFile = readEntireFile("dialogues/harlod_dialogues.txt");
                            // TODO(chj): Need to null terminate everything. This will change. We
                            // want to parse files for strings and tokenize, etc. For now it's hard coded
                            testEntity->dialogueFile.contents[9] = '\0';
                        }
                        startDialogueMode(g, (char*)testEntity->dialogueFile.contents);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    if (h->craft)
    {
        CraftableItemType item = CRAFTABLE_TREE;
        craftItem(h, item);
    }

    if (h->placingItem)
    {
        if (!h->tileToPlace)
        {
            Map *m = g->currentMap;
            h->tileToPlace = addEntity(m);
            *h->tileToPlace = {};
            Entity *tile = h->tileToPlace;
            tile->width = 80;
            tile->height = 80;

            tile->unharvestedSprite = g->treeTexture;
            tile->harvestedSprite = g->treeStumpTexture;
            initEntitySpriteSheet(tile, tile->unharvestedSprite, 1, 1);
            tile->color = g->colors[COLOR_NONE];
            tile->harvestedItem = INV_LEAVES;
            // TODO(chj): Need to know the type before we fill all the specifics out
            tile->craftableItem = CRAFTABLE_TREE;
        }

        h->tileToPlace->position = getTilePlacementPosition(g, h);
        h->tileToPlace->validPlacement = isValidTilePlacment(map, h->tileToPlace);
    }

    if (h->harvesting && h->placingItem)
    {
        placeItem(map, h);
    }
}
