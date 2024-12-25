#ifndef PROFILE_H
#define PROFILE_H

#include <stdbool.h>
#include "ui_constants.h"

// Profile settings
typedef struct {
    char name[MAX_PROFILE_NAME];
    bool parental_controls_enabled;
    int age_rating;
    char pin[MAX_PIN_LENGTH];
} Profile;

// Profile functions
Profile* profile_create(const char* name);
void profile_free(Profile* profile);
bool profile_save(const Profile* profile, const char* filename);
bool profile_load(Profile* profile, const char* filename);
bool profile_verify_pin(const Profile* profile, const char* pin);
void profile_set_pin(Profile* profile, const char* pin);

#endif // PROFILE_H 