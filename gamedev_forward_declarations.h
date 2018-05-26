struct Tile;
struct Plan;
struct Game;
u8 tile_is_solid(Tile* t);
u8 tile_is_slow(Tile* t);
u8 tile_is_warp(Tile* t);
void map_do_warp(Game* game);
