#ifndef SPRITE_SHEET_H
#define SPRITE_SHEET_H

struct SpriteSheet {
    SDL_Surface* sheet;
    int width;
    int height;
    int num_x;
    int num_y;
    int sprite_width;
    int sprite_height;

    ~SpriteSheet();
    void load(const char* path, int sprite_width, int sprite_height);
};
#endif
