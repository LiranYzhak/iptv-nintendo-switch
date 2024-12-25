#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "player.h"
#include "playlist.h"
#include "epg.h"
#include "ui_constants.h"
#include "category_blocker.h"
#include "categories.h"

// Forward declarations
struct UI;
typedef struct UI UI;

// UI States
typedef enum {
    UI_STATE_PLAYLIST,
    UI_STATE_PLAYING,
    UI_STATE_EPG,
    UI_STATE_CATEGORIES,
    UI_STATE_SEARCH
} UIState;

// Animation types
typedef enum {
    ANIM_FADE,
    ANIM_ZOOM,
    ANIM_SLIDE_LEFT,
    ANIM_SLIDE_RIGHT
} AnimationType;

// Player settings
typedef struct {
    int volume;
    bool auto_play;
    bool repeat;
    bool shuffle;
} PlayerSettings;

// Search context
typedef struct {
    char query[256];
    bool show_favorites_only;
    PlaylistItem** results;
    size_t result_count;
    char* selected_category;
} SearchContext;

// Animation transition
typedef struct {
    AnimationType type;
    float duration;
    float progress;
    Uint32 start_time;
    SDL_Texture* prev_texture;
    SDL_Texture* prev_screen;
    SDL_Texture* next_screen;
} Transition;

// Thumbnail structure
typedef struct {
    SDL_Texture* texture;
    char* url;
    time_t last_access;
    bool loading;
} Thumbnail;

// Search history
typedef struct {
    char** items;
    size_t count;
    size_t capacity;
    char* input_buffer;
    char** suggestions;
    size_t suggestion_count;
} SearchHistory;

// Keyboard context
typedef struct {
    char* text;
    bool active;
    char** suggestions;
    size_t suggestion_count;
    int selected_suggestion;
    SearchHistory* history;
} KeyboardContext;

// UI Context
typedef struct UI {
    // Window and rendering
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* font_large;
    
    // State
    UIState state;
    bool quit;
    
    // Player
    Player* player;
    PlayerSettings player_settings;
    
    // Playlist
    Playlist* playlist;
    int selected_item;
    int scroll_offset;
    
    // Categories
    CategoryFilter* category_filter;
    int selected_category;
    int category_count;
    char** categories;
    
    // Search
    SearchContext search;
    PlaylistItem** search_results;
    size_t search_result_count;
    KeyboardContext keyboard;
    
    // Touch input
    bool touch_active;
    int touch_x;
    int touch_y;
    
    // Status message
    char* status_message;
    Uint32 status_timeout;
    
    // Animation
    bool show_info;
    float transition_progress;
    AnimationType current_animation;
    bool animating;
    Transition transition;
    
    // EPG
    EPGData* epg;
    
    // Thumbnails
    Thumbnail* thumbnails;
    size_t thumbnail_count;
    
    // Grid view
    int grid_rows;
    int grid_columns;
    int grid_scroll_offset;
    
    // Category blocker
    CategoryBlockerContext blocker;
} UI;

// UI Functions
UI* ui_create(void);
void ui_free(UI* ui);
void ui_run(UI* ui);
void ui_draw(UI* ui);
void ui_update(UI* ui);
void ui_handle_input(UI* ui);
void ui_set_state(UI* ui, UIState state);
void ui_show_message(UI* ui, const char* message, Uint32 timeout);
void ui_draw_status(UI* ui);
void ui_draw_playlist(UI* ui);
void ui_draw_player(UI* ui);
void ui_draw_epg(UI* ui);
void ui_perform_search(UI* ui);

// Search history functions
SearchHistory* search_history_create(void);
void search_history_free(SearchHistory* history);
void search_history_add(SearchHistory* history, const char* query);
void search_history_clear(SearchHistory* history);

#endif // UI_H 