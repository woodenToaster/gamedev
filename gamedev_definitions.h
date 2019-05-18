#ifndef GD_DEFINITIONS_H
#define GD_DEFINITIONS_H

#define internal static
#define local_persist static
#define global_variable static

#define InvalidCodePath assert(!"InvalidCodePath")

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

#define MEGABYTES(n) ((n) * 1024 * 1024)
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

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
#endif
