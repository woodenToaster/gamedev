#include "gamedev_sprite_sheet.h"

void sprite_sheet_destroy(SpriteSheet* ss)
{
    if (ss)
    {
        SDL_DestroyTexture(ss->sheet);
        ss->sheet = NULL;
        SDL_FreeSurface(ss->surface);
        ss->surface = NULL;
        stbi_image_free(ss->data);
        ss->data = NULL;
    }
}

void sprite_sheet_load(SpriteSheet* ss, const char* path, int x_sprites, int y_sprites, SDL_Renderer* renderer)
{
    ss->surface = create_surface_from_png(&ss->data, path);
    ss->sheet = SDL_CreateTextureFromSurface(renderer, ss->surface);

    SDL_QueryTexture(ss->sheet, NULL, NULL, &ss->width, &ss->height);
    ss->sprite_width = ss->width / x_sprites;
    ss->sprite_height = ss->height / y_sprites;
    ss->num_x = x_sprites;
    ss->num_y = y_sprites;
}
