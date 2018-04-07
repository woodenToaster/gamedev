#include "sprite_sheet.h"

void SpriteSheet::load(const char* path, int num_x_sprites, int num_y_sprites)
{
    SDL_Surface* img = IMG_Load(path);
    if (!img) {
        printf("Failed to load image from %s: %s", path, SDL_GetError());
        exit(1);
    }
    sheet = img;
    width = sheet->w;
    height = sheet->h;
    this->sprite_width = width / num_x_sprites;
    this->sprite_height = height / num_y_sprites;
    this->num_x = num_x_sprites;
    this->num_y = num_y_sprites;
}
