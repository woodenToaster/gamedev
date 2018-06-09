#include "gamedev_entity.h"
#include "gamedev_camera.h"
#include "gamedev_plan.h"

void renderer_fill_rect(SDL_Renderer* renderer, SDL_Rect* dest, u32 color)
{
    u8 r = (u8)((color & 0x00FF0000) >> 16);
    u8 g = (u8)((color & 0x0000FF00) >> 8);
    u8 b = (u8)((color & 0x000000FF) >> 0);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
    SDL_RenderFillRect(renderer, dest);
}

bool overlaps(SDL_Rect* r1, SDL_Rect* r2)
{
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
}

void entity_init_sprite_sheet(Entity* e, const char* path, int num_x, int num_y, SDL_Renderer* renderer)
{
    sprite_sheet_load(&e->sprite_sheet, path, num_x, num_y, renderer);
    e->sprite_rect.w = e->sprite_sheet.sprite_width;
    e->sprite_rect.h = e->sprite_sheet.sprite_height;
}

void entity_set_starting_pos(Entity* e, int x, int y)
{
    e->starting_pos.x = x;
    e->starting_pos.y = y;
}

void entity_set_bounding_box_offset(Entity* e, int x, int y, int w, int h)
{
    e->bb_x_offset = x;
    e->bb_y_offset = y;
    e->bb_w_offset = w;
    e->bb_h_offset = h;
}

void entity_init_dest(Entity* e)
{
    e->dest_rect.x = e->starting_pos.x;
    e->dest_rect.y = e->starting_pos.y;
    e->dest_rect.w = e->sprite_sheet.sprite_width;
    e->dest_rect.h = e->sprite_sheet.sprite_height;
}

u8 entity_is_hero(Entity* e)
{
    return e->type == ET_HERO;
}

void entity_draw(Entity* e, Game* g)
{
    if (e->active)
    {
        e->sprite_rect.x = e->sprite_rect.w * e->animation.current_frame;
        SDL_RenderCopy(g->renderer, e->sprite_sheet.sheet, &e->sprite_rect, &e->dest_rect);
    }

#ifdef DEBUG
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

    u32 magenta = g->colors[MAGENTA];
    if (e->active)
    {
        renderer_fill_rect(g->renderer, &bb_top, magenta);
        renderer_fill_rect(g->renderer, &bb_left, magenta);
        renderer_fill_rect(g->renderer, &bb_right, magenta);
        renderer_fill_rect(g->renderer, &bb_bottom, magenta);
    }
#endif
}

SDL_Rect entity_get_overlap_box(Entity* e1, Entity* e2)
{
    SDL_Rect overlap_box;
    if (e1->bounding_box.x > e2->bounding_box.x)
    {
        overlap_box.x = e1->bounding_box.x;
        overlap_box.w = e2->bounding_box.x + e2->bounding_box.w - e1->bounding_box.x;
        overlap_box.w = min(overlap_box.w, e1->bounding_box.w);
    }
    else
    {
        overlap_box.x = e2->bounding_box.x;
        overlap_box.w = e1->bounding_box.x + e1->bounding_box.w - e2->bounding_box.x;
        overlap_box.w = min(overlap_box.w, e2->bounding_box.w);
    }

    if (e1->bounding_box.y > e2->bounding_box.y)
    {
        overlap_box.y = e1->bounding_box.y;
        overlap_box.h = e2->bounding_box.y + e2->bounding_box.h - e1->bounding_box.y;
        overlap_box.h = min(overlap_box.h, e1->bounding_box.h);
    }
    else
    {
        overlap_box.y = e2->bounding_box.y;
        overlap_box.h = e1->bounding_box.y + e1->bounding_box.h - e2->bounding_box.y;
        overlap_box.h = min(overlap_box.h, e2->bounding_box.h);
    }

    return overlap_box;
}

void entity_move_in_direction(Entity* e, CardinalDir d)
{
    switch(d)
    {
    case CARDINAL_NORTH:
        e->dest_rect.y -= e->speed;
        break;
    case CARDINAL_SOUTH:
        e->dest_rect.y += e->speed;
        break;
    case CARDINAL_WEST:
        e->dest_rect.x -= e->speed;
        break;
    case CARDINAL_EAST:
        e->dest_rect.x += e->speed;
        break;
    default:
        return;
    }
}

Tile* entity_get_tile_at_position(Entity* e, Map* map)
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

void entity_reverse_direction(Entity* e)
{
    // TODO: Try to use entity direction directly instead of plan.mv_dir
    switch (e->plan->mv_dir)
    {
    case CARDINAL_NORTH:
        e->plan->mv_dir = CARDINAL_SOUTH;
        break;
    case CARDINAL_SOUTH:
        e->plan->mv_dir = CARDINAL_NORTH;
        break;
    case CARDINAL_EAST:
        e->plan->mv_dir = CARDINAL_WEST;
        break;
    case CARDINAL_WEST:
        e->plan->mv_dir = CARDINAL_EAST;
        break;
    }
}

