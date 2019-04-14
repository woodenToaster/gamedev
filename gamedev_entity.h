#ifndef GD_ENTITY_H
#define GD_ENTITY_H

#include "gamedev_sprite_sheet.h"

enum CraftableItemType
{
    CRAFTABLE_NONE,
    CRAFTABLE_TREE,
    CRAFTABLE_COUNT
};

enum InventoryItemType
{
    INV_NONE,
    INV_LEAVES,
    INV_TREES,
    INV_COUNT
};

struct Entity
{
    i32 width;
    i32 height;

    // The current rectangle in the sprite sheet to be drawn
    SDL_Rect sprite_rect;
    SpriteSheet sprite_sheet;
    Point spriteDims;
    Animation animation;

    f32 speed;
    Direction direction;
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    bool32 collides;

    bool32 active;
    bool32 isMoving;
    bool32 inQuicksand;

    EntireFile dialogFile;
    EntityType type;

    // Tile
    u32 color;
    u32 tileFlags;
    bool32 isHarvestable;
    bool32 validPlacement;
    InventoryItemType harvestedItem;
    CraftableItemType craftableItem;
    Entity *deleteAfterPlacement;
    SDL_Texture *unharvestedSprite;
    SDL_Texture *harvestedSprite;

    // Hero
    SDL_Rect heroInteractionRect;
    bool32 craft;
    bool32 harvesting;
    bool32 placingItem;
    Entity *tileToPlace;
    u32 inventory[INV_COUNT];
};
#endif
