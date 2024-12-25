#ifndef PLAYER_H
#define PLAYER_H

#include <switch.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "playlist.h"

typedef enum {
    PLAYER_STATE_STOPPED,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_PAUSED,
    PLAYER_STATE_ERROR
} PlayerState;

typedef enum {
    PLAYER_ERROR_NONE = 0,
    PLAYER_ERROR_MEMORY,
    PLAYER_ERROR_OPEN_FILE,
    PLAYER_ERROR_STREAM_INFO,
    PLAYER_ERROR_NO_STREAMS,
    PLAYER_ERROR_VIDEO_CODEC,
    PLAYER_ERROR_AUDIO_CODEC,
    PLAYER_ERROR_SDL,
    PLAYER_ERROR_THREAD
} PlayerError;

struct Player {
    PlayerState state;
    PlayerError last_error;
    PlaylistItem* current_item;
    double current_time;
    void* ffmpeg_ctx;
    void* sdl_ctx;
    Mutex state_mutex;
    Thread playback_thread;
};

typedef struct Player Player;

// Player functions
Player* player_create(void);
void player_free(Player* player);
bool player_load(Player* player, const PlaylistItem* item);
bool player_play(Player* player);
bool player_pause(Player* player);
bool player_stop(Player* player);
double player_get_duration(const Player* player);
double player_get_position(const Player* player);
bool player_seek(Player* player, double position);
SDL_Texture* player_get_video_texture(const Player* player);

// Add error handling functions
const char* player_get_error_string(PlayerError error);
PlayerError player_get_last_error(const Player* player);

#endif // PLAYER_H 