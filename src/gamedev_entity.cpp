void initEntitySpriteSheet(Entity* e, TextureHandle texture, int num_x, int num_y)
{
    initSpriteSheet(&e->spriteSheet, texture, num_x, num_y);
    e->spriteRect.w = e->spriteSheet.spriteWidth;
    e->spriteRect.h = e->spriteSheet.spriteHeight;
    e->spriteSheet.scale = 1;
}

void initHarvestableTree(Entity *tile, Game *game)
{
    addTileFlags(tile, (u32)(TileProperty_Harvest | TileProperty_Solid | TileProperty_Flammable));
    tile->color = game->colors[Color_None];
    tile->collides = true;
    tile->burntTileIndex = 2;
    initEntitySpriteSheet(tile, game->harvestableTreeTexture, 3, 1);
    tile->isVisible = true;
    tile->harvestedItem = InventoryItemType_Leaves;
}

void initGlowTree(Entity *tile, Game *game)
{
    addTileFlags(tile, (u32)(TileProperty_Harvest | TileProperty_Solid));
    tile->collides = true;
    initEntitySpriteSheet(tile, game->glowTreeTexture, 2, 1);
    tile->isVisible = true;
    tile->harvestedItem = InventoryItemType_Glow;
}

b32 isEntity(Entity *e)
{
    b32 result = 0;
    if (e)
    {
        result = e->type == EntityType_Hero;
    }
    return result;
}

void drawEntity(RenderGroup *group, Entity* e, Game* g)
{
    if (e->isVisible && e->type != EntityType_Tile)
    {
        // Draw collision box
        Rect collisionRect = {(int)(e->position.x - 0.5f * e->width), (int)(e->position.y - e->height),
                              e->width, e->height};
        pushFilledRect(group, collisionRect, g->colors[Color_Yellow], RenderLayer_Entities);

        // Draw sprite
        i32 width = e->spriteDims.x;
        i32 height = e->spriteDims.y;
        e->spriteRect.x = e->spriteRect.w * e->animation.currentFrame;
        Rect dest = {(int)(e->position.x - 0.5f*width), (int)(e->position.y - height), width, height};
        pushSprite(group, e->spriteSheet.sheet, e->spriteRect, dest, RenderLayer_Entities);

        // Draw entity interactionRect
        if (e->harvesting)
        {
            pushFilledRect(group, e->heroInteractionRect, g->colors[Color_DarkOrange], RenderLayer_Entities);
        }
    }
}

void drawPlacingTile(RenderGroup *group, Game *g, Entity *h)
{
    if (h->tileToPlace && h->placingItem)
    {
        drawTile(group, g, h->tileToPlace, true);
    }
}

void reverseDirection(Entity* e)
{
    switch (e->direction)
    {
        case Direction_Up:
            e->direction = Direction_Down;
            break;
        case Direction_Down:
            e->direction = Direction_Up;
            break;
        case Direction_Right:
            e->direction = Direction_Left;
            break;
        case Direction_Left:
            e->direction = Direction_Right;
            break;
    }
}

void drawEntities(RenderGroup *group, Game* g)
{
    Map *m = g->currentMap;
    for (u32 entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *e = &m->entities[entityIndex];
        drawEntity(group, e, g);
    }
}

void clampEntityToMap(Entity* h, Map* map)
{
    h->position.x = clampFloat(h->position.x, 0, (f32)map->widthPixels - 0.5f*h->width);
    h->position.y = clampFloat(h->position.y, 0, (f32)map->heightPixels - 0.5f*h->height);
}

void updateHeroInteractionRegion(Entity *h)
{
    // Check for harvestable tile
    Point pointToHarvest = {(i32)h->position.x, (i32)h->position.y};
    i32 interactionRectWidth = h->width;
    i32 interactionRectHeight = h->height;

    switch (h->direction)
    {
        case Direction_Up:
            interactionRectWidth = h->height;
            interactionRectHeight = h->width;
            pointToHarvest.x -= (i32)(0.5f*interactionRectWidth);
            pointToHarvest.y -= (interactionRectHeight + interactionRectWidth);
            break;
        case Direction_Down:
            interactionRectWidth = h->height;
            interactionRectHeight = h->width;
            pointToHarvest.x -= (i32)(0.5f*interactionRectWidth);
            break;
        case Direction_Left:
            pointToHarvest.x -= (i32)(1.5f*h->width);
            pointToHarvest.y -= h->height;
            break;
        case Direction_Right:
            pointToHarvest.x += (i32)(0.5f*h->width);
            pointToHarvest.y -= h->height;
            break;
        default:
            break;
    }

    h->heroInteractionRect = {pointToHarvest.x, pointToHarvest.y, interactionRectWidth, interactionRectHeight};
}

