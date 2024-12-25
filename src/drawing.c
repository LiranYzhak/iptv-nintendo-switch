#include "drawing.h"

void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
               int x, int y, SDL_Color color, bool centered) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;
    
    SDL_Rect dest = {x, y, 0, 0};
    SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
    if (centered) {
        dest.x -= dest.w / 2;
    }
    
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
} 