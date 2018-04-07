#ifndef ENTITY_H
#define ENTITY_H

#include "tile_map.h"
#include "sprite_sheet.h"

enum Direction
{
    UP,
    UP_RIGHT,
    RIGHT,
    DOWN_RIGHT,
    DOWN,
    DOWN_LEFT,
    LEFT,
    UP_LEFT
};

struct Entity
{
    SpriteSheet sprite_sheet;

    Animation animation;
    SDL_Rect sprite_rect;
    int speed;
    Direction direction;
    Point starting_pos;
    SDL_Rect bounding_box;
    SDL_Rect dest_rect;

    bool active;

    Entity(const char* sprite_path,
           int x_sprites,
           int y_sprites,
           int speed,
           float starting_x,
           float starting_y,
           bool active=false);
    ~Entity();

    void draw(SDL_Surface* map);
};

#endif
