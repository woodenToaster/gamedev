#ifndef GD_SPRITE_SHEET_H
#define GD_SPRITE_SHEET_H

struct Sprite
{
    i32 x;
    i32 y;
    i32 width;
    i32 height;
    i32 offsetX;
    i32 offsetY;
};

struct SpriteSheet
{
    int width;
    int height;
    int num_x;
    int num_y;
    int sprite_width;
    int sprite_height;
    SDL_Surface* surface;
    SDL_Texture* sheet;
    unsigned char* data;
};
#endif
