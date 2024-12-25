#include "ui.h"
#include "drawing.h"
#include <SDL2/SDL_ttf.h>

class UIManager {
private:
    UI* ui;

public:
    UIManager() {
        ui = ui_create();
    }

    ~UIManager() {
        if (ui) {
            ui_free(ui);
        }
    }

    void run() {
        if (ui) {
            ui_run(ui);
        }
    }

    UI* get() {
        return ui;
    }
}; 