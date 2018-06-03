#ifndef GD_FONT_H
#define GD_FONT_H

struct TTFFile
{
    char buf[1 << 25];
    char* fname;
    u8 initialized;
};

struct TTFFont
{
    stbtt_fontinfo font;
    char* text;
    f32 size;
    f32 scale;
    int width;
    int height;
    unsigned char* bitmap;
    SDL_Texture* texture;
    TTFFile* ttf_file;
    SDL_Rect dest;
};
#endif
