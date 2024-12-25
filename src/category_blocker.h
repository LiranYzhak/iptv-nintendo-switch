#ifndef CATEGORY_BLOCKER_H
#define CATEGORY_BLOCKER_H

#include <stdbool.h>
#include <SDL2/SDL.h>

// Forward declaration
struct UI;
typedef struct UI UI;

// Menu options
typedef enum {
    MENU_BLOCK,
    MENU_UNBLOCK,
    MENU_CLEAR,
    MENU_BACK,
    MENU_OPTIONS_COUNT
} CategoryBlockerMenu;

// Category blocker context
typedef struct {
    int menu_selection;
    bool active;
} CategoryBlockerContext;

// Functions
void category_blocker_init(UI* ui);
void category_blocker_show(UI* ui);
void category_blocker_hide(UI* ui);
void category_blocker_handle_input(UI* ui, SDL_Event* event);
void category_blocker_draw(UI* ui);

#endif // CATEGORY_BLOCKER_H 