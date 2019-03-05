#include "gamedev_input.h"

void input_update(Input* input, SDL_Scancode key, u8 pressed)
{
    switch (key)
    {
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_L:
    case SDL_SCANCODE_D:
        input->key_pressed[KEY_RIGHT] = pressed;
        break;
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_K:
    case SDL_SCANCODE_W:
        input->key_pressed[KEY_UP] = pressed;
        break;
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_J:
    case SDL_SCANCODE_S:
        input->key_pressed[KEY_DOWN] = pressed;
        break;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_H:
    case SDL_SCANCODE_A:
        input->key_pressed[KEY_LEFT] = pressed;
        break;
    case SDL_SCANCODE_ESCAPE:
        input->key_pressed[KEY_ESCAPE] = pressed;
        break;
    case SDL_SCANCODE_F:
        input->key_pressed[KEY_F] = pressed;
        break;
    case SDL_SCANCODE_SPACE:
        input->key_pressed[KEY_SPACE] = pressed;
    default:
        char* action = pressed ? "pressed" : "released";
        printf("Key %s: %s\n", action, SDL_GetKeyName(SDL_GetKeyFromScancode(key)));
        break;
    }
}

void input_poll(Input* input, Game* game, SDL_GameController **controllerHandles)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYUP:
            input_update(input, event.key.keysym.scancode, GD_FALSE);
            break;
        case SDL_QUIT:
            game->running = GD_FALSE;
            break;
        case SDL_KEYDOWN:
            input_update(input, event.key.keysym.scancode, GD_TRUE);
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
        SDL_GameController *controller = controllerHandles[controllerIndex];
        if(controller && SDL_GameControllerGetAttached(controller))
        {
            // TODO(chj): Handle remaining buttons
            bool32 *pressed = input->button_pressed;
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

            i16 sdlStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            i16 sdlStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

            f32 stickX = 0;
            f32 stickY = 0;
            if (sdlStickX < 0)
            {
                stickX = (f32)sdlStickX / 32768.0f;
            }
            else
            {
                stickX = (f32)sdlStickX / 32767.0f;
            }
            if (sdlStickY < 0)
            {
                stickY = (f32)sdlStickY / 32768.0f;
            }
            else
            {
                stickY = (f32)sdlStickY / 32767.0f;
            }

            // Account for dead zone
            input->stickX = (stickX > -0.1f && stickX < 0.1f) ? 0.0f : stickX;
            input->stickY = (stickY > -0.1f && stickY < 0.1f) ? 0.0f : stickY;
        }
        else
        {
            // TODO: This controller is not plugged in.
        }
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
//         hero.swing_club = GD_TRUE;
//     }
//     break;
// }
