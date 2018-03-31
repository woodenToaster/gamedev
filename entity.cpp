#include "entity.h"

Entity::~Entity() {
    if (sprite_sheet) {
        SDL_FreeSurface(sprite_sheet);
    }
    if (sprite_texture) {
        SDL_DestroyTexture(sprite_texture);
    }
}
