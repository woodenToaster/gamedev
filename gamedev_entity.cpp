#include "gamedev_entity.h"
#include "gamedev_camera.h"
#include "gamedev_plan.h"

static Circle heroInteractionRegion = {};

#if 0
void initEntityPixelData(Entity* e, SDL_Surface *s)
{
    e->pixel_data = (u8*)malloc(sizeof(u8) * e->sprite_rect.w * e->sprite_rect.h);
    for (int y = 0; y < e->sprite_rect.h; ++y)
    {
        for (int x = 0; x < e->sprite_rect.w; ++x)
        {
            int pd_idx = y * e->sprite_rect.w + x;
            if ((((u32*)s->pixels)[y * s->w + x] & s->format->Amask) == 0)
            {
                e->pixel_data[pd_idx] = 0;
            }
            else
            {
                e->pixel_data[pd_idx] = 1;
            }
        }
    }
}
#endif

void initEntitySpriteSheet(Entity* e, const char* path, int num_x, int num_y, SDL_Renderer* renderer)
{
    loadSpriteSheet(&e->sprite_sheet, path, num_x, num_y, renderer);
    e->sprite_rect.w = e->sprite_sheet.sprite_width;
    e->sprite_rect.h = e->sprite_sheet.sprite_height;
    e->sprite_sheet.scale = 1;
}

void initEntityWidthHeight(Entity *e)
{
    e->width = e->sprite_sheet.sprite_width * e->sprite_sheet.scale;
    e->height = e->sprite_sheet.sprite_height * e->sprite_sheet.scale;
}

bool32 entityIsHero(Entity* e)
{
    bool32 result = 0;
    if (e)
    {
        result = e->type == ET_HERO;
    }
    return result;
}

