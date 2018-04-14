#include "entity.h"

Entity::Entity(const char* sprite_path,
               int x_sprites,
               int y_sprites,
               int speed,
               float starting_x,
               float starting_y,
               int bb_x,
               int bb_y,
               int bb_w,
               int bb_h,
               bool active):
    speed(speed),
    starting_pos(starting_x, starting_y),
    bb_x_offset(bb_x),
    bb_y_offset(bb_y),
    bb_w_offset(bb_w),
    bb_h_offset(bb_h),
    active(active),
    sprite_sheet(),
    direction(DOWN)

{
    sprite_sheet.load(sprite_path, x_sprites, y_sprites);
    sprite_rect.w = sprite_sheet.sprite_width;
    sprite_rect.h = sprite_sheet.sprite_height;
    dest_rect.x = (int)starting_pos.x;
    dest_rect.y = (int)starting_pos.y;
    dest_rect.w = sprite_sheet.sprite_width;
    dest_rect.h = sprite_sheet.sprite_height;
}

Entity::~Entity()
{}

void Entity::draw(SDL_Surface* map)
{
    // Draw sprite
    if (active)
    {
        SDL_BlitSurface(sprite_sheet.sheet, &sprite_rect, map, &dest_rect);
    }

#ifdef DEBUG
    // Draw bounding box
    SDL_Rect bb_top;
    SDL_Rect bb_bottom;
    SDL_Rect bb_left;
    SDL_Rect bb_right;
    int bb_line_width = 2;

    bb_top.x = bounding_box.x;
    bb_top.y = bounding_box.y;
    bb_top.w = bounding_box.w;
    bb_top.h = bb_line_width;

    bb_left.x = bounding_box.x;
    bb_left.y = bounding_box.y;
    bb_left.w = bb_line_width;
    bb_left.h = bounding_box.h;

    bb_right.x = bounding_box.x + bounding_box.w;
    bb_right.y = bounding_box.y;
    bb_right.w =bb_line_width;
    bb_right.h = bounding_box.h;

    bb_bottom.x = bounding_box.x;
    bb_bottom.y = bounding_box.y + bounding_box.h;
    bb_bottom.w = bounding_box.w + bb_line_width;
    bb_bottom.h = bb_line_width;

    // TODO: Need global access to colors
    Uint32 magenta = 16711935;
    SDL_FillRect(map, &bb_top, magenta);
    SDL_FillRect(map, &bb_left, magenta);
    SDL_FillRect(map, &bb_right, magenta);
    SDL_FillRect(map, &bb_bottom, magenta);
#endif
}

void Entity::update()
{
    bounding_box.x = dest_rect.x + bb_x_offset;
    bounding_box.y = dest_rect.y + bb_y_offset;
    bounding_box.w = dest_rect.w - bb_w_offset;
    bounding_box.h = dest_rect.h - bb_h_offset;
}

// One piece of a circle split in 8 sections
// The radians start at 2*PI on (1, 0) and go to zero counter-clockwise
Direction get_direction_from_angle(float angle)
{
    float direction_increment = (2.0f * PI) / 8.0f;
    float half_increment = 0.5f * direction_increment;
    Direction result;

    if (angle >= (3.0f * PI) / 2.0f - half_increment &&
        angle < (3.0f * PI) / 2.0f + half_increment)
    {
        result = UP;
    }
    else if (angle >= (3.0f * PI) / 2.0f + half_increment &&
             angle < 2.0f * PI - half_increment)
    {
        result = UP_RIGHT;
    }
    else if (angle >= 2.0f * PI - half_increment ||
             angle < half_increment)
    {
        result = RIGHT;
    }
    else if (angle >= half_increment &&
             angle < PI / 2.0f - half_increment)
    {
        result = DOWN_RIGHT;
    }
    else if (angle >= PI / 2.0f - half_increment &&
             angle < PI / 2.0f + half_increment)
    {
        result = DOWN;
    }
    else if (angle >= PI / 2.0f + half_increment &&
             angle < PI - half_increment)
    {
        result = DOWN_LEFT;
    }
    else if (angle >= PI - half_increment &&
             angle < PI + half_increment)
    {
        result = LEFT;
    }
    else if (angle >= PI + half_increment &&
             angle < (3.0f * PI) / half_increment)
    {
        result = UP_LEFT;
    }
    else
    {
        result = DOWN;
    }
    return result;
}
