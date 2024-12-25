#ifndef STATE_H
#define STATE_H

#include "profile.h"
#include "ui.h"

// Application state
typedef struct {
    Profile* current_profile;
    UI* ui;
    bool initialized;
} AppState;

// State functions
AppState* state_create(void);
void state_free(AppState* state);
bool state_init(AppState* state);
void state_cleanup(AppState* state);
bool state_load_profile(AppState* state, const char* name);
bool state_save_profile(AppState* state);

#endif // STATE_H 