#ifndef CATEGORIES_H
#define CATEGORIES_H

#include <stdbool.h>
#include <stddef.h>
#include "ui_constants.h"

// Forward declarations
struct UI;
typedef struct UI UI;

// Category filter structure
typedef struct CategoryFilter {
    char** blocked_categories;
    size_t blocked_count;
    bool filter_active;
} CategoryFilter;

// Category filter functions
CategoryFilter* category_filter_create(void);
void category_filter_free(CategoryFilter* filter);
bool category_filter_add(CategoryFilter* filter, const char* category);
bool category_filter_remove(CategoryFilter* filter, const char* category);
bool category_filter_is_blocked(const CategoryFilter* filter, const char* category);
void category_filter_clear(CategoryFilter* filter);
void category_filter_enable(CategoryFilter* filter);
void category_filter_disable(CategoryFilter* filter);
bool category_filter_is_enabled(const CategoryFilter* filter);

// Category management functions
void ui_update_categories(UI* ui);
void ui_draw_categories(UI* ui);

#endif // CATEGORIES_H 