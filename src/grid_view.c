#include "ui.h"
#include <SDL2/SDL_image.h>

void ui_draw_grid(UI* ui) {
    if (!ui->playlist) return;
    
    SDL_Color white = {255, 255, 255, 255};
    
    // Calculate grid dimensions
    int item_width = (WINDOW_WIDTH - 60) / ui->grid_columns;
    int item_height = item_width * 9 / 16;  // 16:9 aspect ratio
    int spacing = 20;
    
    // Draw grid items
    int x = 20;
    int y = 80;
    int grid_index = ui->scroll_offset * ui->grid_columns;
    
    for (int row = 0; row < ui->grid_rows && grid_index < (int)ui->playlist->count; row++) {
        for (int col = 0; col < ui->grid_columns && grid_index < (int)ui->playlist->count; col++) {
            PlaylistItem* item = &ui->playlist->items[grid_index];
            SDL_Rect item_rect = {x + col * (item_width + spacing),
                                y + row * (item_height + spacing),
                                item_width, item_height};
            
            // Draw selection highlight
            if (grid_index == ui->selected_item) {
                SDL_SetRenderDrawColor(ui->renderer, 0x00, 0xA5, 0xE0, 0xFF);
                SDL_Rect highlight = {item_rect.x - 2, item_rect.y - 2,
                                    item_rect.w + 4, item_rect.h + 4};
                SDL_RenderFillRect(ui->renderer, &highlight);
            }
            
            // Draw thumbnail or placeholder
            SDL_Texture* thumb = NULL;
            if (item->tvg_logo) {
                thumb = ui_get_thumbnail(ui, item->tvg_logo);
            }
            
            if (thumb) {
                SDL_RenderCopy(ui->renderer, thumb, NULL, &item_rect);
            } else {
                // Draw placeholder
                SDL_SetRenderDrawColor(ui->renderer, 0x40, 0x40, 0x40, 0xFF);
                SDL_RenderFillRect(ui->renderer, &item_rect);
            }
            
            // Draw title
            SDL_Rect title_bg = {item_rect.x, item_rect.y + item_rect.h - 30,
                                item_rect.w, 30};
            SDL_SetRenderDrawColor(ui->renderer, 0, 0, 0, 192);
            SDL_RenderFillRect(ui->renderer, &title_bg);
            
            draw_text(ui->renderer, ui->font, item->title,
                     item_rect.x + 5, title_bg.y + 5, white, false);
            
            grid_index++;
        }
    }
    
    // Draw controls
    draw_text(ui->renderer, ui->font,
              "A: Select   B: Back   Y: View Mode   X: Search",
              WINDOW_WIDTH/2, WINDOW_HEIGHT - 30, white, true);
}

void ui_draw_epg_grid(UI* ui) {
    if (!ui->epg) return;
    
    SDL_Color white = {255, 255, 255, 255};
    
    // Calculate time slots
    time_t now = time(NULL);
    time_t start_time = now - (now % 1800);  // Round to nearest 30 minutes
    int time_slots = 8;  // Show 4 hours
    int slot_width = (WINDOW_WIDTH - 200) / time_slots;
    
    // Draw time headers
    for (int i = 0; i < time_slots; i++) {
        time_t slot_time = start_time + i * 1800;
        struct tm* tm = localtime(&slot_time);
        char time_str[6];
        strftime(time_str, sizeof(time_str), "%H:%M", tm);
        
        draw_text(ui->renderer, ui->font, time_str,
                 200 + i * slot_width, 40, white, true);
    }
    
    // Draw channels and programs
    int y = 80;
    int row = 0;
    
    for (size_t i = ui->scroll_offset;
         i < ui->playlist->count && y < WINDOW_HEIGHT - 60;
         i++) {
        PlaylistItem* item = &ui->playlist->items[i];
        if (!item->tvg_id) continue;
        
        // Draw channel info
        SDL_Rect channel_rect = {0, y, 190, 50};
        SDL_SetRenderDrawColor(ui->renderer, 0x40, 0x40, 0x40, 0xFF);
        SDL_RenderFillRect(ui->renderer, &channel_rect);
        
        // Draw channel logo
        if (item->tvg_logo) {
            SDL_Texture* logo = ui_get_thumbnail(ui, item->tvg_logo);
            if (logo) {
                SDL_Rect logo_rect = {5, y + 5, 40, 40};
                SDL_RenderCopy(ui->renderer, logo, NULL, &logo_rect);
            }
        }
        
        draw_text(ui->renderer, ui->font, item->title,
                 50, y + 15, white, false);
        
        // Draw programs
        for (int slot = 0; slot < time_slots; slot++) {
            time_t slot_start = start_time + slot * 1800;
            time_t slot_end = slot_start + 1800;
            
            EPGProgram* prog = epg_get_program_at(ui->epg, item->tvg_id, slot_start);
            if (prog) {
                // Calculate program position and width
                int prog_x = 200 + (prog->start_time - start_time) * slot_width / 1800;
                int prog_w = (prog->end_time - prog->start_time) * slot_width / 1800;
                
                SDL_Rect prog_rect = {prog_x, y, prog_w, 50};
                
                // Highlight current program
                if (now >= prog->start_time && now < prog->end_time) {
                    SDL_SetRenderDrawColor(ui->renderer, 0x00, 0xA5, 0xE0, 0xFF);
                } else {
                    SDL_SetRenderDrawColor(ui->renderer, 0x30, 0x30, 0x30, 0xFF);
                }
                SDL_RenderFillRect(ui->renderer, &prog_rect);
                
                // Draw program title
                draw_text(ui->renderer, ui->font, prog->title,
                         prog_x + 5, y + 15, white, false);
            }
        }
        
        y += 60;
        row++;
    }
} 