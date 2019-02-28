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
    unsigned char *stb_bitmap = stbtt_GetCodepointBitmap(&f->font, 0, f->scale, character,
                                                         &f->width, &f->height, 0, 0);
    // TODO(chj): Convert the bitmap to an RGBA 32 bit value and make a surface.
    // Then we can free f->bitmap right away.
    if (f->texture)
    {
        SDL_DestroyTexture(f->texture);
        f->texture = NULL;
    }

    u8 *pixel = stb_bitmap;
    f->bitmap = (u32*)malloc(sizeof(u32) * f->width * f->height * 4);
    u32 *tmp = f->bitmap;

    for (int i = 0; i < f->height; ++i)
    {
        for (int j = 0; j < f->width; ++j)
        {
            u8 val = *pixel++;
            *tmp++ = ((val << 24) | (val << 16) | (val << 8) | (val << 0));
        }
    }
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        (void*)f->bitmap,
        f->width, f->height,
        32,
        4 * f->width,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
    );

    if (!surface)
    {
        printf("%s\n", SDL_GetError());
        exit(1);
    }

    f->dest.w = f->width;
    f->dest.h = f->height;

    f->texture = SDL_CreateTextureFromSurface(renderer, surface);
    stbtt_FreeBitmap(stb_bitmap, 0);
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
    free(f->bitmap);
}

void text_draw(Game* g, TTFFont* hundreds, TTFFont* tens, TTFFont* ones)
{
    SDL_RenderCopy(g->renderer, hundreds->texture, NULL, &hundreds->dest);
    SDL_RenderCopy(g->renderer, tens->texture, NULL, &tens->dest);
    SDL_RenderCopy(g->renderer, ones->texture, NULL, &ones->dest);
}
