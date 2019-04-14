#include "gamedev_input.h"

static void initControllers(Input *input)
{
    // TODO(chj): Handle SDL_CONTROLLERDEVICEADDED, SDL_CONTROLLERDEVICEREMOVED, and SDL_CONTROLLERDEVICEREMAPPED
    int maxJoysticks = SDL_NumJoysticks();
    for (int joystickIndex = 0; joystickIndex < maxJoysticks; ++joystickIndex)
    {
        if (joystickIndex >= MAX_CONTROLLERS)
        {
            break;
        }
        if (!SDL_IsGameController(joystickIndex))
        {
            continue;
        }
        input->controllerHandles[joystickIndex] = SDL_GameControllerOpen(joystickIndex);
    }
}

static void destroyControllers(Input *input)
{
    for(int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; ++controllerIndex)
    {
        if (input->controllerHandles[controllerIndex])
        {
            SDL_GameControllerClose(input->controllerHandles[controllerIndex]);
        }
    }
}

static void setKeyState(Input* input, Key key, bool32 isDown)
{
    if (input->keyDown[key] && !isDown)
    {
        input->keyPressed[key] = true;
    }
    input->keyDown[key] = isDown;
}

static void updateInput(Input* input, SDL_Scancode key, bool32 isDown)
{
    switch (key)
    {
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_L:
    case SDL_SCANCODE_D:
        setKeyState(input, KEY_RIGHT, isDown);
        break;
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_K:
    case SDL_SCANCODE_W:
        setKeyState(input, KEY_UP, isDown);
        break;
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_J:
    case SDL_SCANCODE_S:
        setKeyState(input, KEY_DOWN, isDown);
        break;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_H:
    case SDL_SCANCODE_A:
        setKeyState(input, KEY_LEFT, isDown);
        break;
    case SDL_SCANCODE_ESCAPE:
        setKeyState(input, KEY_ESCAPE, isDown);
        break;
    case SDL_SCANCODE_F:
        setKeyState(input, KEY_F, isDown);
        break;
    case SDL_SCANCODE_C:
        setKeyState(input, KEY_C, isDown);
        break;
    case SDL_SCANCODE_P:
        setKeyState(input, KEY_P, isDown);
        break;
    case SDL_SCANCODE_SPACE:
        setKeyState(input, KEY_SPACE, isDown);
        break;
    case SDL_SCANCODE_I:
        setKeyState(input, KEY_I, isDown);
        break;
    default:
        // char* action = pressed ? "pressed" : "released";
        // printf("Key %s: %s\n", action, SDL_GetKeyName(SDL_GetKeyFromScancode(key)));
        break;
    }
}

static f32 normalizeStickInput(short unnormalizedStick)
{
    f32 result = 0.0f;
    if (unnormalizedStick < 0)
    {
        result = (f32)unnormalizedStick / 32768.0f;
    }
    else
    {
        result = (f32)unnormalizedStick / 32767.0f;
    }
    return result;
}

static void pollInput(Input* input, Game* game)
{
    // Reset all button presses
    // TODO(chj): This seems wasteful
    for (u32 key = 0; key < KEY_COUNT; ++key)
    {
        input->keyPressed[key] = 0;
    }
    for (u32 button = 0; button < BUTTON_COUNT; ++button)
    {
        input->buttonPressed[button] = 0;
    }

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYUP:
            updateInput(input, event.key.keysym.scancode, 0);
            break;
        case SDL_QUIT:
            game->running = 0;
            break;
        case SDL_KEYDOWN:
            updateInput(input, event.key.keysym.scancode, true);
            break;
            // case SDL_MOUSEMOTION:
            //     input_handle_mouse(&input);
            //     break;
        }
    }
    // Check controller input
    // TODO(chj): Should we use events for this? What's the difference?
    for (int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; ++controllerIndex)
    {
        SDL_GameController *controller = input->controllerHandles[controllerIndex];
        if(controller && SDL_GameControllerGetAttached(controller))
        {
            // TODO(chj): Handle remaining buttons
            // TODO(chj): Handle keyPressed as distinct from keyDown
            bool32 *pressed = input->buttonPressed;
            pressed[BUTTON_UP] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            pressed[BUTTON_DOWN] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            pressed[BUTTON_LEFT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            pressed[BUTTON_RIGHT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            // bool start = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
            // bool back = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
            // bool leftShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            // bool rightShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            pressed[BUTTON_A] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
            pressed[BUTTON_B] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
            pressed[BUTTON_X] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
            pressed[BUTTON_Y] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);

            short sdlStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            short sdlStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            f32 stickX = normalizeStickInput(sdlStickX);
            f32 stickY = normalizeStickInput(sdlStickY);

            // Account for dead zone
            input->stickX = (stickX > -0.1f && stickX < 0.1f) ? 0.0f : stickX;
            input->stickY = (stickY > -0.1f && stickY < 0.1f) ? 0.0f : stickY;
        }
        else
        {
            // TODO: This controller is not plugged in.
        }
    }

    if (input->keyPressed[KEY_I])
    {
        startInventoryMode(game);
    }
}

// void input_handle_mouse(Input* input)
// {
//     // get vector from center of player to mouse cursor
//     Point hero_center = {
//         hero.e.dest_rect.x + (i32)(0.5 * hero.e.dest_rect.w),
//         hero.e.dest_rect.y + (i32)(0.5 * hero.e.dest_rect.h)
//     };
//     Vec2 mouse_relative_to_hero;
//     mouse_relative_to_hero.x = hero_center.x - ((float)event.motion.x + camera.x);
//     mouse_relative_to_hero.y = hero_center.y - ((float)event.motion.y + camera.y);

//     float angle = 0;
//     if (mouse_relative_to_hero.x != 0 && mouse_relative_to_hero.y != 0)
//     {
//         angle = atan2f(mouse_relative_to_hero.y, mouse_relative_to_hero.x) + PI;
//     }

//     if (angle != 0)
//     {
//         hero.e.direction = get_direction_from_angle(angle);
//     }
//     break;
// }
// case SDL_MOUSEBUTTONUP:
// {
//     if (event.button.button == SDL_BUTTON_LEFT)
//     {
//         hero.swing_club = true;
//     }
//     break;
// }
