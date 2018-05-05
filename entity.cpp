#include "entity.h"

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

void entity_update(Entity* e)
{
    e->bounding_box.x = e->dest_rect.x + e->bb_x_offset;
    e->bounding_box.y = e->dest_rect.y + e->bb_y_offset;
    e->bounding_box.w = e->dest_rect.w - e->bb_w_offset;
    e->bounding_box.h = e->dest_rect.h - e->bb_h_offset;
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

void entity_list_destroy(EntityList* el)
{
    for (u32 i = 0; i < el->count; ++i)
    {
        entity_destroy(el->entities[i]);
    }
}

bool overlaps(SDL_Rect* r1, SDL_Rect* r2)
{
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
}
