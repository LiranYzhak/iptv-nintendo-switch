#include "ui.h"
#include "ui_constants.h"
#include <string.h>
#include <stdlib.h>

SearchHistory* search_history_create(void) {
    SearchHistory* history = malloc(sizeof(SearchHistory));
    if (!history) return NULL;
    
    history->items = calloc(MAX_SEARCH_HISTORY, sizeof(char*));
    history->count = 0;
    history->capacity = MAX_SEARCH_HISTORY;
    history->input_buffer = calloc(MAX_INPUT_LENGTH, sizeof(char));
    history->suggestions = calloc(MAX_SUGGESTIONS, sizeof(char*));
    history->suggestion_count = 0;
    
    return history;
}

void search_history_free(SearchHistory* history) {
    if (!history) return;
    
    for (size_t i = 0; i < history->count; i++) {
        free(history->items[i]);
    }
    free(history->items);
    
    free(history->input_buffer);
    
    for (size_t i = 0; i < history->suggestion_count; i++) {
        free(history->suggestions[i]);
    }
    free(history->suggestions);
    
    free(history);
}

void search_history_add(SearchHistory* history, const char* query) {
    if (!history || !query || !query[0]) return;
    
    // Check if query already exists
    for (size_t i = 0; i < history->count; i++) {
        if (strcmp(history->items[i], query) == 0) {
            // Move to front
            char* temp = history->items[i];
            memmove(&history->items[1], &history->items[0], i * sizeof(char*));
            history->items[0] = temp;
            return;
        }
    }
    
    // Add new query
    char* new_query = strdup(query);
    if (!new_query) return;
    
    if (history->count >= history->capacity) {
        free(history->items[history->capacity - 1]);
        memmove(&history->items[1], &history->items[0], 
                (history->capacity - 1) * sizeof(char*));
    } else {
        memmove(&history->items[1], &history->items[0], 
                history->count * sizeof(char*));
        history->count++;
    }
    
    history->items[0] = new_query;
}

void search_history_update_suggestions(UI* ui) {
    SearchHistory* history = ui->keyboard.history;
    const char* input = ui->keyboard.text;
    
    // Clear old suggestions
    for (size_t i = 0; i < history->suggestion_count; i++) {
        free(history->suggestions[i]);
    }
    history->suggestion_count = 0;
    
    if (!input || !input[0]) return;
    
    // Find matching items from history
    size_t input_len = strlen(input);
    for (size_t i = 0; i < history->count; i++) {
        if (strncasecmp(history->items[i], input, input_len) == 0) {
            history->suggestions[history->suggestion_count++] = 
                strdup(history->items[i]);
            
            if (history->suggestion_count >= MAX_SUGGESTIONS) break;
        }
    }
    
    // Add matching items from playlist if space remains
    if (ui->playlist && history->suggestion_count < MAX_SUGGESTIONS) {
        for (size_t i = 0; i < ui->playlist->count; i++) {
            PlaylistItem* item = &ui->playlist->items[i];
            if (item->title && 
                strncasecmp(item->title, input, input_len) == 0) {
                history->suggestions[history->suggestion_count++] = 
                    strdup(item->title);
                
                if (history->suggestion_count >= MAX_SUGGESTIONS) break;
            }
        }
    }
} 