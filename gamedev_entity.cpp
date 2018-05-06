#include "gamedev_entity.h"

void entity_init_sprite_sheet(Entity* e, const char* path, int num_x, int num_y)
{
    sprite_sheet_load(&e->sprite_sheet, path, num_x, num_y);
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

void entity_draw(Entity* e, SDL_Surface* map)
{
    // Draw sprite
    if (e->active)
    {
        e->sprite_rect.x = e->sprite_rect.w * e->animation.current_frame;
        SDL_BlitSurface(e->sprite_sheet.sheet, &e->sprite_rect, map, &e->dest_rect);
    }

#if 0
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

    // TODO: Need global access to colors
    Uint32 magenta = 16711935;
    SDL_FillRect(map, &bb_top, magenta);
    SDL_FillRect(map, &bb_left, magenta);
    SDL_FillRect(map, &bb_right, magenta);
    SDL_FillRect(map, &bb_bottom, magenta);
#endif
}

void entity_update(Entity* e, u32 last_frame_duration)
{
    e->bounding_box.x = e->dest_rect.x + e->bb_x_offset;
    e->bounding_box.y = e->dest_rect.y + e->bb_y_offset;
    e->bounding_box.w = e->dest_rect.w - e->bb_w_offset;
    e->bounding_box.h = e->dest_rect.h - e->bb_h_offset;

    animation_update(&e->animation, last_frame_duration);
}

void entity_destroy(Entity* e)
{
    sprite_sheet_destroy(&e->sprite_sheet);
}

void set_hero_sprite(Entity* e)
{
    switch (e->direction)
    {
    case UP:
        e->sprite_rect.x = 0;
        e->sprite_rect.y = e->sprite_sheet.sprite_height;
        break;
    case UP_RIGHT:
        e->sprite_rect.x = 8 * e->sprite_sheet.sprite_width;
        e->sprite_rect.y = 0;
        break;
    case RIGHT:
        e->sprite_rect.x = 0;
        e->sprite_rect.y = 0;
        break;
    case DOWN_RIGHT:
        e->sprite_rect.x = 8 * e->sprite_sheet.sprite_width;
        e->sprite_rect.y = 4 * e->sprite_sheet.sprite_height;
        break;
    case DOWN:
        e->sprite_rect.x = 0;
        e->sprite_rect.y = 4 * e->sprite_sheet.sprite_height;
        break;
    case DOWN_LEFT:
        e->sprite_rect.x = 8 * e->sprite_sheet.sprite_width;
        e->sprite_rect.y = 3 * e->sprite_sheet.sprite_height;
        break;
    case LEFT:
        e->sprite_rect.x = 0;
        e->sprite_rect.y = 3 * e->sprite_sheet.sprite_height;
        break;
    case UP_LEFT:
        e->sprite_rect.x = 8 * e->sprite_sheet.sprite_width;
        e->sprite_rect.y = e->sprite_sheet.sprite_height;
        break;
    default:
        break;
    }
}

void entity_list_update(EntityList* el, u32 last_frame_duration)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        entity_update(el->entities[i], last_frame_duration);
    }
}

void entity_list_draw(EntityList* el, SDL_Surface* map_surface)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        entity_draw(el->entities[i], map_surface);
    }
}

void entity_list_destroy(EntityList* el)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        entity_destroy(el->entities[i]);
    }
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

void hero_draw_club(Hero* h, u32 now, SDL_Surface* map_surface, u32 color)
{
    if (now < h->club_swing_timeout)
    {
        SDL_FillRect(map_surface, &h->club_rect, color);
    }
}

bool overlaps(SDL_Rect* r1, SDL_Rect* r2)
{
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
}
