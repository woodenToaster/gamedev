#ifndef ENTITY_H
#define ENTITY_H

#include "tile_map.h"

enum Direction {
    UP,
    UP_RIGHT,
    RIGHT,
    DOWN_RIGHT,
    DOWN,
    DOWN_LEFT,
    LEFT,
    UP_LEFT
};

// Sprite sheet
struct SpriteSheet {
    SDL_Surface* sheet;
    int width;
    int height;
    int num_x;
    int num_y;
    int sprite_width;
    int sprite_height;

    void load(const char* path, int sprite_width, int sprite_height);
};

struct Entity {
    SpriteSheet sprite_sheet;

    Animation animation;
    SDL_Rect sprite_rect;
    int speed;
    Direction direction;

    SDL_Rect bounding_box;
    SDL_Rect dest_rect;

    ~Entity();
};

#endif
