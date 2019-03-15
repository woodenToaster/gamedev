#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_tilemap.h"
#include "gamedev_sprite_sheet.h"

struct Entity;
struct Hero;

typedef void (*interactWithEntity)(Entity *e, Entity *other);
typedef void (*interactWithHero)(Entity *e, Hero *h, Game *g);

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

enum InventoryItemType
{
    INV_LEAVES,
    INV_COUNT
};

struct Hero
{
    Entity e;
    SDL_Rect clubRect;
    u32 nextClubSwingDelay;
    u32 clubSwingTimeout;
    f32 speed;
    f32 boundingRadius;
    bool32 isMoving;
    bool32 inQuicksand;
    bool32 swingClub;
    bool32 harvest;
    u32 inventory[INV_COUNT];
};

struct Harlod
{
    Entity e;
    interactWithHero onHeroInteract;
};
#endif
