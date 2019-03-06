#ifndef GD_INPUT_H
#define GD_INPUT_H

#define MAX_CONTROLLERS 2

enum Key
{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ESCAPE,
    KEY_F,
    KEY_SPACE,
    KEY_COUNT
};

enum Buttons
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_COUNT
};

struct Input
{
    u8 key_pressed[KEY_COUNT];
    bool32 button_pressed[BUTTON_COUNT];
    SDL_GameController *controllerHandles[MAX_CONTROLLERS];
    f32 stickX;
    f32 stickY;
};
#endif
