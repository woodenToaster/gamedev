#include "gamedev_font.h"

void ttf_file_read(TTFFile* t)
{
    FILE* f = fopen(t->fname, "rb");
    fread(t->buf, 1, 1 << 25, f);
    t->initialized = GD_TRUE;
    fclose(f);
}


void ttf_font_init(TTFFont* f, TTFFile* file, f32 size)
{
    if (!file->initialized)
    {
        ttf_file_read(file);
    }

    f->ttf_file = file;
    f->size = size;

    stbtt_InitFont(&f->font, (u8*)file->buf, stbtt_GetFontOffsetForIndex((u8*)file->buf, 0));
    f->scale = stbtt_ScaleForPixelHeight(&f->font, f->size);
}

void ttf_font_create_bitmap(TTFFont* f, int character, SDL_Renderer* renderer)
{
    f->bitmap = stbtt_GetCodepointBitmap(&f->font, 0, f->scale, character,
                                         &f->width, &f->height, 0, 0);

    if (f->texture)
    {
        SDL_DestroyTexture(f->texture);
        f->texture = NULL;

    }

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        (void*)f->bitmap,
        f->width, f->height,
        8,
        f->width,
        0, 0, 0, 0
    );

    if (!surface)
    {
        printf("%s\n", SDL_GetError());
        exit(1);
    }

    f->dest.w = f->width;
    f->dest.h = f->height;

    SDL_Color grayscale[256];
    for(int i = 0; i < 256; i++){
        grayscale[i].r = (u8)i;
        grayscale[i].g = (u8)i;
        grayscale[i].b = (u8)i;
    }
    SDL_SetPaletteColors(surface->format->palette, grayscale, 0, 256);

    f->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void ttf_font_update_pos(TTFFont* t, int x, int y)
{
    t->dest.x = x;
    t->dest.y = y;
}

void ttf_font_destroy(TTFFont* f)
{
    SDL_DestroyTexture(f->texture);
    stbtt_FreeBitmap(f->bitmap, NULL);
}

void text_draw(Game* g, TTFFont* tens, TTFFont* ones)
{
    SDL_RenderCopy(g->renderer, tens->texture, NULL, &tens->dest);
    SDL_RenderCopy(g->renderer, ones->texture, NULL, &ones->dest);
}