Entity *addEntity(Map *m)
{
    assert(m->entityCount < ArrayCount(m->entities));
    Entity *result = &m->entities[m->entityCount++];
    *result = {};

    return result;
}

Entity *addFlame(Game *g, Vec2 pos)
{
    Map *m = g->currentMap;
    Entity *result = addEntity(m);
    result->width = 80;
    result->height = 80;
    initEntitySpriteSheet(result, g->flameTexture, 10, 1);
    initAnimation(&result->animation, 10, 100);
    result->color = g->colors[Color_None];
    result->isVisible = true;
    result->shouldAnimate = true;
    result->position = pos;
    addTileFlags(result, TileProperty_Flame);

    return result;
}

void removeEntity(Map *m, Vec2 pos, TileProperty prop)
{
    for (u32 i = 0; i < m->entityCount; ++i) {
        Entity *e = m->entities + i;
        Rect entityRect = getEntityRect(e);
        if (isTileFlagSet(e, prop) && positionIsInRect(pos, &entityRect))
        {
            *e = m->entities[--m->entityCount];
            break;
        }
    }
}

internal BeltItem *findItemInBelt(Entity *h, CraftableItemType item)
{
    BeltItem *result = 0;
    for (u32 itemIndex = 0; itemIndex < h->beltItemCount; ++itemIndex)
    {
        if (h->beltItems[itemIndex].type == item)
        {
            result = &h->beltItems[itemIndex];
        }
    }
    return result;
}

internal void craftItem(Entity *h, CraftableItemType item)
{
    u32 numRequiredItems = 0;
    InventoryItemType requiredItem = InventoryItemType_None;

    switch (item)
    {
        case Craftable_Tree:
            numRequiredItems = 2;
            requiredItem = InventoryItemType_Leaves;
            break;
        case Craftable_Glow_Juice:
            numRequiredItems = 1;
            requiredItem = InventoryItemType_Glow;
            break;
        default:
            break;
    }

    if (requiredItem != InventoryItemType_None)
    {
        if (h->inventory[requiredItem] >= numRequiredItems)
        {
            if (h->beltItemCount < ArrayCount(h->beltItems))
            {
                h->inventory[requiredItem] -= numRequiredItems;
                BeltItem *beltItem = findItemInBelt(h, item);
                if (beltItem)
                {
                    beltItem->count++;
                }
                else
                {
                    beltItem = &h->beltItems[h->beltItemCount++];
                    beltItem->type = item;
                    beltItem->count = 1;
                }
            }
        }
    }
}

internal void placeItem(Game *g, Entity *h)
{
    Map *m = g->currentMap;
    if (h->tileToPlace && h->tileToPlace->validPlacement)
    {
        Entity *tile = h->tileToPlace;
        BeltItem *item = h->beltItems + h->activeBeltItemIndex;
        if (--item->count == 0)
        {
            // Remove it from the belt
            *item = h->beltItems[--h->beltItemCount];
        }

        switch (h->tileToPlace->craftableItem)
        {
            case Craftable_Tree:
                // TODO(cjh): Calls initEntitySpriteSheet twice, which does QueryTexture
                initHarvestableTree(tile, g);
                break;
            case Craftable_Glow_Juice:
                // TODO(cjh): Calls initEntitySpriteSheet twice, which does QueryTexture
                initGlowTree(tile, g);
                break;
            default:
                break;
        }

        if (tile->deleteAfterPlacement)
        {
            u64 indexToRemove = tile->deleteAfterPlacement - m->entities;
            m->entities[indexToRemove] = m->entities[--m->entityCount];
            tile->deleteAfterPlacement = NULL;
        }

        h->placingItem = false;
        h->tileToPlace = NULL;
    }
}