void entity_check_collisions_with_tiles(Entity* e, Map* map, SDL_Rect* saved_pos)
{
    Tile* current_tile = entity_get_tile_at_position(e, map);

    if (tile_is_solid(current_tile))
    {
        // Collision with an impenetrable tile. Revert to original position
        e->dest_rect = *saved_pos;
        switch (e->type)
        {
        case ET_BUFFALO:
            entity_reverse_direction(e);
            break;
        default:
            break;
        }
    }
}

void entity_check_collisions_with_entities(Entity* e, Game* game)
{
    for (u32 i = 0; i < game->current_map->active_entities.count; ++i) {
        Entity* other_e = game->current_map->active_entities.entities[i];
        if (e == other_e) {
            continue;
        }
        if (other_e->active && overlaps(&e->bounding_box, &other_e->bounding_box))
        {
            if (entity_is_hero(e))
            {
                SDL_Rect overlap_box = entity_get_overlap_box(e, other_e);
                // TODO: Handle properly instead of just drawing the overlap.
                renderer_fill_rect(game->renderer, &overlap_box, game->colors[MAGENTA]);
                // TODO: pixel collision
            }
            else if (e->type == ET_BUFFALO)
            {
                if (other_e->type == ET_BUFFALO)
                {
                    entity_reverse_direction(e);
                }
            }
        }
    }
}

void entity_set_collision_point(Entity* e)
{
    e->collision_pt.y = e->dest_rect.y + e->dest_rect.h - e->collision_pt_offset;
    e->collision_pt.x = e->dest_rect.x + (i32)(e->dest_rect.w / 2.0);
}

void entity_update(Entity* e, Map* map, u32 last_frame_duration)
{
    if (e->has_plan) {
        plan_update(e, last_frame_duration);
        if (e->can_move && e->active)
        {
            SDL_Rect saved_position = e->dest_rect;
            entity_move_in_direction(e, e->plan->mv_dir);
            animation_update(&e->animation, last_frame_duration, GD_TRUE);
            entity_set_collision_point(e);
            entity_check_collisions_with_tiles(e, map, &saved_position);
            // entity_check_collisions_with_entities(e, map, &saved_position);
            entity_set_collision_point(e);
        }
    }
    e->bounding_box.x = e->dest_rect.x + e->bb_x_offset;
    e->bounding_box.y = e->dest_rect.y + e->bb_y_offset;
    e->bounding_box.w = e->dest_rect.w - e->bb_w_offset;
    e->bounding_box.h = e->dest_rect.h - e->bb_h_offset;
}

void entity_destroy(Entity* e)
{
    sprite_sheet_destroy(&e->sprite_sheet);
}


void entity_list_update(EntityList* el, Map* map, u32 last_frame_duration)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        entity_update(el->entities[i], map, last_frame_duration);
    }
}

void entity_list_draw(EntityList* el, Game* g)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        entity_draw(el->entities[i], g);
    }
}

void entity_list_destroy(EntityList* el)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        entity_destroy(el->entities[i]);
    }
}

