#include <switch.h>
#include "ui.h"

int main(int argc, char* argv[])
{
    // Initialize console
    consoleInit(NULL);
    
    // Create UI
    UI* ui = ui_create();
    if (!ui) {
        consoleExit(NULL);
        return 1;
    }
    
    // Run main loop
    ui_run(ui);
    
    // Cleanup
    ui_free(ui);
    consoleExit(NULL);
    
    return 0;
} 