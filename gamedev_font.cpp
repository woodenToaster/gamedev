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
    if (f->texture)
    {
        SDL_DestroyTexture(f->texture);
        f->texture = NULL;
    }

    u8 *pixel = stb_bitmap;
    f->bitmap = (u32*)malloc(sizeof(u32) * f->width * f->height * 4);
    u32 *tmp = f->bitmap;

    for (int i = 0; i < f->height * f->width; ++i)
    {
        u8 val = *pixel++;
        *tmp++ = ((val << 24) | (val << 16) | (val << 8) | (val << 0));
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
    free(f->bitmap);
}

void ttf_font_update_pos(TTFFont* t, int x, int y)
{
    t->dest.x = x;
    t->dest.y = y;
}

void ttf_font_destroy(TTFFont* f)
{
    SDL_DestroyTexture(f->texture);
}

// TODO(chj): Account for wrapping off the viewport
void text_draw(Game* g, SDL_Texture *codepointTextures[], FontMetadata *fontMetadata,
               CodepointMetadata *codepointMetadata, char* text, i32 x=0, i32 y=0)
{
    // Leave a little padding in case the character extends left
    i32 xpos = 2 + x;
    i32 baseline = fontMetadata->baseline + y;
    u32 at = 0;
    while (text[at])
    {
        CodepointMetadata *cpm = &codepointMetadata[text[at]];
        i32 width = cpm->x1 - cpm->x0;
        i32 height = cpm->y1 - cpm->y0;
        SDL_Rect source = {cpm->x0, cpm->y0, width, height};
        SDL_Rect dest = {xpos, baseline + cpm->y0, width, height};

        if (text[at] != ' ')
        {
            SDL_Texture *t = codepointTextures[text[at]];
            SDL_RenderCopy(g->renderer, t, NULL, &dest);
            xpos += (int)(cpm->advance * fontMetadata->scale);
        }
        else
        {
            // TODO(chj): Don't hardcode space size. We'd like to get it from cpm->advance
            // but we don't store any metadata for a space
            xpos += (int)(750 * fontMetadata->scale);
        }

        if (text[at + 1])
        {
            // add kerning value specific to this character and the next
            xpos += (int)(fontMetadata->scale * stbtt_GetCodepointKernAdvance(&fontMetadata->info,
                                                                              text[at], text[at + 1]));
        }
        ++at;
    }
}

void text_draw(Game* g, TTFFont* hundreds, TTFFont* tens, TTFFont* ones)
{
    SDL_RenderCopy(g->renderer, hundreds->texture, NULL, &hundreds->dest);
    SDL_RenderCopy(g->renderer, tens->texture, NULL, &tens->dest);
    SDL_RenderCopy(g->renderer, ones->texture, NULL, &ones->dest);
}
