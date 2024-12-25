#include "category_blocker.h"
#include "ui.h"
#include "ui_constants.h"
#include "categories.h"
#include <SDL2/SDL_ttf.h>

static void draw_menu(UI* ui);
static void handle_menu_selection(UI* ui);

void category_blocker_init(UI* ui) {
    ui->blocker.menu_selection = 0;
    ui->blocker.active = false;
}

void category_blocker_show(UI* ui) {
    ui->blocker.active = true;
    ui->blocker.menu_selection = 0;
}

void category_blocker_hide(UI* ui) {
    ui->blocker.active = false;
}

void category_blocker_handle_input(UI* ui, SDL_Event* event) {
    if (!ui->blocker.active) return;
    
    switch (event->type) {
        case SDL_KEYDOWN:
            switch (event->key.keysym.sym) {
                case SDLK_UP:
                    ui->blocker.menu_selection = 
                        (ui->blocker.menu_selection - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
                    break;
                    
                case SDLK_DOWN:
                    ui->blocker.menu_selection = 
                        (ui->blocker.menu_selection + 1) % MENU_OPTIONS_COUNT;
                    break;
                    
                case SDLK_RETURN:
                    handle_menu_selection(ui);
                    break;
                    
                case SDLK_ESCAPE:
                    category_blocker_hide(ui);
                    break;
            }
            break;
    }
}

void category_blocker_draw(UI* ui) {
    if (!ui->blocker.active) return;
    
    // Draw semi-transparent background
    SDL_SetRenderDrawBlendMode(ui->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ui->renderer, 0, 0, 0, 192);
    SDL_RenderFillRect(ui->renderer, NULL);
    
    draw_menu(ui);
}

static void draw_menu(UI* ui) {
    const int menu_width = 300;
    const int menu_height = MENU_OPTIONS_COUNT * 40 + 20;
    const int menu_x = (WINDOW_WIDTH - menu_width) / 2;
    const int menu_y = (WINDOW_HEIGHT - menu_height) / 2;
    
    // Draw menu background
    SDL_Rect menu_rect = {menu_x, menu_y, menu_width, menu_height};
    SDL_SetRenderDrawColor(ui->renderer, 32, 32, 32, 255);
    SDL_RenderFillRect(ui->renderer, &menu_rect);
    
    // Draw menu options
    const char* options[] = {
        "Block Category",
        "Unblock Category",
        "Clear All",
        "Back"
    };
    
    for (int i = 0; i < MENU_OPTIONS_COUNT; i++) {
        SDL_Color color = {255, 255, 255, 255};
        if (i == ui->blocker.menu_selection) {
            color.r = 255;
            color.g = 255;
            color.b = 0;
        }
        
        // Draw option text
        SDL_Surface* text = TTF_RenderText_Blended(ui->font, options[i], color);
        if (text) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(ui->renderer, text);
            if (texture) {
                SDL_Rect dst = {
                    menu_x + 10,
                    menu_y + 10 + i * 40,
                    text->w,
                    text->h
                };
                SDL_RenderCopy(ui->renderer, texture, NULL, &dst);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(text);
        }
    }
}

static void handle_menu_selection(UI* ui) {
    switch (ui->blocker.menu_selection) {
        case MENU_BLOCK:
            // Show category selection dialog
            break;
            
        case MENU_UNBLOCK:
            // Show blocked categories list
            break;
            
        case MENU_CLEAR:
            category_filter_clear(ui->category_filter);
            break;
            
        case MENU_BACK:
            category_blocker_hide(ui);
            break;
    }
} 