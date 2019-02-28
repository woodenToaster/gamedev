#ifndef GD_DEFINITIONS_H
#define GD_DEFINITIONS_H

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef int32_t bool32;

enum
{
    GD_FALSE,
    GD_TRUE
};

enum EntityType
{
    ET_TILE,
    ET_BUFFALO,
    ET_HERO,
    ET_NUM_TYPES
};

enum CardinalDir
{
    CARDINAL_NORTH,
    CARDINAL_SOUTH,
    CARDINAL_WEST,
    CARDINAL_EAST,
    CARDINAL_COUNT,
};

enum Direction
{
    DIR_UP,
    DIR_UP_RIGHT,
    DIR_RIGHT,
    DIR_DOWN_RIGHT,
    DIR_DOWN,
    DIR_DOWN_LEFT,
    DIR_LEFT,
    DIR_UP_LEFT,
    DIR_COUNT
};

enum Sounds
{
    SOUND_MUD,
    SOUND_COUNT
};

#endif
