#include "entity.h"

Entity::~Entity() {
    if (sprite_sheet.sheet) {
        SDL_FreeSurface(sprite_sheet.sheet);
    }
    // if (sprite_texture) {
    //     SDL_DestroyTexture(sprite_texture);
    // }
}

void SpriteSheet::load(const char* path, int sprite_w, int sprite_h) {
    SDL_Surface* img = IMG_Load(path);
    if (!img) {
        printf("Failed to load image from %s: %s", path, SDL_GetError());
        exit(1);
    }
    sheet = img;
    width = sheet->w;
    height = sheet->h;
    this->sprite_width = sprite_w;
    this->sprite_height = sprite_h;
    this->num_x = width / sprite_w;
    this->num_y = height / sprite_h;
}
