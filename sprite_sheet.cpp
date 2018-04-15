#include "sprite_sheet.h"

void sprite_sheet_destroy(SpriteSheet* ss)
{
    if (ss)
    {
        SDL_FreeSurface(ss->sheet);
        ss->sheet = NULL;
    }
}

void sprite_sheet_load(SpriteSheet* ss, const char* path, int x_sprites, int y_sprites)
{
    SDL_Surface* img = IMG_Load(path);
    if (!img) {
        printf("Failed to load image from %s: %s", path, SDL_GetError());
        exit(1);
    }
    ss->sheet = img;
    ss->width = ss->sheet->w;
    ss->height = ss->sheet->h;
    ss->sprite_width = ss->width / x_sprites;
    ss->sprite_height = ss->height / y_sprites;
    ss->num_x = x_sprites;
    ss->num_y = y_sprites;
}
