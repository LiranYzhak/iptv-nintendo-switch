#include "playlist.h"
#include "ui.h"
#include "network.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 16

Playlist* playlist_create(void) {
    Playlist* playlist = (Playlist*)malloc(sizeof(Playlist));
    if (!playlist) return NULL;

    playlist->items = (PlaylistItem*)malloc(sizeof(PlaylistItem) * INITIAL_CAPACITY);
    if (!playlist->items) {
        free(playlist);
        return NULL;
    }

    playlist->count = 0;
    playlist->capacity = INITIAL_CAPACITY;
    
    return playlist;
}

void playlist_free(Playlist* playlist) {
    if (!playlist) return;

    for (size_t i = 0; i < playlist->count; i++) {
        free(playlist->items[i].name);
        free(playlist->items[i].url);
        free(playlist->items[i].group);
        free(playlist->items[i].logo);
    }

    free(playlist->items);
    free(playlist);
}

bool playlist_add_item_details(Playlist* playlist, const char* name, const char* url,
                             const char* group, const char* logo) {
    if (!playlist || !name || !url) return false;
    
    PlaylistItem item = {0};
    item.name = strdup(name);
    item.url = strdup(url);
    if (group) item.group = strdup(group);
    if (logo) item.logo = strdup(logo);
    
    bool result = playlist_add_item(playlist, &item);
    
    // Clean up if add failed
    if (!result) {
        free(item.name);
        free(item.url);
        free(item.group);
        free(item.logo);
    }
    
    return result;
}

bool playlist_add_item(Playlist* playlist, const PlaylistItem* item) {
    if (!playlist || !item) return false;
    
    // Resize if needed
    if (playlist->count >= playlist->capacity) {
        size_t new_capacity = playlist->capacity == 0 ? 32 : playlist->capacity * 2;
        PlaylistItem* new_items = realloc(playlist->items, 
                                        new_capacity * sizeof(PlaylistItem));
        if (!new_items) return false;
        
        playlist->items = new_items;
        playlist->capacity = new_capacity;
    }
    
    // Copy item
    playlist->items[playlist->count] = *item;
    playlist->count++;
    
    return true;
}

bool playlist_load_from_file(Playlist* playlist, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[1024];
    char current_title[256] = "";
    char current_group[256] = "";
    char current_logo[1024] = "";

    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\r\n")] = 0;

        if (strncmp(line, "#EXTINF:", 8) == 0) {
            // Parse metadata line
            char* title = strchr(line + 8, ',');
            if (title) {
                *title = '\0';
                title++;
                strncpy(current_title, title, sizeof(current_title) - 1);
                
                // Parse group and logo if present
                char* group = strstr(title, "group-title=\"");
                if (group) {
                    group += 13;  // Skip "group-title=""
                    char* end = strchr(group, '"');
                    if (end) {
                        *end = '\0';
                        strncpy(current_group, group, sizeof(current_group) - 1);
                    }
                }
                
                char* logo = strstr(title, "tvg-logo=\"");
                if (logo) {
                    logo += 10;  // Skip "tvg-logo=""
                    char* end = strchr(logo, '"');
                    if (end) {
                        *end = '\0';
                        strncpy(current_logo, logo, sizeof(current_logo) - 1);
                    }
                }
            }
        } else if (line[0] != '#' && strlen(line) > 0) {
            // URL line - use playlist_add_item_details instead
            playlist_add_item_details(playlist, 
                                   strlen(current_title) > 0 ? current_title : line,
                                   line, 
                                   current_group,
                                   current_logo);
            current_title[0] = '\0';
            current_group[0] = '\0';
            current_logo[0] = '\0';
        }
    }

    fclose(file);
    return true;
}

bool playlist_load_from_url(Playlist* playlist, const char* url) {
    NetworkBuffer* buffer = network_download(url);
    if (!buffer) return false;

    // Save to temporary file
    char temp_filename[] = "temp_playlist.m3u";
    FILE* temp_file = fopen(temp_filename, "wb");
    if (!temp_file) {
        network_buffer_free(buffer);
        return false;
    }

    fwrite(buffer->data, 1, buffer->size, temp_file);
    fclose(temp_file);

    // Load from temporary file
    bool result = playlist_load_from_file(playlist, temp_filename);

    // Cleanup
    remove(temp_filename);
    network_buffer_free(buffer);

    return result;
}

bool playlist_save(const Playlist* playlist, const char* filename) {
    if (!playlist || !filename) return false;

    FILE* file = fopen(filename, "w");
    if (!file) return false;

    fprintf(file, "#EXTM3U\n");

    for (size_t i = 0; i < playlist->count; i++) {
        fprintf(file, "#EXTINF:-1");
        if (playlist->items[i].group) {
            fprintf(file, " group-title=\"%s\"", playlist->items[i].group);
        }
        if (playlist->items[i].logo) {
            fprintf(file, " tvg-logo=\"%s\"", playlist->items[i].logo);
        }
        fprintf(file, ",%s\n", playlist->items[i].name);
        fprintf(file, "%s\n", playlist->items[i].url);
    }

    fclose(file);
    return true;
}

void playlist_draw_item(UI* ui, PlaylistItem* item, int x, int y, bool selected) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {128, 128, 128, 255};
    SDL_Color color = selected ? white : gray;
    
    // Draw item...
} 