bool32 entityIsHarlod(Entity *e)
{
    bool32 result = 0;
    if (e)
    {
        result = e->type == ET_HARLOD;
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
    if (e->active)
    {
        SDL_Rect dest = {};
        e->sprite_rect.x = e->sprite_rect.w * e->animation.current_frame;
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
        SDL_RenderCopy(g->renderer, e->sprite_sheet.sheet, &e->sprite_rect, &dest);

        // Hero interactionRect
        if (e->harvesting)
        {
            renderFilledRect(g->renderer, &e->heroInteractionRect, g->colors[COLOR_DARK_ORANGE]);
        }
    }
}

SDL_Rect getEntitiesOverlapBox(Entity* e1, Entity* e2)
{
    SDL_Rect overlap_box;
    if (e1->bounding_box.x > e2->bounding_box.x)
    {
        overlap_box.x = e1->bounding_box.x;
        overlap_box.w = e2->bounding_box.x + e2->bounding_box.w - e1->bounding_box.x;
        overlap_box.w = minInt32(overlap_box.w, e1->bounding_box.w);
    }
    else
    {
        overlap_box.x = e2->bounding_box.x;
        overlap_box.w = e1->bounding_box.x + e1->bounding_box.w - e2->bounding_box.x;
        overlap_box.w = minInt32(overlap_box.w, e2->bounding_box.w);
    }

    if (e1->bounding_box.y > e2->bounding_box.y)
    {
        overlap_box.y = e1->bounding_box.y;
        overlap_box.h = e2->bounding_box.y + e2->bounding_box.h - e1->bounding_box.y;
        overlap_box.h = minInt32(overlap_box.h, e1->bounding_box.h);
    }
    else
    {
        overlap_box.y = e2->bounding_box.y;
        overlap_box.h = e1->bounding_box.y + e1->bounding_box.h - e2->bounding_box.y;
        overlap_box.h = minInt32(overlap_box.h, e2->bounding_box.h);
    }

    return overlap_box;
}

Tile* getTileAtEntityPosition(Entity* e, Map* map)
{
    SDL_Rect tile_under_entity = {
        (int)((e->collision_pt.x / map->tile_width) * map->tile_width),
        (int)((e->collision_pt.y / map->tile_height) * map->tile_height)
    };
    int map_coord_x = tile_under_entity.y / map->tile_height;
    int map_coord_y = tile_under_entity.x / map->tile_width;
    int tile_index = map_coord_x * map->cols + map_coord_y;

    return map->tiles[tile_index];
}

void reverseEntityDiretion(Entity* e)
{
    // TODO: Try to use entity direction directly instead of plan.mv_dir
    switch (e->plan.mv_dir)
    {
    case CARDINAL_NORTH:
        e->plan.mv_dir = CARDINAL_SOUTH;
        break;
    case CARDINAL_SOUTH:
        e->plan.mv_dir = CARDINAL_NORTH;
        break;
    case CARDINAL_EAST:
        e->plan.mv_dir = CARDINAL_WEST;
        break;
    case CARDINAL_WEST:
        e->plan.mv_dir = CARDINAL_EAST;
        break;
    }
}

#if 0
void checkEntityCollisionsWithTiles(Entity* e, Map* map, SDL_Rect* saved_pos)
{
    Tile* current_tile = getTileAtEntityPosition(e, map);

    if (isSolidTile(current_tile))
    {
        // Collision with an impenetrable tile. Revert to original position
        e->dest_rect = *saved_pos;
        switch (e->type)
        {
        case ET_BUFFALO:
            reverseEntityDiretion(e);
            break;
        default:
            break;
        }
    }
}
#endif

#if 0
bool32 checkEntitiesPixelCollision(Entity* e1, Entity* e2, SDL_Rect* overlap_box)
{
    for (int y = overlap_box->y; y < overlap_box->y + overlap_box->h; ++y)
    {
        for (int x = overlap_box->x; x < overlap_box->x + overlap_box->w; ++x)
        {
            // Convert overlap_box coordinates to pixel coordinates within the sprites
            int e1_index_x = x - e1->dest_rect.x;
            int e1_index_y = y - e1->dest_rect.y;
            int e1_index = e1_index_y * e1->sprite_rect.w + e1_index_x;

            int e2_index_x = x - e2->dest_rect.x;
            int e2_index_y = y - e2->dest_rect.y;
            int e2_index = e2_index_y * e2->sprite_rect.w + e2_index_x;

            if (e1->pixel_data[e1_index] == 1 && e2->pixel_data[e2_index] == 1)
            {
                return true;
            }
        }
    }
    return false;
}
#endif

#if 0
void DEBUGprintEntityPixels(Entity* e)
{
    for (int y = 0; y < e->sprite_rect.h; ++y)
    {
        for (int x = 0; x < e->sprite_rect.w; ++x)
        {
            printf("%x", e->pixel_data[y * e->sprite_rect.w + x]);
        }
        printf("\n");
    }
}
#endif

void checkEntityCollisionsWithEntities(Entity* e, Game* game)
{
    u32 entityCount = game->current_map->active_entities.count;
    for (u32 y = 0; y < entityCount; ++y) {
        Entity* otherEntity = game->current_map->active_entities.entities[y];
        if (e == otherEntity) {
            continue;
        }
        if (otherEntity->active && rectsOverlap(&e->bounding_box, &otherEntity->bounding_box))
        {
            // Draw bounding box overlap
            SDL_Rect overlap_box = getEntitiesOverlapBox(e, otherEntity);
            renderFilledRect(game->renderer, &overlap_box, game->colors[COLOR_MAGENTA]);

            switch (e->type)
            {
            case ET_BUFFALO:
            {
                reverseEntityDiretion(e);
                break;
            }
            default:
                break;
            }
        }
    }
}

void setEntityCollisionPoint(Entity* e)
{
    e->collision_pt.x = (int)e->position.x + (i32)(e->width / 2.0);
    e->collision_pt.y = (int)e->position.y + e->height - e->collision_pt_offset;
}

void updateEntity(Entity* e, Map* map, u32 last_frame_duration)
{
    (void)last_frame_duration;
    (void)map;
    (void)e;
    // if (e->has_plan) {
    //     plan_update(e, last_frame_duration);
    //     if (e->can_move && e->active)
    //     {
    //         SDL_Rect saved_position = e->dest_rect;
    //         moveEntityInDirection(e, e->plan.mv_dir);
    //         updateAnimation(&e->animation, last_frame_duration, true);
    //         setEntityCollisionPoint(e);
    //         checkEntityCollisionsWithTiles(e, map, &saved_position);
    //         // entity_check_collisions_with_entities(e, map, &saved_position);
    //         setEntityCollisionPoint(e);
    //     }
    // }
    // e->bounding_box.x = (int)e->position.x + e->bb_x_offset;
    // e->bounding_box.y = (int)e->position.y + e->bb_y_offset;
}

void destroyEntity(Entity* e)
{
    destroySpriteSheet(&e->sprite_sheet);
    // free(e->pixel_data);
}


void updateEntityList(Game *g)
{
    EntityList *el = &g->current_map->active_entities;
    Map *map = g->current_map;
    for (u32 y = 0; y < el->count; ++y)
    {
        updateEntity(el->entities[y], map, g->dt);
    }
}

void drawEntityList(Game* g)
{
    EntityList *el = &g->current_map->active_entities;
    for (u32 y = 0; y < el->count; ++y)
    {
        drawEntity(el->entities[y], g);
    }
}

void destroyEntityList(EntityList* el)
{
    for (u32 y = 0; y < el->count; ++y)
    {
        destroyEntity(el->entities[y]);
    }
}

void checkHeroCollisionsWithEntities(Hero *h, Game *g, SDL_Rect saved_position)
{
    (void)saved_position;
    u32 entityCount = g->current_map->active_entities.count;
    for (u32 y = 0; y < entityCount; ++y)
    {
        Entity* otherEntity = g->current_map->active_entities.entities[y];
        if (entityIsHero(otherEntity))
        {
            continue;
        }
        if (otherEntity->active && rectsOverlap(&h->e.bounding_box, &otherEntity->bounding_box))
        {
            // TODO(chj): Enable pixel collision if we don't have to duplicate pixel data
            // if (checkEntitiesPixelCollision(&h->e, otherEntity, &overlap_box))
            // {
            // h->e.position.x = (f32)saved_position.x;
            // h->e.position.y = (f32)saved_position.y;
            // setEntityCollisionPoint(&h->e);

            // h->e.velocity.x = 0.0f;
            // h->e.velocity.y = 0.0f;
            // }
        }
    }
}

void checkHeroCollisionsWithTiles(Hero* h, Game* game)
{
    Tile* current_tile = getTileAtEntityPosition(&h->e, game->current_map);

    f32 quicksandValue = 10.0f;
    if (isSlowTile(current_tile) && !h->inQuicksand)
    {
        h->e.speed *= 1 / quicksandValue;
        h->inQuicksand = true;
        if (h->isMoving)
        {
            sound_queue(global_sounds[SOUND_MUD], game->sounds);
        }
    }
    else if (h->inQuicksand)
    {
        h->e.speed *= quicksandValue;
        h->inQuicksand = false;
    }
    if (isWarpTile(current_tile))
    {
        map_do_warp(game);
        h->e.position.x = (f32)h->e.starting_pos.x;
        h->e.position.y = (f32)h->e.starting_pos.y;
        h->e.position.x = (f32)h->e.starting_pos.x;
        h->e.position.y = (f32)h->e.starting_pos.y;
    }
}

void clampHeroToMap(Hero* h, Map* map)
{
    h->e.position.x = clampFloat(h->e.position.x, 0, (f32)map->width_pixels - 0.5f*h->e.width);
    h->e.position.y = clampFloat(h->e.position.y, 0, (f32)map->height_pixels - 0.5f*h->e.height);
}

void harvestTile(Hero *h, Game *g, Tile *tileToHarvest)
{
    destroyTile(tileToHarvest);
    // TODO(chj): Don't re-init?
    initTile(tileToHarvest, tile_properties[TP_NONE], g->colors[COLOR_NONE],
              g->renderer, "sprites/tree_stump.png");
    tileToHarvest->active = true;
    tileToHarvest->harvested = true;
    setTileSpriteSize(tileToHarvest, 64, 64);

    // Update inventory
    h->inventory[tileToHarvest->harvestedItem]++;
}

void heroInteract(Hero *h, Game *g)
{
    // Check for harvestable tile
    Point pointToHarvest = {(i32)h->e.position.x, (i32)h->e.position.y};
    i32 interactionRectWidth = h->e.width;
    i32 interactionRectHeight = h->e.height;

    switch (h->e.direction)
    {
    case DIR_UP:
        interactionRectWidth = h->e.height;
        interactionRectHeight = h->e.width;
        pointToHarvest.x -= (i32)(0.5f*interactionRectWidth);
        pointToHarvest.y -= (interactionRectHeight + interactionRectWidth);
        break;
    case DIR_DOWN:
        interactionRectWidth = h->e.height;
        interactionRectHeight = h->e.width;
        pointToHarvest.x -= (i32)(0.5f*interactionRectWidth);
        break;
    case DIR_LEFT:
        pointToHarvest.x -= (i32)(1.5f*h->e.width);
        pointToHarvest.y -= h->e.height;
        break;
    case DIR_RIGHT:
        pointToHarvest.x += (i32)(0.5f*h->e.width);
        pointToHarvest.y -= h->e.height;
        break;
    default:
        break;
    }

    h->e.heroInteractionRect = {pointToHarvest.x, pointToHarvest.y, interactionRectWidth, interactionRectHeight};

#if 0
    // Tile *tileToHarvest = map_get_tile_at_point(g->current_map, pointToHarvest);
    Tile *tileToHarvest = getTileAtPosition(g->current_map, pointToHarvest);
    if (tileToHarvest && tileToHarvest->is_harvestable && !tileToHarvest->harvested)
    {
        harvestTile(h, g, tileToHarvest);
    }

    // Check for entities to interact with in a circle
    Vec2 heroCenter = {};
    heroCenter.x = h->e.position.x + (0.5f * h->e.width);
    heroCenter.y = h->e.position.y + (0.5f * h->e.height);
    heroInteractionRegion.center = heroCenter;
    heroInteractionRegion.radius = maxFloat32((f32)h->e.width, (f32)h->e.height) * 0.666666f;

    // See if there is an entity there
    for (u32 entityIndex = 0; entityIndex < g->current_map->active_entities.count; ++entityIndex)
    {
        Entity *e = g->current_map->active_entities.entities[entityIndex];
        if (entityIsHarlod(e))
        {
            if(circleOverlapsRect(&heroInteractionRegion, &e->bounding_box))
            {
                // TODO(chj): Find a cleaner way to do this
                interactWithHero *f = (interactWithHero*)((u8*)e + offsetof(Harlod, onHeroInteract));
                (*f)(e, h, g);
            }
        }
    }

    // Check for interactive tiles
    Point heroCenterPoint = {(i32)heroCenter.x, (i32)heroCenter.y};
    const u32 stencilSize = 9;
    Tile *tileStencil[stencilSize] = {};
    u32 stencilIndex = 0;
    for (i32 tileIndexY = heroCenterPoint.y - g->current_map->tile_height;
         tileIndexY <= heroCenterPoint.y + (i32)g->current_map->tile_height;
         tileIndexY += g->current_map->tile_height)
    {
        for (i32 tileIndexX = heroCenterPoint.x - g->current_map->tile_width;
             tileIndexX <= heroCenterPoint.x + (i32)g->current_map->tile_width;
             tileIndexX += g->current_map->tile_width)
        {
            SDL_Rect tileRect = {tileIndexX, tileIndexY,
                                 (i32)g->current_map->tile_width, (i32)g->current_map->tile_height};
            if (circleOverlapsRect(&heroInteractionRegion, &tileRect))
            {
                Point tileIndexPoint = {tileIndexX, tileIndexY};
                tileStencil[stencilIndex] = map_get_tile_at_point(g->current_map, tileIndexPoint);
            }
            stencilIndex++;
        }
    }

    for (u32 y = 0; y < stencilSize; ++y)
    {
        if (tileStencil[y] && tile_is_interactive(tileStencil[y]) && tileStencil[y]->onHeroInteract)
        {
            tileStencil[y]->onHeroInteract(tileStencil[y], h);
        }
    }
#endif
}

void harlodInteractWithHero(Entity *e, Hero *h, Game *g)
{
    (void)h;
    if (!e->dialogFile.contents)
    {
        e->dialogFile = readEntireFile("dialogs/harlod_dialogs.txt");
        // TODO(chj): Need to null terminate everything. This will change. We
        // want to parse files for strings and tokenize, etc. For now it's hard coded
        e->dialogFile.contents[9] = '\0';
        // TODO(chj): Free dialog.contents
    }
    startDialogMode(g, (char*)e->dialogFile.contents);
}

internal void craftItem(Hero *h, CraftableItem item)
{
    switch (item)
    {
    case CRAFTABLE_TREE:
    {
        u32 leavesRequiredForTree = 2;
        if (h->inventory[INV_LEAVES] >= leavesRequiredForTree)
        {
            h->inventory[INV_LEAVES] -= leavesRequiredForTree;
            ++h->inventory[INV_TREES];
        }
        break;
    }
    default:
        break;
    }
}

internal void placeItem(Game *g, Hero *h, CraftableItem item)
{
    (void)h;
    if (item == CRAFTABLE_TREE)
    {
        Tile *t = (Tile*)malloc(sizeof(Tile));
        // TODO(chj): Free this
        // TODO(chj): Share textures. Ref counting?
        t->width = t->height = 80;
        initTile(t, tile_properties[TP_HARVEST] | tile_properties[TP_SOLID],
                 g->colors[COLOR_NONE], g->renderer, "sprites/tree.png");
        setTileSpriteSize(t, 64, 64);
        t->active = true;
        t->is_harvestable = true;
        t->harvestedItem = INV_LEAVES;
        // TODO(chj): Free what was there before

        int locationToPlaceTile = 30;
        g->current_map->tiles[locationToPlaceTile] = t;
    }
}

#if 0
internal bool32 isInMap(Game *g, Vec2 pos)
{
    bool32 result = true;
    if (pos.x < 0 || pos.x > g->current_map->width_pixels - 1 ||
        pos.y < 0 || pos.y > g->current_map->height_pixels - 1)
    {
        result = false;
    }
    return result;
}

internal bool32 canMoveToPosition(Game *g, Vec2 pos)
{
    bool result = false;
    Map *m = g->current_map;
    Tile *tile = getTileAtPosition(m, pos);
    if (tile)
    {
        result = true;
        if (isSolidTile(tile))
        {
            result = false;
        }
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

internal void updateHero(Hero* h, Input* input, Game* g)
{
    Map *map = g->current_map;
    // TODO(chj): Handle joystick and keyboard on separate paths
    Vec2 acceleration = {};
    acceleration.x = input->stickX;
    acceleration.y = input->stickY;

    if (input->key_down[KEY_RIGHT])
    {
        acceleration.x = 1.0f;
    }
    if (input->key_down[KEY_LEFT])
    {
        acceleration.x = -1.0f;
    }
    if (input->key_down[KEY_UP])
    {
        acceleration.y = -1.0f;
    }
    if (input->key_down[KEY_DOWN])
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
        u32 spriteHeight = h->e.sprite_sheet.sprite_height;
        if (acceleration.x > 0)
        {
            h->e.sprite_rect.y = 0 * spriteHeight;
            h->e.direction = DIR_RIGHT;
        }
        if (acceleration.x < 0)
        {
            h->e.sprite_rect.y = 3 * spriteHeight;
            h->e.direction = DIR_LEFT;
        }
        if (acceleration.y < 0)
        {
            h->e.sprite_rect.y = 1 * spriteHeight;
            h->e.direction = DIR_UP;
        }
        if (acceleration.y > 0)
        {
            h->e.sprite_rect.y = 4 * spriteHeight;
            h->e.direction = DIR_DOWN;
        }

        h->isMoving = true;
    }

    // Diagonal movement
    if (acceleration.x != 0.0f && acceleration.y != 0.0f)
    {
        acceleration *= 0.707186781187f;
    }

    acceleration *= h->e.speed;
    // Friction
    acceleration -= 8 * h->e.velocity;

    f32 dt = (f32)g->dt / 1000.0f;
    Vec2 oldPosition = h->e.position;
    Vec2 playerDelta = (0.5 * acceleration * square(dt)) + (h->e.velocity * dt);
    Vec2 newPosition =  playerDelta + h->e.position;
    h->e.velocity = (acceleration * dt) + h->e.velocity;

    if (!h->isMoving)
    {
        h->e.animation.current_frame = 0;
        h->e.sprite_rect.x = 0;
    }

    for (int iter = 0; iter < 4; ++iter)
    {
        f32 tMin = 1.0f;
        Vec2 wallNormal = {};
        bool32 hit = false;
        Vec2 desiredPosition = h->e.position + playerDelta;

        for (u32 entityIndex = 0; entityIndex < map->entityCount; ++entityIndex)
        {
            Entity *testEntity = &map->entities[entityIndex];

            f32 heightOffset = 0.5f*h->e.height;
            if (testEntity->type == ET_TILE)
            {
                // TODO(chj): Clean this up. Need special handling based off where position is.
                // It's at the center of tiles but the bottom of entities
                heightOffset = 0.0f;
            }
            f32 minX = maxFloat32(0.0f, testEntity->position.x - 0.5f*testEntity->width - 0.5f*h->e.width);
            f32 minY = maxFloat32(0.0f, testEntity->position.y - 0.5f*testEntity->height - heightOffset);
            Vec2 minCorner = {minX, minY};
            f32 maxX = minFloat32((f32)map->width_pixels,
                                  testEntity->position.x + 0.5f*testEntity->width + 0.5f*h->e.width);
            f32 maxY = minFloat32((f32)map->height_pixels,
                                  testEntity->position.y + 0.5f*testEntity->height + h->e.height);
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
                    if (h->isMoving)
                    {
                        sound_queue(global_sounds[SOUND_MUD], g->sounds);
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

        h->e.position += (tMin * playerDelta);
        h->e.velocity -= 1 * vec2_dot(&h->e.velocity, &wallNormal) * wallNormal;
        playerDelta = desiredPosition - h->e.position;
        playerDelta -= 1 * vec2_dot(&playerDelta, &wallNormal) * wallNormal;
    }

    clampHeroToMap(h, map);

    h->swingClub = input->key_pressed[KEY_F];
    h->e.harvesting = input->key_pressed[KEY_SPACE] || input->button_pressed[BUTTON_A];
    h->craft = input->key_pressed[KEY_C];
    h->place = input->key_pressed[KEY_P];

    if (h->e.harvesting)
    {
        heroInteract(h, g);
    }
    if (h->craft)
    {
        CraftableItem item = CRAFTABLE_TREE;
        craftItem(h, item);
    }
    if (h->place)
    {
        CraftableItem item = CRAFTABLE_TREE;
        placeItem(g, h, item);
    }
}

Entity createBuffalo(f32 starting_x, f32 starting_y, SDL_Renderer* renderer)
{
    Entity buffalo = {};
    initEntitySpriteSheet(&buffalo, "sprites/Buffalo.png", 4, 1, renderer);
    initEntityWidthHeight(&buffalo);
    buffalo.starting_pos = {starting_x, starting_y};
    buffalo.position = buffalo.starting_pos;
    // initEntityDest(&buffalo);
    // setEntityCollisionPoint(&buffalo);
    buffalo.bounding_box = {0, 0, 50, 50};
    buffalo.bb_x_offset = 10;
    buffalo.bb_y_offset = 10;
    buffalo.speed = 3;
    buffalo.type = ET_BUFFALO;
    buffalo.can_move = true;
    buffalo.collision_pt_offset = 32;
    initAnimation(&buffalo.animation, 4, 100);
    buffalo.plan = {};
    buffalo.has_plan = true;
    buffalo.plan.move_delay = (rand() % 2000) + 1000;
    buffalo.plan.mv_dir = (CardinalDir)(rand() % 4);
    return buffalo;
}

