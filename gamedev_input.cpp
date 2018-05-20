#include "gamedev_input.h"

void input_update(Input* input, SDL_Scancode key, u8 pressed)
{
    switch (key)
    {
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_L:
    case SDL_SCANCODE_D:
        input->is_pressed[KEY_RIGHT] = pressed;
        break;
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_K:
    case SDL_SCANCODE_W:
        input->is_pressed[KEY_UP] = pressed;
        break;
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_J:
    case SDL_SCANCODE_S:
        input->is_pressed[KEY_DOWN] = pressed;
        break;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_H:
    case SDL_SCANCODE_A:
        input->is_pressed[KEY_LEFT] = pressed;
        break;
    case SDL_SCANCODE_ESCAPE:
        input->is_pressed[KEY_ESCAPE] = pressed;
        break;
    case SDL_SCANCODE_F:
        input->is_pressed[KEY_F] = pressed;
        break;
    default:
        char* action = pressed ? "pressed" : "released";
        printf("Key %s: %s\n", action, SDL_GetKeyName(SDL_GetKeyFromScancode(key)));
        break;
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
