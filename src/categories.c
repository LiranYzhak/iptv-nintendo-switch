#include "categories.h"
#include "ui.h"
#include "playlist.h"
#include <stdlib.h>
#include <string.h>

static void update_category_list(UI* ui);
static bool add_category_if_new(UI* ui, const char* category);

void ui_update_categories(UI* ui) {
    if (!ui || !ui->playlist) return;
    
    // Clear existing categories
    for (int i = 0; i < ui->category_count; i++) {
        free(ui->categories[i]);
    }
    free(ui->categories);
    ui->categories = NULL;
    ui->category_count = 0;
    
    // Collect unique categories
    for (size_t i = 0; i < ui->playlist->count; i++) {
        PlaylistItem* item = &ui->playlist->items[i];
        if (item->group && strlen(item->group) > 0) {
            add_category_if_new(ui, item->group);
        }
    }
    
    // Sort categories alphabetically
    if (ui->category_count > 1) {
        qsort(ui->categories, ui->category_count, sizeof(char*), 
              (int (*)(const void*, const void*))strcmp);
    }
}

static bool add_category_if_new(UI* ui, const char* category) {
    // Check if category already exists
    for (int i = 0; i < ui->category_count; i++) {
        if (strcmp(ui->categories[i], category) == 0) {
            return false;
        }
    }
    
    // Add new category
    char** new_categories = realloc(ui->categories, 
                                  (ui->category_count + 1) * sizeof(char*));
    if (!new_categories) return false;
    
    ui->categories = new_categories;
    ui->categories[ui->category_count] = strdup(category);
    if (!ui->categories[ui->category_count]) return false;
    
    ui->category_count++;
    return true;
}

void ui_draw_categories(UI* ui) {
    if (!ui || !ui->categories) return;
    
    // Draw category list
    int y = 60;
    SDL_Color white = {255, 255, 255, 255};
    
    for (int i = 0; i < ui->category_count; i++) {
        SDL_Color color = white;
        if (i == ui->selected_category) {
            color.r = 255;
            color.g = 255;
            color.b = 0;
        }
        
        SDL_Surface* text = TTF_RenderText_Blended(ui->font, ui->categories[i], color);
        if (text) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(ui->renderer, text);
            if (texture) {
                SDL_Rect dst = {
                    20,
                    y + i * 40,
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