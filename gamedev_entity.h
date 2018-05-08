#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_tilemap.h"
#include "gamedev_sprite_sheet.h"

struct Entity
{
    SpriteSheet sprite_sheet;
    Animation animation;
    Plan plan;
    SDL_Rect sprite_rect;
    int speed;
    Direction direction;
    Point starting_pos;
    SDL_Rect bounding_box;
    SDL_Rect dest_rect;
    int bb_x_offset;
    int bb_y_offset;
    int bb_w_offset;
    int bb_h_offset;
    u8 active;
    u8 has_plan;
    u8 can_move;
};

struct Hero
{
    Entity e;
    u8 is_moving;
    u8 in_quicksand;
    u8 swing_club;
    u32 next_club_swing_delay;
    u32 club_swing_timeout;
    SDL_Rect club_rect;
    Point collision_pt;
};

struct EntityList
{
    Entity** entities;
    u32 count;
};

#endif