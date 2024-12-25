#include "ui.h"
#include "ui_constants.h"
#include <switch.h>
#include <SDL2/SDL.h>

void ui_handle_input(UI* ui) {
    SDL_Event event;
    
    // Calculate visible items
    int visible_items = (WINDOW_HEIGHT - SCROLL_MARGIN * 2) / ITEM_HEIGHT;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                ui->quit = true;
                break;
                
            case SDL_CONTROLLERBUTTONDOWN:
                switch (event.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        if (ui->selected_item > 0) {
                            ui->selected_item--;
                            if (ui->selected_item < ui->scroll_offset) {
                                ui->scroll_offset = ui->selected_item;
                            }
                        }
                        break;
                        
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        if (ui->playlist && ui->selected_item < (int)ui->playlist->count - 1) {
                            ui->selected_item++;
                            if (ui->selected_item >= ui->scroll_offset + (WINDOW_HEIGHT - 120) / ITEM_HEIGHT) {
                                ui->scroll_offset = ui->selected_item - (WINDOW_HEIGHT - 120) / ITEM_HEIGHT + 1;
                            }
                        }
                        break;
                        
                    case SDL_CONTROLLER_BUTTON_A:
                        if (ui->state == UI_STATE_PLAYLIST && ui->playlist) {
                            PlaylistItem* item = &ui->playlist->items[ui->selected_item];
                            if (player_load(ui->player, item)) {
                                player_play(ui->player);
                                ui_set_state(ui, UI_STATE_PLAYING);
                            }
                        }
                        break;
                        
                    case SDL_CONTROLLER_BUTTON_B:
                        if (ui->state == UI_STATE_PLAYING) {
                            player_stop(ui->player);
                            ui_set_state(ui, UI_STATE_PLAYLIST);
                        }
                        break;
                }
                break;
        }
    }
} 