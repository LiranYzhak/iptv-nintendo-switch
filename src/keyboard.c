#include "keyboard.h"
#include "search.h"
#include "ui.h"
#include "ui_constants.h"
#include <string.h>
#include <ctype.h>

static const char keyboard_layout[KEYBOARD_ROWS][KEYBOARD_COLS] = {
    "1234567890",
    "qwertyuiop",
    "asdfghjkl;",
    "zxcvbnm,./"
};

void keyboard_init(UI* ui) {
    ui->keyboard.text = calloc(MAX_INPUT_LENGTH, sizeof(char));
    ui->keyboard.active = false;
    ui->keyboard.suggestions = NULL;
    ui->keyboard.suggestion_count = 0;
    ui->keyboard.selected_suggestion = -1;
}

void keyboard_show(UI* ui) {
    ui->keyboard.active = true;
    ui->keyboard.selected_suggestion = -1;
}

void keyboard_hide(UI* ui) {
    ui->keyboard.active = false;
}

bool keyboard_is_visible(UI* ui) {
    return ui->keyboard.active;
}

void keyboard_handle_input(UI* ui, SDL_Event* event) {
    if (!ui->keyboard.active) return;
    
    switch (event->type) {
        case SDL_KEYDOWN:
            keyboard_handle_keypress(ui, event->key.keysym.sym);
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            keyboard_handle_click(ui, event->button.x, event->button.y);
            break;
    }
}

void keyboard_handle_keypress(UI* ui, SDL_Keycode key) {
    size_t len = strlen(ui->keyboard.text);
    
    if (key == SDLK_BACKSPACE && len > 0) {
        ui->keyboard.text[len - 1] = '\0';
    }
    else if (key == SDLK_RETURN) {
        if (ui->keyboard.suggestion_count > 0 && 
            ui->keyboard.selected_suggestion >= 0) {
            // Use selected suggestion
            strncpy(ui->keyboard.text, 
                   ui->keyboard.suggestions[ui->keyboard.selected_suggestion],
                   MAX_INPUT_LENGTH - 1);
        }
        keyboard_hide(ui);
        return;
    }
    else if (key == SDLK_UP) {
        if (ui->keyboard.suggestion_count > 0) {
            if (ui->keyboard.selected_suggestion < 0) {
                ui->keyboard.selected_suggestion = ui->keyboard.suggestion_count - 1;
            } else {
                ui->keyboard.selected_suggestion = 
                    (ui->keyboard.selected_suggestion - 1 + KEYBOARD_ROWS) % KEYBOARD_ROWS;
            }
        }
    }
    else if (key == SDLK_DOWN) {
        if (ui->keyboard.suggestion_count > 0) {
            if (ui->keyboard.selected_suggestion < 0) {
                ui->keyboard.selected_suggestion = 0;
            } else {
                ui->keyboard.selected_suggestion = 
                    (ui->keyboard.selected_suggestion + 1) % KEYBOARD_ROWS;
            }
        }
    }
    else if (key == SDLK_RIGHT) {
        if (ui->keyboard.suggestion_count > 0) {
            if (ui->keyboard.selected_suggestion < 0) {
                ui->keyboard.selected_suggestion = 0;
            } else {
                ui->keyboard.selected_suggestion = 
                    (ui->keyboard.selected_suggestion + 1) % KEYBOARD_COLS;
            }
        }
    }
    else if (isprint(key) && len < MAX_INPUT_LENGTH - 1) {
        ui->keyboard.text[len] = (char)key;
        ui->keyboard.text[len + 1] = '\0';
    }
    
    // Update suggestions
    search_history_update_suggestions(ui);
}

void keyboard_handle_click(UI* ui, int x, int y) {
    // Handle suggestion clicks
    if (ui->keyboard.suggestion_count > 0) {
        int suggestion_y = y - KEYBOARD_ROWS * 40;  // Adjust based on layout
        if (suggestion_y >= 0 && suggestion_y < (int)(ui->keyboard.suggestion_count * 30)) {
            int index = suggestion_y / 30;
            strncpy(ui->keyboard.text, ui->keyboard.suggestions[index], 
                   MAX_INPUT_LENGTH - 1);
            keyboard_hide(ui);
            return;
        }
    }
    
    // Handle keyboard clicks
    // ... rest of click handling code ...
} 