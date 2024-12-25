#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <stdbool.h>
#include <time.h>
#include "ui_constants.h"

// Playlist item structure
typedef struct {
    char* title;
    char* url;
    char* name;
    char* logo;
    char* group;
    char* language;
    char* tvg_id;
    char* tvg_name;
    char* tvg_logo;
    int duration;
    bool favorite;
    time_t last_played;
} PlaylistItem;

// Playlist structure
typedef struct {
    PlaylistItem* items;
    size_t count;
    size_t capacity;
    char* filename;
    time_t last_modified;
} Playlist;

// Playlist functions
Playlist* playlist_create(void);
void playlist_free(Playlist* playlist);
bool playlist_load_m3u(Playlist* playlist, const char* filename);
bool playlist_save_m3u(const Playlist* playlist, const char* filename);
bool playlist_add_item(Playlist* playlist, const PlaylistItem* item);
void playlist_remove_item(Playlist* playlist, size_t index);
void playlist_clear(Playlist* playlist);
void playlist_sort(Playlist* playlist);

// Item functions
PlaylistItem* playlist_item_create(void);
void playlist_item_free(PlaylistItem* item);
PlaylistItem* playlist_item_copy(const PlaylistItem* item);

// Add both function declarations
bool playlist_add_item(Playlist* playlist, const PlaylistItem* item);
bool playlist_add_item_details(Playlist* playlist, const char* name, const char* url,
                             const char* group, const char* logo);

#endif // PLAYLIST_H 