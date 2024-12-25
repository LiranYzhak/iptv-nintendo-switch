#include "nxgamepad.h"

static NXGamepad gamepad;

void nxgamepad_init(void) {
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&gamepad.id);
    gamepad.connected = false;
    gamepad.buttons = 0;
}

void nxgamepad_update(void) {
    PadState pad;
    padUpdate(&pad);
    
    gamepad.connected = padIsConnected(&pad);
    if (gamepad.connected) {
        gamepad.buttons = padGetButtons(&pad);
        padGetStickPos(&pad, 0, &gamepad.left_stick);
        padGetStickPos(&pad, 1, &gamepad.right_stick);
    }
}

bool nxgamepad_is_pressed(u64 button) {
    return gamepad.connected && (gamepad.buttons & button);
}

bool nxgamepad_is_released(u64 button) {
    return gamepad.connected && !(gamepad.buttons & button);
}

JoystickPosition nxgamepad_get_left_stick(void) {
    return gamepad.left_stick;
}

JoystickPosition nxgamepad_get_right_stick(void) {
    return gamepad.right_stick;
} 