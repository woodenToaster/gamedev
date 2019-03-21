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
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
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
    bool32 key_pressed[KEY_COUNT];
    bool32 key_down[KEY_COUNT];
    bool32 button_pressed[BUTTON_COUNT];
    bool32 button_down[BUTTON_COUNT];
    SDL_GameController *controllerHandles[MAX_CONTROLLERS];
    f32 stickX;
    f32 stickY;
};

static void initControllers(Input *input);
static void destroyControllers(Input *input);
static void updateInput(Input* input, SDL_Scancode key, u8 pressed);
static void pollInput(Input* input, Game* game);
#endif
