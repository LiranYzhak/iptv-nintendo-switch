#include "ui.h"
#include "drawing.h"
#include <SDL2/SDL_ttf.h>

void ui_draw(UI* ui) {
    // Clear screen
    SDL_SetRenderDrawColor(ui->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ui->renderer);
    
    // Draw current state
    switch (ui->state) {
        case UI_STATE_PLAYLIST:
            ui_draw_playlist(ui);
            break;
            
        case UI_STATE_PLAYING:
            ui_draw_player(ui);
            break;
            
        case UI_STATE_EPG:
            ui_draw_epg(ui);
            break;
            
        case UI_STATE_CATEGORIES:
            // Use existing category drawing code
            break;
            
        case UI_STATE_SEARCH:
            // Use existing search drawing code
            break;
    }
    
    // Draw status message if active
    if (ui->status_message && SDL_GetTicks() < ui->status_timeout) {
        ui_draw_status(ui);
    }
    
    SDL_RenderPresent(ui->renderer);
} 