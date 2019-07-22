#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_sprite_sheet.h"

enum EntityType
{
    ET_TILE,
    ET_BUFFALO,
    ET_HERO,
    ET_HARLOD,
    ET_ENEMY,
    ET_FLAME,
    ET_NUM_TYPES
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
    INV_NONE,
    INV_LEAVES,
    INV_GLOW,
    INV_COUNT
};

enum FireState
{
    FIRE_NONE,
    FIRE_STARTED,
    FIRE_CAUGHT,
    FIRE_BURNT
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
    Point spriteDims;
    Animation animation;

    f32 speed;
    Direction direction;
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    b32 collides;

    b32 active;
    b32 isMoving;
    b32 inQuicksand;

    EntityType type;

    // Tile
    u32 color;
    u32 tileFlags;
    b32 validPlacement;
    FireState fireState;
    i32 timeToCatchFire;
    i32 timeSpentOnFire;
    u32 burntTileIndex;
    InventoryItemType harvestedItem;
    CraftableItemType craftableItem;
    Entity *deleteAfterPlacement;

    // Hero
    Rect heroInteractionRect;
    b32 craftTree;
    b32 craftGlowJuice;
    b32 harvesting;
    b32 placingItem;
    Entity *tileToPlace;
    // TODO(cjh): inventory[0] is always 0 because of INV_NONE
    u32 inventory[INV_COUNT];
    u32 beltItemCount;
    i32 activeBeltItemIndex;
    BeltItem beltItems[8];

    // Harlod
    EntireFile dialogueFile;
};
#endif
