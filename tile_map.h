#ifndef TILE_MAP_H
#define TILE_MAP_H

struct Animation
{
    int total_frames;
    int current_frame;
    Uint32 delay;
    Uint32 elapsed;

    void update(Uint32 elapsed_last_frame);
};

struct Tile
{
    static const Uint32 NONE = 0x0;
    static const Uint32 SOLID = 0x01;
    static const Uint32 WATER = 0x01 << 1;
    static const Uint32 QUICKSAND = 0x01 << 2;
    static const Uint32 STICKY = 0x01 << 3;
    static const Uint32 REVERSE = 0x01 << 4;
    static const Uint32 WARP = 0x01 << 5;
    static const Uint32 FIRE = 0x01 << 6;

    static const int tile_width = 80;
    static const int tile_height = 80;

    Uint32 flags;
    Uint32 color;
    SDL_Surface* sprite;
    SDL_Rect sprite_rect;
    Animation animation;
    bool active;

    Tile::Tile(Uint32 flags, Uint32 color, const char* sprite_path=NULL);
    ~Tile();

    bool is_fire();
    bool is_solid();
    bool is_slow();
    bool is_warp();
    Uint32 get_flags();
    Uint32 get_color();
    Uint64 as_u64();

    void set_sprite_size(int width, int height);
    void draw(SDL_Surface* map_surface, SDL_Rect* tile_rect);
};

struct Map
{
    int rows;
    int cols;
    Tile** tiles;
    bool is_active;
    int width_pixels;
    int height_pixels;
    int tile_rows_per_screen;
    int tile_cols_per_screen;

    Map::Map(int rows, int cols, Tile** tiles);
};

#endif
