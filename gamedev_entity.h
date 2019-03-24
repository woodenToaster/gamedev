#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_tilemap.h"
#include "gamedev_sprite_sheet.h"

struct Entity;
struct Hero;

typedef void (*interactWithEntity)(Entity *e, Entity *other);
typedef void (*interactWithHero)(Entity *e, Hero *h, Game *g);

enum CraftableItem
{
    CRAFTABLE_TREE,
    CRAFTABLE_COUNT
};

enum InventoryItemType
{
    INV_LEAVES,
    INV_TREES,
    INV_COUNT
};

struct Entity
{
    // These are the actual drawn sizes, which may be different from the sprite size
    i32 width;
    i32 height;

    // The current rectangle in the sprite sheet to be drawn
    SDL_Rect sprite_rect;
    SpriteSheet sprite_sheet;

    Animation animation;
    Plan plan;

    f32 speed;
    Direction direction;
    Vec2 starting_pos;
    Point collision_pt;
    u32 collision_pt_offset;

    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    // Offsets are from top left of sprite_rect
    i32 bb_x_offset;
    i32 bb_y_offset;
    SDL_Rect bounding_box;

    bool32 active;
    bool32 has_plan;
    bool32 can_move;

    EntireFile dialogFile;
    EntityType type;
};

struct EntityList
{
    Entity** entities;
    u32 count;
};

struct Hero
{
    Entity e;
    SDL_Rect clubRect;
    u32 nextClubSwingDelay;
    u32 clubSwingTimeout;
    f32 boundingRadius;
    bool32 isMoving;
    bool32 inQuicksand;
    bool32 swingClub;
    bool32 harvest;
    bool32 craft;
    bool32 place;
    u32 inventory[INV_COUNT];
};

struct Harlod
{
    Entity e;
    interactWithHero onHeroInteract;
};

struct Knight
{
    Entity e;
};
#endif
