
SDL_Surface* create_surface_from_png(unsigned char** img_data, const char* fname)
{
    int width, height, channels_in_file;
    *img_data = stbi_load(fname, &width, &height, &channels_in_file, 0);

    if (!*img_data)
    {
        printf("Loading image failed: %s\n", stbi_failure_reason());
        exit(1);
    }
    u32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (channels_in_file == STBI_rgb) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = (channels_in_file == STBI_rgb) ? 0 : 0xff000000;
#endif

    int depth, pitch;
    if (channels_in_file == STBI_rgb)
    {
        depth = 24;
        pitch = 3 * width;
    }
    else
    {
        depth = 32;
        pitch = 4 * width;
    }
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)(*img_data), width, height, depth, pitch,
                                                    rmask, gmask, bmask, amask);

    if (!surface)
    {
        printf("Failed to create surface from png: %s\n", SDL_GetError());
        stbi_image_free(*img_data);
        exit(1);
    }

    return surface;
}
