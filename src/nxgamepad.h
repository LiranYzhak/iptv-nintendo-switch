#ifndef NXGAMEPAD_H
#define NXGAMEPAD_H

#include <switch.h>

// Gamepad state
typedef struct {
    HidControllerID id;
    bool connected;
    u64 buttons;
    JoystickPosition left_stick;
    JoystickPosition right_stick;
} NXGamepad;

// Functions
void nxgamepad_init(void);
void nxgamepad_update(void);
bool nxgamepad_is_pressed(u64 button);
bool nxgamepad_is_released(u64 button);
JoystickPosition nxgamepad_get_left_stick(void);
JoystickPosition nxgamepad_get_right_stick(void);

#endif // NXGAMEPAD_H 