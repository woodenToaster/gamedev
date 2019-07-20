#include "gamedev_font.h"

void generateFontData(FontMetadata *fontMetadata, Game *game)
{
    EntireFile fontFile = readEntireFile("fonts/arialbd.ttf");

    fontMetadata->size = 20;
    stbtt_InitFont(&fontMetadata->info, fontFile.contents, 0);
    fontMetadata->scale = stbtt_ScaleForPixelHeight(&fontMetadata->info, fontMetadata->size);
    stbtt_GetFontVMetrics(&fontMetadata->info, &fontMetadata->ascent, &fontMetadata->descent ,
                          &fontMetadata->lineGap);
    fontMetadata->baseline = (int)(fontMetadata->ascent * fontMetadata->scale);

    for (char codepoint = '!'; codepoint <= '~'; ++codepoint)
    {
        CodepointMetadata *cpMeta = &fontMetadata->codepointMetadata[codepoint];
        stbtt_GetCodepointHMetrics(&fontMetadata->info, codepoint, &cpMeta->advance, &cpMeta->leftSideBearing);
        stbtt_GetCodepointBitmapBoxSubpixel(&fontMetadata->info, codepoint, fontMetadata->scale,
                                            fontMetadata->scale, /* x_shift */ 0, 0,
                                            &cpMeta->x0, &cpMeta->y0, &cpMeta->x1, &cpMeta->y1);
        i32 bitmapWidth = cpMeta->x1 - cpMeta->x0;
        i32 bitmapHeight = cpMeta->y1 - cpMeta->y0;
        u8* stb_bitmap = (u8*)malloc(sizeof(u8) * bitmapWidth * bitmapHeight);
        stbtt_MakeCodepointBitmapSubpixel(&fontMetadata->info, stb_bitmap, bitmapWidth, bitmapHeight, bitmapWidth,
                                          fontMetadata->scale, fontMetadata->scale, /* x_shift */ 0, 0, codepoint);

        SDL_Surface* surface = SDL_CreateRGBSurface(0, bitmapWidth, bitmapHeight, 32,
                                                    0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        if (!surface)
        {
            printf("%s\n", SDL_GetError());
            exit(1);
        }

        SDL_LockSurface(surface);
        u8 *srcPixel = stb_bitmap;
        u32 *destPixel = (u32*)surface->pixels;

        for (int i = 0; i < bitmapHeight * bitmapWidth; ++i)
        {
            u8 val = *srcPixel++;
            *destPixel++ = ((val << 24) | (val << 16) | (val << 8) | (val << 0));
        }
        SDL_UnlockSurface(surface);

        SDL_Texture *texture = SDL_CreateTextureFromSurface(game->renderer, surface);
        fontMetadata->textures[codepoint] = texture;
        stbtt_FreeBitmap(stb_bitmap, 0);
        SDL_FreeSurface(surface);
    }
}

void destroyFontMetadata(FontMetadata *fmd)
{
    for (int i = '!'; i <= '~'; ++i)
    {
        SDL_DestroyTexture(fmd->textures[i]);
    }
}

