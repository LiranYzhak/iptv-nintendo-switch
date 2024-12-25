#include "categories.h"
#include <stdlib.h>
#include <string.h>

CategoryFilter* category_filter_create(void) {
    CategoryFilter* filter = malloc(sizeof(CategoryFilter));
    if (!filter) return NULL;
    
    filter->blocked_categories = calloc(MAX_BLOCKED_CATEGORIES, sizeof(char*));
    if (!filter->blocked_categories) {
        free(filter);
        return NULL;
    }
    
    filter->blocked_count = 0;
    filter->filter_active = false;
    
    return filter;
}

void category_filter_free(CategoryFilter* filter) {
    if (!filter) return;
    
    for (size_t i = 0; i < filter->blocked_count; i++) {
        free(filter->blocked_categories[i]);
    }
    free(filter->blocked_categories);
    free(filter);
}

bool category_filter_add(CategoryFilter* filter, const char* category) {
    if (!filter || !category || filter->blocked_count >= MAX_BLOCKED_CATEGORIES) {
        return false;
    }
    
    // Check if category is already blocked
    for (size_t i = 0; i < filter->blocked_count; i++) {
        if (strcmp(filter->blocked_categories[i], category) == 0) {
            return true;  // Already blocked
        }
    }
    
    // Add new category
    char* new_category = strdup(category);
    if (!new_category) return false;
    
    filter->blocked_categories[filter->blocked_count++] = new_category;
    return true;
}

bool category_filter_remove(CategoryFilter* filter, const char* category) {
    if (!filter || !category) return false;
    
    for (size_t i = 0; i < filter->blocked_count; i++) {
        if (strcmp(filter->blocked_categories[i], category) == 0) {
            free(filter->blocked_categories[i]);
            
            // Move remaining categories up
            for (size_t j = i; j < filter->blocked_count - 1; j++) {
                filter->blocked_categories[j] = filter->blocked_categories[j + 1];
            }
            
            filter->blocked_count--;
            return true;
        }
    }
    
    return false;
}

bool category_filter_is_blocked(const CategoryFilter* filter, const char* category) {
    if (!filter || !filter->filter_active || !category) return false;
    
    for (size_t i = 0; i < filter->blocked_count; i++) {
        if (strcmp(filter->blocked_categories[i], category) == 0) {
            return true;
        }
    }
    
    return false;
}

void category_filter_clear(CategoryFilter* filter) {
    if (!filter) return;
    
    for (size_t i = 0; i < filter->blocked_count; i++) {
        free(filter->blocked_categories[i]);
    }
    filter->blocked_count = 0;
}

void category_filter_enable(CategoryFilter* filter) {
    if (filter) filter->filter_active = true;
}

void category_filter_disable(CategoryFilter* filter) {
    if (filter) filter->filter_active = false;
}

bool category_filter_is_enabled(const CategoryFilter* filter) {
    return filter ? filter->filter_active : false;
} 