#if 0
internal b32 isInMap(Game *g, Vec2 pos)
{
    b32 result = true;
    if (pos.x < 0 || pos.x > g->currentMap->widthPixels - 1 ||
        pos.y < 0 || pos.y > g->currentMap->heightPixels - 1)
    {
        result = false;
    }
    return result;
}
#endif

internal b32 testWall(f32 wall, f32 relX, f32 relY, f32 playerDeltaX, f32 playerDeltaY,
                         f32 *tMin, f32 minY, f32 maxY)
{
    b32 hit = false;

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

internal Entity *getTileAtPosition(Map *m, Vec2 pos)
{
    Entity *result = 0;

    for (size_t entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *e = &m->entities[entityIndex];
        if (e->type == EntityType_Tile)
        {
            Rect tileRect = getEntityRect(e);
            if (positionIsInRect(pos, &tileRect))
            {
                result = e;
                break;
            }
        }
    }

    return result;
}

internal Vec2 getTilePlacementPosition(Game *g, Entity *h)
{
    Vec2 result = {};

    Entity *tile = h->tileToPlace;
    if (tile)
    {
        // Calculate position to place new tile from hero's position, with Minkowski sum
        u32 col = (u32)(h->position.x / tile->width);
        u32 row = (u32)(h->position.y / tile->height);

        switch (h->direction)
        {
            case Direction_Up:
                row = (u32)((h->position.y - h->height) / tile->height);
                row--;
                break;
            case Direction_Down:
                row++;
                break;
            case Direction_Left:
                col = (u32)((h->position.x - 0.5f*h->width) / tile->width);
                col--;
                break;
            case Direction_Right:
                col = (u32)((h->position.x + 0.5f*h->width) / tile->width);
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

internal b32 isValidTilePlacment(Map *m, Entity *tileToPlace)
{
    b32 result = true;
    tileToPlace->deleteAfterPlacement = NULL;

    for (u32 entityIndex = 0; entityIndex < m->entityCount; ++entityIndex)
    {
        Entity *testEntity = &m->entities[entityIndex];

        if (testEntity != tileToPlace)
        {
            Rect tileRect = getEntityRect(tileToPlace);
            Rect testRect = getEntityRect(testEntity);

            if (rectsOverlap(&tileRect, &testRect))
            {
                if (testEntity->collides)
                {
                    result = false;
                }
                else
                {
                    // A tile can be placed here because this is not a colliding tile.
                    // However, we need to remember the tile that will be replaced so
                    // we can delete it after we place the new tile here.
                    tileToPlace->deleteAfterPlacement = testEntity;
                }
                break;
            }
        }
    }

    return result;
}

void pushInteractionHint(RenderGroup *group, Game *g, char *text)
{
    i32 textLength = (i32)strlen(text);
    i32 screenCenterX = g->camera.viewport.w / 2;
    // TODO(cjh): Get total pixel size of all the sprites creating the text (for centering)
    i32 fontWidth = 10;
    i32 destX = screenCenterX - ((textLength / 2) * fontWidth);
    // TODO(cjh): @temp This should be a percentage of the screen height
    i32 beltHeight = 80;
    i32 destY = g->camera.viewport.h + g->camera.viewport.y - beltHeight;
    drawText(group, &g->fontMetadata, text, destX, destY);
}

internal void updateHero(RenderGroup *renderGroup, Entity* h, Input* input, Game* g)
{
    Map *map = g->currentMap;
    // TODO(cjh): Handle joystick and keyboard on separate paths
    Vec2 acceleration = {};
    acceleration.x = input->stickX;
    acceleration.y = input->stickY;

    if (input->keyDown[Key_Right])
    {
        acceleration.x = 1.0f;
    }
    if (input->keyDown[Key_Left])
    {
        acceleration.x = -1.0f;
    }
    if (input->keyDown[Key_Up])
    {
        acceleration.y = -1.0f;
    }
    if (input->keyDown[Key_Down])
    {
        acceleration.y = 1.0f;
    }

    if (acceleration.x == 0.0f && acceleration.y == 0.0f)
    {
        // TODO(cjh): should this really be false when skidding to a stop?
        h->isMoving = false;
    }
    else
    {
        u32 spriteHeight = h->spriteSheet.spriteHeight;
        if (acceleration.x > 0)
        {
            h->spriteRect.y = 0 * spriteHeight;
            h->direction = Direction_Right;
        }
        if (acceleration.x < 0)
        {
            h->spriteRect.y = 3 * spriteHeight;
            h->direction = Direction_Left;
        }
        if (acceleration.y < 0)
        {
            h->spriteRect.y = 1 * spriteHeight;
            h->direction = Direction_Up;
        }
        if (acceleration.y > 0)
        {
            h->spriteRect.y = 4 * spriteHeight;
            h->direction = Direction_Down;
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

    int iters = 4;
    int lastIter = iters - 1;
    Entity *interactableThisFrame = 0;
    for (int iter = 0; iter < iters; ++iter)
    {
        f32 tMin = 1.0f;
        Vec2 wallNormal = {};
        b32 hit = false;
        Vec2 desiredPosition = h->position + playerDelta;

        for (u32 entityIndex = 0; entityIndex < map->entityCount; ++entityIndex)
        {
            Entity *testEntity = &map->entities[entityIndex];

            // NOTE(cjh): Check for interactable entities, but only on last iteration
            if (iter == lastIter && !h->placingItem)
            {
                updateHeroInteractionRegion(h);
                switch (testEntity->type)
                {
                    case EntityType_Tile:
                    {
                        // Tile position is at center of tile
                        Rect tileBoundingBox = {(i32)testEntity->position.x - (i32)(0.5f*testEntity->width),
                                                (i32)testEntity->position.y - (i32)(0.5f*testEntity->height),
                                                testEntity->width, testEntity->height};

                        if (rectsOverlap(&h->heroInteractionRect, &tileBoundingBox))
                        {
                            if (isTileFlagSet(testEntity, TileProperty_Harvest))
                            {
                                interactableThisFrame = testEntity;
                                pushInteractionHint(renderGroup, g, "SPC to harvest");
                            }
                            // TODO(cjh): Do we need TileProperty_Interactive?
                            if (isTileFlagSet(testEntity, TileProperty_Interactive))
                            {
                                // NOTE(cjh): Light campfire
                                if (isTileFlagSet(testEntity, TileProperty_Campfire))
                                {
                                    if (!testEntity->isLit)
                                    {
                                        interactableThisFrame = testEntity;
                                        pushInteractionHint(renderGroup, g, "SPC to light campfire");
                                    }
                                    else
                                    {
                                        interactableThisFrame = testEntity;
                                        pushInteractionHint(renderGroup, g, "SPC to extinguish campfire");
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case EntityType_Harlod:
                    {
                        Rect harlodCollisionRegion = {(i32)(testEntity->position.x - 0.5f*testEntity->width),
                                                      (i32)(testEntity->position.y - testEntity->height),
                                                      testEntity->width, testEntity->height};

                        if (rectsOverlap(&h->heroInteractionRect, &harlodCollisionRegion))
                        {
                            interactableThisFrame = testEntity;
                            pushInteractionHint(renderGroup, g, "SPC to talk");
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            // NOTE(cjh): Collision checks
            f32 heightOffset = 0.5f*h->height;
            if (testEntity->type == EntityType_Tile)
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


            if (isTileFlagSet(testEntity, TileProperty_Quicksand))
            {
                b32 inQuicksand = false;
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
                    if (h->isMoving)
                    {
                        queueSound(&g->sounds, &g->mudSound);
                    }
                    // TODO(cjh): This isn't quite the effect I'm looking for.
                    h->velocity -= 0.98f*h->velocity;
                }
            }
            // TODO(cjh): Use tileFlags TileProperty_Solid instead of collides for tiles
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
    h->harvesting = input->keyPressed[Key_Space] || input->buttonPressed[Button_A];
    h->craftTree = input->keyPressed[Key_C] || input->buttonPressed[Button_Y];
    h->craftGlowJuice = input->keyPressed[Key_V] || input->buttonPressed[Button_B];

    CraftableItemType itemTypeToPlace = Craftable_None;
    if (input->keyPressed[Key_P] || input->buttonPressed[Button_X])
    {
        if (h->activeBeltItemIndex < (i32)h->beltItemCount)
        {
            BeltItem *item = h->beltItems + h->activeBeltItemIndex;
            if (item->count > 0)
            {
                h->placingItem = true;
                itemTypeToPlace = item->type;
            }
        }
    }

    if (h->harvesting && !h->placingItem)
    {
        // NOTE(cjh): It's possible that the movement applied this frame could
        // invalidate the interation region since the movement is applied after
        // interactables are checked. This could result in bugs if we have large
        // movements like teleportation

        // updateHeroInteractionRegion(h);
        if (interactableThisFrame)
        {
            Entity *testEntity = interactableThisFrame;
            switch (testEntity->type)
            {
                case EntityType_Tile:
                {
                    if (isTileFlagSet(testEntity, TileProperty_Harvest))
                    {
                        removeTileFlags(testEntity, TileProperty_Harvest);
                        testEntity->collides = false;
                        testEntity->spriteRect.x += testEntity->spriteRect.w;
                        h->inventory[testEntity->harvestedItem]++;
                    }
                    // TODO(cjh): Do we need TileProperty_Interactive?
                    if (isTileFlagSet(testEntity, TileProperty_Interactive))
                    {
                        // NOTE(cjh): Light campfire
                        if (isTileFlagSet(testEntity, TileProperty_Campfire))
                        {
                            if (!testEntity->isLit)
                            {
                                addFlame(g, testEntity->position);
                                testEntity->isLit = true;
                            }
                            else
                            {
                                removeEntity(map, testEntity->position, TileProperty_Flame);
                                testEntity->isLit = false;
                            }
                        }
                    }
                    break;
                }
                case EntityType_Harlod:
                {
                    if (!testEntity->dialogueFile.contents)
                    {
                        testEntity->dialogueFile = platform.readEntireFile("dialogues/harlod_dialogues.txt");
                        // TODO(cjh): Need to null terminate everything. This will change. We
                        // want to parse files for strings and tokenize, etc. For now it's hard coded
                        testEntity->dialogueFile.contents[9] = '\0';
                    }
                    startDialogueMode(g, (char*)testEntity->dialogueFile.contents);
                break;
                }
                default:
                    break;
            }
        }
    }

    if (h->craftTree)
    {
        craftItem(h, Craftable_Tree);
    }
    if (h->craftGlowJuice)
    {
        craftItem(h, Craftable_Glow_Juice);
    }

    if (h->placingItem)
    {
        pushInteractionHint(renderGroup, g, "SPC to place tile");
        if (!h->tileToPlace)
        {
            Map *m = g->currentMap;
            h->tileToPlace = addEntity(m);
            Entity *tile = h->tileToPlace;
            // TODO(cjh): @temp No hard coded values
            tile->width = 80;
            tile->height = 80;
            tile->isVisible = true;
            tile->craftableItem = itemTypeToPlace;

            switch (itemTypeToPlace)
            {
                case Craftable_Tree:
                    // TODO(cjh): This is called again in placeItem
                    initEntitySpriteSheet(tile, g->harvestableTreeTexture, 3, 1);
                    break;
                case Craftable_Glow_Juice:
                    initEntitySpriteSheet(tile, g->glowTreeTexture, 2, 1);
                    break;
            }
        }

        h->tileToPlace->position = getTilePlacementPosition(g, h);
        h->tileToPlace->validPlacement = isValidTilePlacment(map, h->tileToPlace);
    }

    if (h->harvesting && h->placingItem)
    {
        placeItem(g, h);
        // NOTE(cjh): So we don't draw the interactionRect while placing a tile
        h->harvesting = false;
    }

    if (input->keyPressed[Key_X] || input->buttonPressed[Button_RTrigger])
    {
        h->activeBeltItemIndex++;
        h->activeBeltItemIndex %= ArrayCount(h->beltItems);
    }
    if (input->keyPressed[Key_Z] || input->buttonPressed[Button_LTrigger])
    {
        --h->activeBeltItemIndex;
        h->activeBeltItemIndex %= ArrayCount(h->beltItems);
    }
}

