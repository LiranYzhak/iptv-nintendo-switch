#include "ui.h"
#include "drawing.h"
#include <SDL2/SDL_ttf.h>

typedef struct {
    UI* ui;
} UIManager;

UIManager* ui_manager_create(void) {
    UIManager* manager = (UIManager*)malloc(sizeof(UIManager));
    if (!manager) return NULL;
    
    manager->ui = ui_create();
    if (!manager->ui) {
        free(manager);
        return NULL;
    }
    
    return manager;
}

void ui_manager_free(UIManager* manager) {
    if (!manager) return;
    if (manager->ui) {
        ui_free(manager->ui);
    }
    free(manager);
}

void ui_manager_run(UIManager* manager) {
    if (manager && manager->ui) {
        ui_run(manager->ui);
    }
}

UI* ui_manager_get_ui(UIManager* manager) {
    return manager ? manager->ui : NULL;
} 