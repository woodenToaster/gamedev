#include "gamedev_entity.h"
#include "gamedev_camera.h"
#include "gamedev_plan.h"

static Circle heroInteractionRegion = {};

void initEntityPixelData(Entity* e)
{
    e->pixel_data = (u8*)malloc(sizeof(u8) * e->sprite_rect.w * e->sprite_rect.h);
    SDL_Surface* s = e->sprite_sheet.surface;
    for (int i = 0; i < e->sprite_rect.h; ++i)
    {
        for (int j = 0; j < e->sprite_rect.w; ++j)
        {
            int pd_idx = i * e->sprite_rect.w + j;
            if ((((u32*)s->pixels)[i * s->w + j] & s->format->Amask) == 0)
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

void initEntitySpriteSheet(Entity* e, const char* path, int num_x, int num_y, SDL_Renderer* renderer)
{
    sprite_sheet_load(&e->sprite_sheet, path, num_x, num_y, renderer);
    e->sprite_rect.w = e->sprite_sheet.sprite_width;
    e->sprite_rect.h = e->sprite_sheet.sprite_height;
    // TODO: Do this for each sprite in the sheet?
    initEntityPixelData(e);
}

void setEntityStartingPos(Entity* e, int x, int y)
{
    e->starting_pos.x = x;
    e->starting_pos.y = y;
}

void setEntityBoundingBoxOffset(Entity* e, int x, int y, int w, int h)
{
    e->bb_x_offset = x;
    e->bb_y_offset = y;
    e->bb_w_offset = w;
    e->bb_h_offset = h;
}

void initEntityDest(Entity* e)
{
    e->dest_rect.x = e->starting_pos.x;
    e->dest_rect.y = e->starting_pos.y;
    e->dest_rect.w = e->sprite_sheet.sprite_width;
    e->dest_rect.h = e->sprite_sheet.sprite_height;
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

void drawEntity(Entity* e, Game* g)
{
    if (e->active)
    {
        e->sprite_rect.x = e->sprite_rect.w * e->animation.current_frame;
        SDL_RenderCopy(g->renderer, e->sprite_sheet.sheet, &e->sprite_rect, &e->dest_rect);
    }

#if 1
    // Draw bounding box
    SDL_Rect bb_top;
    SDL_Rect bb_bottom;
    SDL_Rect bb_left;
    SDL_Rect bb_right;
    int bb_line_width = 2;

    bb_top.x = e->bounding_box.x;
    bb_top.y = e->bounding_box.y;
    bb_top.w = e->bounding_box.w;
    bb_top.h = bb_line_width;

    bb_left.x = e->bounding_box.x;
    bb_left.y = e->bounding_box.y;
    bb_left.w = bb_line_width;
    bb_left.h = e->bounding_box.h;

    bb_right.x = e->bounding_box.x + e->bounding_box.w;
    bb_right.y = e->bounding_box.y;
    bb_right.w =bb_line_width;
    bb_right.h = e->bounding_box.h;

    bb_bottom.x = e->bounding_box.x;
    bb_bottom.y = e->bounding_box.y + e->bounding_box.h;
    bb_bottom.w = e->bounding_box.w + bb_line_width;
    bb_bottom.h = bb_line_width;

    u32 magenta = g->colors[COLOR_MAGENTA];
    if (e->active)
    {
        renderFilledRect(g->renderer, &bb_top, magenta);
        renderFilledRect(g->renderer, &bb_left, magenta);
        renderFilledRect(g->renderer, &bb_right, magenta);
        renderFilledRect(g->renderer, &bb_bottom, magenta);
    }
#endif
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

void moveEntityInDirection(Entity* e, CardinalDir d)
{
    switch(d)
    {
    case CARDINAL_NORTH:
        e->dest_rect.y -= (int)e->speed;
        break;
    case CARDINAL_SOUTH:
        e->dest_rect.y +=(int)e->speed;
        break;
    case CARDINAL_WEST:
        e->dest_rect.x -= (int)e->speed;
        break;
    case CARDINAL_EAST:
        e->dest_rect.x += (int)e->speed;
        break;
    default:
        return;
    }
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

void checkEntityCollisionsWithTiles(Entity* e, Map* map, SDL_Rect* saved_pos)
{
    Tile* current_tile = getTileAtEntityPosition(e, map);

    if (tile_is_solid(current_tile))
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

bool32 checkEntitiesPixelCollision(Entity* e1, Entity* e2, SDL_Rect* overlap_box)
{
    for (int i = overlap_box->y; i < overlap_box->y + overlap_box->h; ++i)
    {
        for (int j = overlap_box->x; j < overlap_box->x + overlap_box->w; ++j)
        {
            // Convert overlap_box coordinates to pixel coordinates within the sprites
            int e1_index_x = j - e1->dest_rect.x;
            int e1_index_y = i - e1->dest_rect.y;
            int e1_index = e1_index_y * e1->sprite_rect.w + e1_index_x;

            int e2_index_x = j - e2->dest_rect.x;
            int e2_index_y = i - e2->dest_rect.y;
            int e2_index = e2_index_y * e2->sprite_rect.w + e2_index_x;

            if (e1->pixel_data[e1_index] == 1 && e2->pixel_data[e2_index] == 1)
            {
                return true;
            }
        }
    }
    return false;
}

#ifdef DEBUG
void DEBUGprintEntityPixels(Entity* e)
{
    for (int i = 0; i < e->sprite_rect.h; ++i)
    {
        for (int j = 0; j < e->sprite_rect.w; ++j)
        {
            printf("%x", e->pixel_data[i * e->sprite_rect.w + j]);
        }
        printf("\n");
    }
}
#endif

void checkEntityCollisionsWithEntities(Entity* e, Game* game)
{
    u32 entityCount = game->current_map->active_entities.count;
    for (u32 i = 0; i < entityCount; ++i) {
        Entity* otherEntity = game->current_map->active_entities.entities[i];
        if (e == otherEntity) {
            continue;
        }
        if (otherEntity->active && rectsOverlap(&e->bounding_box, &otherEntity->bounding_box))
        {
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
    e->collision_pt.y = e->dest_rect.y + e->dest_rect.h - e->collision_pt_offset;
    e->collision_pt.x = e->dest_rect.x + (i32)(e->dest_rect.w / 2.0);
}

void updateEntity(Entity* e, Map* map, u32 last_frame_duration)
{
    // if (e->has_plan) {
    //     plan_update(e, last_frame_duration);
    //     if (e->can_move && e->active)
    //     {
    //         SDL_Rect saved_position = e->dest_rect;
    //         moveEntityInDirection(e, e->plan.mv_dir);
    //         updateAnimation(&e->animation, last_frame_duration, GD_TRUE);
    //         setEntityCollisionPoint(e);
    //         checkEntityCollisionsWithTiles(e, map, &saved_position);
    //         // entity_check_collisions_with_entities(e, map, &saved_position);
    //         setEntityCollisionPoint(e);
    //     }
    // }
    e->bounding_box.x = e->dest_rect.x + e->bb_x_offset;
    e->bounding_box.y = e->dest_rect.y + e->bb_y_offset;
    e->bounding_box.w = e->dest_rect.w - e->bb_w_offset;
    e->bounding_box.h = e->dest_rect.h - e->bb_h_offset;
}

void destroyEntity(Entity* e)
{
    sprite_sheet_destroy(&e->sprite_sheet);
    free(e->pixel_data);
}


void updateEntityList(EntityList* el, Map* map, u32 last_frame_duration)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        updateEntity(el->entities[i], map, last_frame_duration);
    }
}

void drawEntityList(EntityList* el, Game* g)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        drawEntity(el->entities[i], g);
    }
}

void destroyEntityList(EntityList* el)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        destroyEntity(el->entities[i]);
    }
}

void checkHeroCollisionsWithEntities(Hero *h, Game *g, SDL_Rect saved_position)
{
    u32 entityCount = g->current_map->active_entities.count;
    for (u32 i = 0; i < entityCount; ++i)
    {
        Entity* otherEntity = g->current_map->active_entities.entities[i];
        if (entityIsHero(otherEntity))
        {
            continue;
        }
        if (otherEntity->active && rectsOverlap(&h->e.bounding_box, &otherEntity->bounding_box))
        {
            SDL_Rect overlap_box = getEntitiesOverlapBox(&h->e, otherEntity);
            if (checkEntitiesPixelCollision(&h->e, otherEntity, &overlap_box))
            {
                h->e.dest_rect = saved_position;
                h->e.position.x = (f32)saved_position.x;
                h->e.position.y = (f32)saved_position.y;

                h->e.velocity.x = 0.0f;
                h->e.velocity.y = 0.0f;
            }
        }
    }
}

void checkHeroCollisionsWithTiles(Hero* h, Game* game, SDL_Rect saved_position)
{
    Tile* current_tile = getTileAtEntityPosition(&h->e, game->current_map);

    if (tile_is_solid(current_tile))
    {
        h->e.dest_rect = saved_position;
        h->e.position.x = (f32)saved_position.x;
        h->e.position.y = (f32)saved_position.y;

        h->e.velocity.x = 0.0f;
        h->e.velocity.y = 0.0f;
    }
    if (tile_is_slow(current_tile) && !h->inQuicksand)
    {
        h->speed -= 990;
        h->inQuicksand = GD_TRUE;
        if (h->isMoving)
        {
            sound_queue(global_sounds[SOUND_MUD], game->sounds);
        }
    }
    else if (h->inQuicksand)
    {
        h->speed += 990;
        h->inQuicksand = GD_FALSE;
    }
    if (tile_is_warp(current_tile))
    {
        map_do_warp(game);
        h->e.dest_rect.x = (int)h->e.starting_pos.x;
        h->e.dest_rect.y = (int)h->e.starting_pos.y;
        h->e.position.x = (f32)h->e.starting_pos.x;
        h->e.position.y = (f32)h->e.starting_pos.y;
    }
}

void clampHeroToMap(Hero* h, Map* map)
{
    h->e.position.x = clampFloat(h->e.position.x, 0, (f32)map->width_pixels - h->e.dest_rect.w);
    h->e.position.y = clampFloat(h->e.position.y, 0, (f32)map->height_pixels - h->e.dest_rect.h);
}

void processInput(Hero *h, Input* input, f32 dt)
{
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

    // TODO(chj): Sprite selection doesn't belong in here
    if (acceleration.x > 0)
    {
        h->e.sprite_rect.y = 2 * h->e.sprite_sheet.sprite_height;
    }
    if (acceleration.x < 0)
    {
        h->e.sprite_rect.y = 1 * h->e.sprite_sheet.sprite_height;
    }
    if (acceleration.y < 0)
    {
        h->e.sprite_rect.y = 3 * h->e.sprite_sheet.sprite_height;
    }
    if (acceleration.y > 0)
    {
        h->e.sprite_rect.y = 0 * h->e.sprite_sheet.sprite_height;
    }

    // Diagonal movement
    if (acceleration.x != 0.0f && acceleration.y != 0.0f)
    {
        acceleration *= 0.707186781187f;
    }

    acceleration *= h->e.speed;

    // Friction
    acceleration -= 4 * h->e.velocity;

    h->e.position = (0.5 * acceleration * square(dt)) +
                    (h->e.velocity * dt) +
                    h->e.position;

    h->e.velocity = (acceleration * dt) + h->e.velocity;

    h->swingClub = input->key_pressed[KEY_F];
    h->harvest = input->key_pressed[KEY_SPACE] || input->button_pressed[BUTTON_A];

    h->e.dest_rect.x = (int)(h->e.position.x);
    h->e.dest_rect.y = (int)(h->e.position.y);
}

void harvestTile(Hero *h, Game *g, Tile *tileToHarvest)
{
    destroyTile(tileToHarvest);
    // TODO(chj): Don't re-init?
    initTile(tileToHarvest, tile_properties[TP_NONE], g->colors[COLOR_NONE],
              g->renderer, "sprites/tree_stump.png");
    tileToHarvest->active = GD_TRUE;
    tileToHarvest->harvested = GD_TRUE;
    tile_set_sprite_size(tileToHarvest, 64, 64);

    // Update inventory
    h->inventory[tileToHarvest->harvestedItem]++;
}

void heroInteract(Hero *h, Game *g)
{
    // Check for harvestable tile

    // get next tile in facing direction if it's close enough
    i32 harvest_threshold = 10;
    // TODO(chj): This is based on the bottom center of the sprite. Not very accurate
    // We want to test on a 10 pixel box outside the bounding box
    Point point_to_harvest = h->e.collision_pt;

    switch (h->e.direction)
    {
    case DIR_UP:
        point_to_harvest.y -= harvest_threshold;
        break;
    case DIR_DOWN:
        point_to_harvest.y += harvest_threshold;
        break;
    case DIR_LEFT:
        point_to_harvest.x -= harvest_threshold;
        break;
    case DIR_RIGHT:
        point_to_harvest.x += harvest_threshold;
        break;
    default:
        break;
    }

    Tile *tileToHarvest = map_get_tile_at_point(g->current_map, point_to_harvest);
    if (tileToHarvest && tileToHarvest->is_harvestable && !tileToHarvest->harvested)
    {
        harvestTile(h, g, tileToHarvest);
    }

    // Check for entities to interact with in a circle
    Vec2 heroCenter = {};
    heroCenter.x = h->e.dest_rect.x + (0.5f * h->e.dest_rect.w);
    heroCenter.y = h->e.dest_rect.y + (0.5f * h->e.dest_rect.h);
    heroInteractionRegion.center = heroCenter;
    heroInteractionRegion.radius = maxFloat32((f32)h->e.dest_rect.w, (f32)h->e.dest_rect.h) * 0.666666f;

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

    for (u32 i = 0; i < stencilSize; ++i)
    {
        if (tileStencil[i] && tile_is_interactive(tileStencil[i]) && tileStencil[i]->onHeroInteract)
        {
            tileStencil[i]->onHeroInteract(tileStencil[i], h);
        }
    }
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

void updateHero(Hero* h, Input* input, Game* g)
{
    SDL_Rect saved_position = h->e.dest_rect;

    processInput(h, input, (f32)g->dt / 1000.0f);

    if (saved_position.x != h->e.dest_rect.x ||
        saved_position.y != h->e.dest_rect.y)
    {
        h->isMoving = GD_TRUE;
    }
    else
    {
        h->isMoving = GD_FALSE;
    }

    if (!h->isMoving)
    {
        h->e.animation.current_frame = 0;
        h->e.sprite_rect.x = 0;
    }

    clampHeroToMap(h, g->current_map);
    setEntityCollisionPoint(&h->e);
    checkHeroCollisionsWithTiles(h, g, saved_position);
    checkHeroCollisionsWithEntities(h, g, saved_position);

    if (h->harvest)
    {
        heroInteract(h, g);
    }
}

void hero_update_club(Hero* h, u32 now)
{
    h->clubRect.x = h->e.dest_rect.x + h->e.dest_rect.w / 2;
    h->clubRect.y = h->e.dest_rect.y + h->e.dest_rect.h / 2;

    switch(h->e.direction)
    {
    case DIR_DOWN:
        h->clubRect.w = 8;
        h->clubRect.x -= 4;
        h->clubRect.h = 32;
        h->clubRect.y += 16;
        break;
    case DIR_LEFT:
        h->clubRect.w = 32;
        h->clubRect.h = 8;
        h->clubRect.y += 16;
        h->clubRect.x -= 32;
        break;
    case DIR_RIGHT:
        h->clubRect.y += 16;
        h->clubRect.w = 32;
        h->clubRect.h = 8;
        break;
    case DIR_UP:
        h->clubRect.x -= 4;
        h->clubRect.y -= 32;
        h->clubRect.w = 8;
        h->clubRect.h = 32;
        break;
    }

    if (h->swingClub && now > h->nextClubSwingDelay + 500)
    {
        h->nextClubSwingDelay = now;
        h->clubSwingTimeout = now + 500;
    }
    else
    {
        h->swingClub = GD_FALSE;
    }
}

// void hero_draw_club(Hero* h, u32 now, Game* g)
// {
//     if (now < h->clubSwingTimeout)
//     {
//         SDL_FillRect(g->current_map->surface, &h->clubRect, g->colors[BLACK]);
//     }
// }

Entity createBuffalo(int starting_x, int starting_y, SDL_Renderer* renderer)
{
    Entity buffalo = {};
    initEntitySpriteSheet(&buffalo, "sprites/Buffalo.png", 4, 1, renderer);
    setEntityStartingPos(&buffalo, starting_x, starting_y);
    setEntityBoundingBoxOffset(&buffalo, 0, 0, 0, 0);
    initEntityDest(&buffalo);
    setEntityCollisionPoint(&buffalo);
    buffalo.speed = 3;
    buffalo.type = ET_BUFFALO;
    buffalo.can_move = GD_TRUE;
    buffalo.collision_pt_offset = 32;
    initAnimation(&buffalo.animation, 4, 100);
    buffalo.plan = {};
    buffalo.has_plan = GD_TRUE;
    buffalo.plan.move_delay = (rand() % 2000) + 1000;
    buffalo.plan.mv_dir = (CardinalDir)(rand() % 4);
    return buffalo;
}

