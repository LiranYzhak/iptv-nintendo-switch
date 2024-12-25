#ifndef CATEGORY_FILTER_H
#define CATEGORY_FILTER_H

#include <stdbool.h>
#include "playlist.h"

typedef struct {
    char** categories;
    size_t count;
    bool* selected;
} CategoryFilter;

CategoryFilter* category_filter_create(void);
void category_filter_free(CategoryFilter* filter);
void category_filter_update(CategoryFilter* filter, Playlist* playlist);
bool category_filter_matches(CategoryFilter* filter, PlaylistItem* item);

#endif // CATEGORY_FILTER_H 