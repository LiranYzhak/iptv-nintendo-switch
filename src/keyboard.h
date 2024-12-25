#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>
#include "ui.h"

void keyboard_handle_input(UI* ui, SDL_Event* event);
void keyboard_handle_keypress(UI* ui, SDL_Keycode key);
void keyboard_handle_click(UI* ui, int x, int y);

#endif // KEYBOARD_H 