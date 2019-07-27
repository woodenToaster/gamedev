#include "gamedev_font.h"

void generateFontData(FontMetadata *fontMetadata, Game *game)
{
    EntireFile fontFile = platform.readEntireFile("fonts/arialbd.ttf");

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

        TextureHandle texture = createTextureFromGreyscaleBitmap(game, stb_bitmap, bitmapWidth, bitmapHeight);
        fontMetadata->textures[codepoint] = texture;
    }

}

void destroyFontMetadata(FontMetadata *fmd)
{
    for (int i = '!'; i <= '~'; ++i)
    {
        rendererAPI.destroyTexture(fmd->textures[i]);
    }
}

