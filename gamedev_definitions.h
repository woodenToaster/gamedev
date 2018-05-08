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

enum
{
    GD_FALSE,
    GD_TRUE
};

enum CardinalDir
{
    CARDINAL_UP,
    CARDINAL_DOWN,
    CARDINAL_LEFT,
    CARDINAL_RIGHT,
};

enum Direction
{
    UP,
    UP_RIGHT,
    RIGHT,
    DOWN_RIGHT,
    DOWN,
    DOWN_LEFT,
    LEFT,
    UP_LEFT
};
#endif
