#ifndef GD_SPRITE_SHEET_H
#define GD_SPRITE_SHEET_H

struct SpriteSheet
{
    int width;
    int height;
    int num_x;
    int num_y;
    int sprite_width;
    int sprite_height;
    SDL_Surface* sheet;
    unsigned char* data;
};
#endif
