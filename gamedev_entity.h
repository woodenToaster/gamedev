#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_tilemap.h"
#include "gamedev_sprite_sheet.h"

struct Entity;
typedef void (*interactWithEntity)(Entity *e, Entity *other);
typedef void (*interactWithHero)(Entity *e, Entity *other, Game *g);

struct Entity
{
    EntityType type;

    SpriteSheet sprite_sheet;
    SDL_Rect sprite_rect;
    Animation animation;
    Plan plan;
    u8* pixel_data;

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

    EntireFile dialogFile;
};

struct EntityList
{
    Entity** entities;
    u32 count;
};

struct Hero
{
    Entity e;
    SDL_Rect club_rect;
    u32 next_club_swing_delay;
    u32 club_swing_timeout;
    f32 speed;
    u8 is_moving;
    u8 in_quicksand;
    bool32 swing_club;
    u8 harvest;
};

struct Harlod
{
    Entity e;
    interactWithHero onHeroInteract;
};
#endif
