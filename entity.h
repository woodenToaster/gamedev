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

struct Entity {
    SDL_Surface* sprite_sheet;
    SDL_Texture* sprite_texture;
    int sprite_sheet_width;
    int sprite_sheet_height;
    int num_x_sprites;
    int num_y_sprites;
    int w_increment;
    int h_increment;

    Animation animation;
    SDL_Rect sprite_rect;
    short** pixel_data;
    int current_frame;
    int speed;
    Direction direction;

    SDL_Rect bounding_box;
    SDL_Rect dest_rect;

    ~Entity();
};

#endif
