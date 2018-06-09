#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_tilemap.h"
#include "gamedev_sprite_sheet.h"

struct Entity
{
    EntityType type;

    SpriteSheet sprite_sheet;
    SDL_Rect sprite_rect;
    Animation animation;
    Plan* plan;

    int speed;
    Direction direction;
    Point starting_pos;
    Point collision_pt;
    u32 collision_pt_offset;

    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    SDL_Rect dest_rect;
    SDL_Rect bounding_box;
    int bb_x_offset;
    int bb_y_offset;
    int bb_w_offset;
    int bb_h_offset;

    u8 active;
    u8 has_plan;
    u8 can_move;
};

struct EntityList
{
    Entity** entities;
    u32 count;
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
};
#endif
