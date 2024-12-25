#include "search.h"
#include "ui_constants.h"
#include <string.h>
#include <ctype.h>

void search_init(SearchContext* ctx) {
    memset(ctx->query, 0, sizeof(ctx->query));
    ctx->show_favorites_only = false;
    ctx->results = NULL;
    ctx->result_count = 0;
    ctx->selected_category = NULL;
}

void search_clear(SearchContext* ctx) {
    free(ctx->results);
    ctx->results = NULL;
    ctx->result_count = 0;
}

bool search_matches_item(const SearchContext* ctx, const PlaylistItem* item) {
    if (!item || !item->title) return false;
    if (ctx->show_favorites_only && !item->favorite) return false;
    
    // If category is selected, check if item belongs to it
    if (ctx->selected_category && 
        (!item->group || strcmp(item->group, ctx->selected_category) != 0)) {
        return false;
    }
    
    // Empty query matches everything
    if (!ctx->query[0]) return true;
    
    // Case insensitive search in title
    char* lower_title = strdup(item->title);
    char* lower_query = strdup(ctx->query);
    
    for (char* p = lower_title; *p; p++) *p = tolower(*p);
    for (char* p = lower_query; *p; p++) *p = tolower(*p);
    
    bool matches = strstr(lower_title, lower_query) != NULL;
    
    free(lower_title);
    free(lower_query);
    
    return matches;
}

void search_update(UI* ui, const char* query) {
    strncpy(ui->search.query, query, sizeof(ui->search.query) - 1);
    search_filter_results(ui);
}

void search_filter_results(UI* ui) {
    search_clear(&ui->search);
    
    if (!ui->playlist) return;
    
    // Count matching items first
    size_t count = 0;
    for (size_t i = 0; i < ui->playlist->count; i++) {
        if (search_matches_item(&ui->search, &ui->playlist->items[i])) {
            count++;
        }
    }
    
    if (count == 0) return;
    
    // Allocate results array
    ui->search.results = malloc(count * sizeof(PlaylistItem*));
    
    // Fill results array
    size_t j = 0;
    for (size_t i = 0; i < ui->playlist->count; i++) {
        if (search_matches_item(&ui->search, &ui->playlist->items[i])) {
            ui->search.results[j++] = &ui->playlist->items[i];
        }
    }
    
    ui->search.result_count = count;
    search_sort_results(ui);
}

void search_sort_results(UI* ui) {
    // Sort by title (case insensitive)
    if (ui->search.result_count > 1) {
        qsort(ui->search.results, ui->search.result_count, 
              sizeof(PlaylistItem*), compare_items_by_title);
    }
}

int compare_items_by_title(const void* a, const void* b) {
    const PlaylistItem* item1 = *(const PlaylistItem**)a;
    const PlaylistItem* item2 = *(const PlaylistItem**)b;
    
    if (!item1->title) return 1;
    if (!item2->title) return -1;
    
    return strcasecmp(item1->title, item2->title);
}

void search_history_update_suggestions(UI* ui) {
    // הוסף כאן את הלוגיקה לעדכון הצעות החיפוש
} 