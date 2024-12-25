#ifndef DRAWING_H
#define DRAWING_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
               int x, int y, SDL_Color color, bool centered);

#endif // DRAWING_H 