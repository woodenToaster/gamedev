
SDL_Texture* createTextureFromPng(const char* fname, SDL_Renderer *renderer)
{
    unsigned char *img_data;
    int width;
    int height;
    int channels_in_file;
    img_data = stbi_load(fname, &width, &height, &channels_in_file, 0);

    if (!img_data)
    {
        printf("Loading image failed: %s\n", stbi_failure_reason());
        exit(1);
    }
    u32 rmask;
    u32 gmask;
    u32 bmask;
    u32 amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    bool bigEndian = true;
    int shift = (channels_in_file == STBI_rgb) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else
    bool bigEndian = false;
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = (channels_in_file == STBI_rgb) ? 0 : 0xff000000;
#endif

    int depth;
    int pitch;
    u32 pixelFormat;
    if (channels_in_file == STBI_rgb)
    {
        depth = 24;
        pitch = 3 * width;
        pixelFormat = bigEndian ? SDL_PIXELFORMAT_RGB888 : SDL_PIXELFORMAT_BGR888;
    }
    else
    {
        depth = 32;
        pitch = 4 * width;
        pixelFormat = bigEndian ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_ABGR8888;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STATIC,
                                             width, height);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(texture, NULL, (void*)img_data, pitch);

    stbi_image_free(img_data);

    return texture;
}
