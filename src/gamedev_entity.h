#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_sprite_sheet.h"

#define MAX_BELT_ITEMS 8

enum EntityType
{
    EntityType_Tile,
    EntityType_Buffalo,
    EntityType_Hero,
    EntityType_Harlod,
    EntityType_Flame,

    EntityType_Count
};

enum CraftableItemType
{
    Craftable_None,
    Craftable_Tree,
    Craftable_Glow_Juice,

    Craftable_Count
};

enum InventoryItemType
{
    InventoryItemType_None,
    InventoryItemType_Leaves,
    InventoryItemType_Glow,

    InventoryItemType_Count
};

enum FireState
{
    FireState_None,
    FireState_Started,
    FireState_Caught,
    FireState_Burnt,

    FireState_Count
};

struct BeltItem
{
    CraftableItemType type;
    u32 count;
};

struct Entity
{
    i32 width;
    i32 height;

    // The current rectangle in the sprite sheet to be drawn
    Rect spriteRect;
    SpriteSheet spriteSheet;
    Animation animation;

    f32 speed;
    Direction direction;
    // For tiles, this is the center of the tile. For other entites, it is the
    // bottome center of the collision rect
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    b32 collides;

    // Whether or not the renderer will draw this Entity
    b32 isVisible;
    // Whether or not animation frames should be advanced
    b32 shouldAnimate;

    // Inactive entites are on the free list. Treat them as if they don't exist
    b32 active;
    b32 isMoving;
    b32 inQuicksand;

    EntityType type;

    Entity *nextFree;

    // TODO(cjh): @win32
// #pragma warning(disable:4201)
    // union
    // {
    //     struct
    //     {
            // Tile
            Vec4u8 color;
            u32 tileFlags;
            u64 timeToCatchFire;
            u64 timeSpentOnFire;
            u32 burntTileIndex;
            b32 validPlacement;
            FireState fireState;
            InventoryItemType harvestedItem;
            CraftableItemType craftableItem;
            Entity *deleteAfterPlacement;
        // };

    // Campfire
    b32 isLit;

        // struct
        // {
            // Hero
            b32 craftTree;
            b32 craftGlowJuice;
            b32 harvesting;
            b32 placingItem;
            // TODO(cjh): inventory[0] is always 0 because of InventoryItemType_None
            u32 beltItemCount;
            i32 activeBeltItemIndex;
            Rect heroInteractionRect;
            BeltItem beltItems[MAX_BELT_ITEMS];
            u32 inventory[InventoryItemType_Count];
            Entity *tileToPlace;
        // };

        // struct
        // {
            // Harlod
            EntireFile dialogueFile;
        // };
    // };
// #pragma warning(default:4201)
};
#endif
