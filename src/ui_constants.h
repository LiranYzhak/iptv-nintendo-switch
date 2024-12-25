#ifndef UI_CONSTANTS_H
#define UI_CONSTANTS_H

// Window dimensions
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// UI element sizes
#define ITEM_HEIGHT 40
#define SCROLL_MARGIN 20
#define STATUS_DURATION 3000  // 3 seconds

// Helper macros
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Thumbnail settings
#define MAX_THUMBNAILS 100
#define THUMBNAIL_WIDTH 160
#define THUMBNAIL_HEIGHT 90

// Search history settings
#define MAX_SEARCH_HISTORY 20
#define MAX_SUGGESTIONS 10
#define MAX_QUERY_LENGTH 256
#define MAX_INPUT_LENGTH 256

// Keyboard settings
#define KEYBOARD_ROWS 4
#define KEYBOARD_COLS 10
#define MAX_INPUT_LENGTH 256

// Grid view settings
#define DEFAULT_GRID_ROWS 3
#define DEFAULT_GRID_COLS 4
#define MIN_GRID_COLS 2
#define MAX_GRID_COLS 6

// Category filter settings
#define MAX_BLOCKED_CATEGORIES 50
#define MAX_CATEGORY_LENGTH 64

// Profile settings
#define MAX_PROFILE_NAME 32
#define MAX_PIN_LENGTH 8

#endif // UI_CONSTANTS_H 