void hero_check_collisions_with_tiles(Hero* h, Game* game, SDL_Rect saved_position)
{
    Tile* current_tile = entity_get_tile_at_position(&h->e, game->current_map);

    if (tile_is_solid(current_tile))
    {
        h->e.dest_rect = saved_position;
        h->e.position.x = (f32)saved_position.x;
        h->e.position.y = (f32)saved_position.y;

        h->e.velocity.x = 0.0f;
        h->e.velocity.y = 0.0f;
    }
    if (tile_is_slow(current_tile) && !h->in_quicksand)
    {
        // TODO: Update velocity (or acceleration?)
        h->e.speed -= 9;
        h->in_quicksand = GD_TRUE;
        if (h->is_moving)
        {
            sound_queue(global_sounds[MUD_SOUND], game->sounds);
        }
    }
    else if (h->in_quicksand)
    {
        h->e.speed += 9;
        h->in_quicksand = GD_FALSE;
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

void hero_clamp_to_map(Hero* h, Map* map)
{

    h->e.dest_rect.x = clamp(h->e.dest_rect.x, 0, map->width_pixels - h->e.dest_rect.w);
    h->e.dest_rect.y = clamp(h->e.dest_rect.y, 0, map->height_pixels - h->e.dest_rect.h);

    if (h->e.dest_rect.x != (int)h->e.position.x || h->e.dest_rect.y != (int)h->e.position.y)
    {
        h->e.velocity.x = 0.0f;
        h->e.velocity.y = 0.0f;
    }
    h->e.position.x = (f32)h->e.dest_rect.x;
    h->e.position.y = (f32)h->e.dest_rect.y;

}

void hero_process_input(Hero* h, Input* input, f32 dt)
{
    Vec2 acceleration = {};

    if (input->is_pressed[KEY_RIGHT])
    {
        acceleration.x = 1.0f;
        h->e.sprite_rect.y = 2 * h->e.sprite_sheet.sprite_height;
    }
    if (input->is_pressed[KEY_LEFT])
    {
        acceleration.x = -1.0f;
        h->e.sprite_rect.y = 1 * h->e.sprite_sheet.sprite_height;
    }
    if (input->is_pressed[KEY_UP])
    {
        acceleration.y = -1.0f;
        h->e.sprite_rect.y = 3 * h->e.sprite_sheet.sprite_height;
    }
    if (input->is_pressed[KEY_DOWN])
    {
        acceleration.y = 1.0f;
        h->e.sprite_rect.y = 0 * h->e.sprite_sheet.sprite_height;
    }

    if (acceleration.x != 0.0f && acceleration.y != 0.0f)
    {
        acceleration *= 0.707186781187f;
    }

    f32 speed = 1000.0f; // m/s^2
    acceleration *= speed;

    acceleration -= 4 * h->e.velocity;

    h->e.position = 0.5 * acceleration * square(dt) +
                    h->e.velocity * (f32)dt +
                    h->e.position;

    h->e.velocity = acceleration * (f32)dt + h->e.velocity;

    if (input->is_pressed[KEY_F])
    {
        h->swing_club = GD_TRUE;
    }

    h->e.dest_rect.x = (int)h->e.position.x;
    h->e.dest_rect.y = (int)h->e.position.y;
}

void hero_update(Hero* h, Input* input, Game* g)
{
    SDL_Rect saved_position = h->e.dest_rect;

    hero_process_input(h, input, (f32)g->dt / 1000.0f);

    if (saved_position.x != h->e.dest_rect.x ||
        saved_position.y != h->e.dest_rect.y)
    {
        h->is_moving = GD_TRUE;
    }
    else
    {
        h->is_moving = GD_FALSE;
    }

    if (!h->is_moving)
    {
        h->e.animation.current_frame = 0;
        h->e.sprite_rect.x = 0;
    }

    hero_clamp_to_map(h, g->current_map);
    entity_set_collision_point(&h->e);
    hero_check_collisions_with_tiles(h, g, saved_position);
}

void hero_update_club(Hero* h, u32 now)
{
    h->club_rect.x = h->e.dest_rect.x + h->e.dest_rect.w / 2;
    h->club_rect.y = h->e.dest_rect.y + h->e.dest_rect.h / 2;

    switch(h->e.direction)
    {
    case DOWN:
        h->club_rect.w = 8;
        h->club_rect.x -= 4;
        h->club_rect.h = 32;
        h->club_rect.y += 16;
        break;
    case LEFT:
        h->club_rect.w = 32;
        h->club_rect.h = 8;
        h->club_rect.y += 16;
        h->club_rect.x -= 32;
        break;
    case RIGHT:
        h->club_rect.y += 16;
        h->club_rect.w = 32;
        h->club_rect.h = 8;
        break;
    case UP:
        h->club_rect.x -= 4;
        h->club_rect.y -= 32;
        h->club_rect.w = 8;
        h->club_rect.h = 32;
        break;
    }

    if (h->swing_club && now > h->next_club_swing_delay + 500)
    {
        h->next_club_swing_delay = now;
        h->club_swing_timeout = now + 500;
    }
    else
    {
        h->swing_club = GD_FALSE;
    }
}

// void hero_draw_club(Hero* h, u32 now, Game* g)
// {
//     if (now < h->club_swing_timeout)
//     {
//         SDL_FillRect(g->current_map->surface, &h->club_rect, g->colors[BLACK]);
//     }
// }

Entity create_buffalo(int starting_x, int starting_y, Plan* plan, SDL_Renderer* renderer)
{
    Entity buffalo = {};
    entity_init_sprite_sheet(&buffalo, "sprites/Buffalo.png", 4, 1, renderer);
    entity_set_starting_pos(&buffalo, starting_x, starting_y);
    entity_set_bounding_box_offset(&buffalo, 0, 0, 0, 0);
    entity_init_dest(&buffalo);
    entity_set_collision_point(&buffalo);
    buffalo.speed = 3;
    buffalo.type = ET_BUFFALO;
    buffalo.can_move = GD_TRUE;
    buffalo.collision_pt_offset = 32;
    animation_init(&buffalo.animation, 4, 100);
    buffalo.plan = plan;
    buffalo.has_plan = GD_TRUE;
    buffalo.plan->move_delay = (rand() % 2000) + 1000;
    buffalo.plan->mv_dir = (CardinalDir)(rand() % 4);
    return buffalo;
}

