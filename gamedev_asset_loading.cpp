
SDL_Surface* create_surface_from_png(unsigned char** img_data, const char* fname)
{
    unsigned char *i_data;
    int width, height, channels_in_file;
    i_data = stbi_load(fname, &width, &height, &channels_in_file, 0);

    if (!i_data)
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
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)(i_data), width, height, depth, pitch,
                                                    rmask, gmask, bmask, amask);

#if DEBUG
    FILE *stb_data = fopen("stb_data.out", "w");
    FILE *sdl_data = fopen("sdl_data.out", "w");
    u32* pixels = (u32*)i_data;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (pixels[y*height + x] & surface->format->Amask == 0)
            {
                fprintf(stb_data, "%d, ", 0);
            }
            else
            {
                fprintf(stb_data, "%d, ", 1);
            }
        }
        printf("\n");
    }
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int pd_idx = i * width + j;
            if ((((u32*)surface->pixels)[i * width + j] & surface->format->Amask) == 0)
            {
                fprintf(sdl_data, "%d, ", 0);
            }
            else
            {
                fprintf(sdl_data, "%d, ", 1);
            }
        }
        printf("\n");
    }
    fclose(stb_data);
    fclose(sdl_data);
#endif

    if (!surface)
    {
        printf("Failed to create surface from png: %s\n", SDL_GetError());
        stbi_image_free(*img_data);
        exit(1);
    }

    return surface;
}
