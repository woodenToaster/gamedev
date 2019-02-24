#ifndef GD_INPUT_H
#define GD_INPUT_H

enum Key
{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ESCAPE,
    KEY_F,
    KEY_SPACE,
    KEY_LAST
};

struct Input
{
    u8 is_pressed[KEY_LAST];
};
#endif
