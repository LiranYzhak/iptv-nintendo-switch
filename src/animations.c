#include "animations.h"
#include "ui.h"
#include <SDL2/SDL.h>

// Forward declarations of static functions
static void animation_draw_fade(UI* ui);
static void animation_draw_zoom(UI* ui);
static void animation_draw_slide_left(UI* ui);
static void animation_draw_slide_right(UI* ui);

void animation_start(UI* ui, AnimationType type) {
    ui->transition.type = type;
    ui->transition.prev_screen = SDL_CreateTexture(ui->renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH, WINDOW_HEIGHT);
        
    ui->transition.next_screen = SDL_CreateTexture(ui->renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH, WINDOW_HEIGHT);
        
    ui->transition.progress = 0.0f;
    ui->transition.start_time = SDL_GetTicks();
    ui->animating = true;
}

void animation_update(UI* ui) {
    if (!ui->animating) return;
    
    Uint32 current_time = SDL_GetTicks();
    float elapsed = (current_time - ui->transition.start_time) / 1000.0f;
    ui->transition.progress = elapsed / ui->transition.duration;
    
    if (ui->transition.progress >= 1.0f) {
        ui->transition.progress = 1.0f;
        ui->animating = false;
    }
    
    switch (ui->transition.type) {
        case ANIM_FADE:
            animation_draw_fade(ui);
            break;
            
        case ANIM_ZOOM:
            animation_draw_zoom(ui);
            break;
            
        case ANIM_SLIDE_LEFT:
            animation_draw_slide_left(ui);
            break;
            
        case ANIM_SLIDE_RIGHT:
            animation_draw_slide_right(ui);
            break;
    }
}

static void animation_draw_fade(UI* ui) {
    // Draw previous screen
    SDL_SetTextureAlphaMod(ui->transition.prev_screen, 
                          255 * (1.0f - ui->transition.progress));
    SDL_RenderCopy(ui->renderer, ui->transition.prev_screen, NULL, NULL);
    
    // Draw next screen
    SDL_SetTextureAlphaMod(ui->transition.next_screen,
                          255 * ui->transition.progress);
    SDL_RenderCopy(ui->renderer, ui->transition.next_screen, NULL, NULL);
}

static void animation_draw_zoom(UI* ui) {
    SDL_Rect src = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_Rect dst = {
        WINDOW_WIDTH/2 * ui->transition.progress,
        WINDOW_HEIGHT/2 * ui->transition.progress,
        WINDOW_WIDTH * (1.0f - ui->transition.progress),
        WINDOW_HEIGHT * (1.0f - ui->transition.progress)
    };
    
    // Draw previous screen zooming out
    SDL_RenderCopy(ui->renderer, ui->transition.prev_screen, NULL, &dst);
    
    // Draw next screen behind it
    SDL_RenderCopy(ui->renderer, ui->transition.next_screen, NULL, NULL);
}

static void animation_draw_slide_left(UI* ui) {
    SDL_Rect prev_rect = {
        -WINDOW_WIDTH * ui->transition.progress, 0,
        WINDOW_WIDTH, WINDOW_HEIGHT
    };
    SDL_Rect next_rect = {
        WINDOW_WIDTH * (1.0f - ui->transition.progress), 0,
        WINDOW_WIDTH, WINDOW_HEIGHT
    };
    
    SDL_RenderCopy(ui->renderer, ui->transition.prev_screen, NULL, &prev_rect);
    SDL_RenderCopy(ui->renderer, ui->transition.next_screen, NULL, &next_rect);
}

static void animation_draw_slide_right(UI* ui) {
    SDL_Rect prev_rect = {
        WINDOW_WIDTH * ui->transition.progress, 0,
        WINDOW_WIDTH, WINDOW_HEIGHT
    };
    SDL_Rect next_rect = {
        -WINDOW_WIDTH * (1.0f - ui->transition.progress), 0,
        WINDOW_WIDTH, WINDOW_HEIGHT
    };
    
    SDL_RenderCopy(ui->renderer, ui->transition.prev_screen, NULL, &prev_rect);
    SDL_RenderCopy(ui->renderer, ui->transition.next_screen, NULL, &next_rect);
} 