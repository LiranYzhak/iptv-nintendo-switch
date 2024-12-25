#ifndef SEARCH_H
#define SEARCH_H

#include "ui.h"
#include "playlist.h"

// Search functions
void search_init(SearchContext* ctx);
void search_clear(SearchContext* ctx);
void search_update(UI* ui, const char* query);
bool search_matches_item(const SearchContext* ctx, const PlaylistItem* item);
void search_filter_results(UI* ui);
void search_sort_results(UI* ui);
void search_history_update_suggestions(UI* ui);

// Internal functions
static int compare_items_by_title(const void* a, const void* b);

#endif // SEARCH_H 