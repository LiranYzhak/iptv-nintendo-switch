#include "player.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <string.h>

// Forward declarations
static void playback_thread_func(void* arg);

struct FFmpegContext {
    AVFormatContext* format_ctx;
    AVCodecContext* video_codec_ctx;
    AVCodecContext* audio_codec_ctx;
    int video_stream_idx;
    int audio_stream_idx;
    struct SwsContext* sws_ctx;
    SwrContext* swr_ctx;
    AVFrame* frame;
    AVPacket* packet;
};

struct SDLContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_AudioDeviceID audio_dev;
    SDL_AudioSpec audio_spec;
};

static const char* error_strings[] = {
    "No error",
    "Memory allocation failed",
    "Could not open media file",
    "Could not find stream information",
    "No suitable streams found",
    "Could not initialize video codec",
    "Could not initialize audio codec",
    "SDL initialization failed",
    "Thread creation failed"
};

const char* player_get_error_string(PlayerError error) {
    if (error < 0 || error >= sizeof(error_strings) / sizeof(error_strings[0])) {
        return "Unknown error";
    }
    return error_strings[error];
}

PlayerError player_get_last_error(const Player* player) {
    return player ? player->last_error : PLAYER_ERROR_NONE;
}

Player* player_create(void) {
    Player* player = (Player*)malloc(sizeof(Player));
    if (!player) {
        return NULL;
    }

    player->state = PLAYER_STATE_STOPPED;
    player->current_item = NULL;
    player->current_time = 0.0;
    player->last_error = PLAYER_ERROR_NONE;
    
    // Initialize mutex for thread safety
    mutexInit(&player->state_mutex);
    
    // Initialize FFmpeg
    player->ffmpeg_ctx = malloc(sizeof(struct FFmpegContext));
    if (!player->ffmpeg_ctx) {
        player->last_error = PLAYER_ERROR_MEMORY;
        mutexUnlock(&player->state_mutex);
        free(player);
        return NULL;
    }
    memset(player->ffmpeg_ctx, 0, sizeof(struct FFmpegContext));

    // Initialize SDL
    player->sdl_ctx = malloc(sizeof(struct SDLContext));
    if (!player->sdl_ctx) {
        player->last_error = PLAYER_ERROR_MEMORY;
        mutexUnlock(&player->state_mutex);
        free(player->ffmpeg_ctx);
        free(player);
        return NULL;
    }
    memset(player->sdl_ctx, 0, sizeof(struct SDLContext));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        player->last_error = PLAYER_ERROR_SDL;
        mutexUnlock(&player->state_mutex);
        free(player->sdl_ctx);
        free(player->ffmpeg_ctx);
        free(player);
        return NULL;
    }

    return player;
}

void player_free(Player* player) {
    if (!player) return;

    // Stop playback if running
    if (player->state == PLAYER_STATE_PLAYING || 
        player->state == PLAYER_STATE_PAUSED) {
        player_stop(player);
    }

    struct FFmpegContext* fctx = (struct FFmpegContext*)player->ffmpeg_ctx;
    struct SDLContext* sctx = (struct SDLContext*)player->sdl_ctx;

    // Cleanup FFmpeg
    if (fctx) {
        // Free codec contexts safely
        if (fctx->video_codec_ctx) {
            avcodec_flush_buffers(fctx->video_codec_ctx);
            avcodec_free_context(&fctx->video_codec_ctx);
        }
        if (fctx->audio_codec_ctx) {
            avcodec_flush_buffers(fctx->audio_codec_ctx);
            avcodec_free_context(&fctx->audio_codec_ctx);
        }

        // Close input and free contexts
        if (fctx->format_ctx) {
            avformat_close_input(&fctx->format_ctx);
        }
        if (fctx->sws_ctx) {
            sws_freeContext(fctx->sws_ctx);
        }
        if (fctx->swr_ctx) {
            swr_free(&fctx->swr_ctx);
        }

        // Free frame and packet
        if (fctx->frame) {
            av_frame_free(&fctx->frame);
        }
        if (fctx->packet) {
            av_packet_free(&fctx->packet);
        }

        free(fctx);
    }

    // Cleanup SDL
    if (sctx) {
        // Stop and close audio
        if (sctx->audio_dev) {
            SDL_PauseAudioDevice(sctx->audio_dev, 1);
            SDL_ClearQueuedAudio(sctx->audio_dev);
            SDL_CloseAudioDevice(sctx->audio_dev);
        }

        // Destroy video resources
        if (sctx->texture) {
            SDL_DestroyTexture(sctx->texture);
        }
        if (sctx->renderer) {
            SDL_DestroyRenderer(sctx->renderer);
        }
        if (sctx->window) {
            SDL_DestroyWindow(sctx->window);
        }

        free(sctx);
    }

    // Free mutex
    mutexUnlock(&player->state_mutex);

    SDL_Quit();
    free(player);
}

bool player_load(Player* player, const PlaylistItem* item) {
    if (!player || !item) {
        if (player) player->last_error = PLAYER_ERROR_MEMORY;
        return false;
    }

    // Stop current playback if any
    if (player->state != PLAYER_STATE_STOPPED) {
        player_stop(player);
    }

    player->last_error = PLAYER_ERROR_NONE;
    player->current_item = (PlaylistItem*)item;
    struct FFmpegContext* fctx = (struct FFmpegContext*)player->ffmpeg_ctx;
    struct SDLContext* sctx = (struct SDLContext*)player->sdl_ctx;

    // Clean up previous resources
    if (fctx->video_codec_ctx) {
        avcodec_free_context(&fctx->video_codec_ctx);
    }
    if (fctx->audio_codec_ctx) {
        avcodec_free_context(&fctx->audio_codec_ctx);
    }
    if (fctx->format_ctx) {
        avformat_close_input(&fctx->format_ctx);
    }

    // Open file/stream
    if (avformat_open_input(&fctx->format_ctx, item->url, NULL, NULL) < 0) {
        player->last_error = PLAYER_ERROR_OPEN_FILE;
        return false;
    }

    if (avformat_find_stream_info(fctx->format_ctx, NULL) < 0) {
        player->last_error = PLAYER_ERROR_STREAM_INFO;
        avformat_close_input(&fctx->format_ctx);
        return false;
    }

    // Find video and audio streams
    fctx->video_stream_idx = av_find_best_stream(fctx->format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    fctx->audio_stream_idx = av_find_best_stream(fctx->format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    if (fctx->video_stream_idx < 0 && fctx->audio_stream_idx < 0) {
        player->last_error = PLAYER_ERROR_NO_STREAMS;
        avformat_close_input(&fctx->format_ctx);
        return false;
    }

    // Initialize codecs
    if (fctx->video_stream_idx >= 0) {
        AVStream* video_stream = fctx->format_ctx->streams[fctx->video_stream_idx];
        const AVCodec* video_codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
        
        fctx->video_codec_ctx = avcodec_alloc_context3(video_codec);
        avcodec_parameters_to_context(fctx->video_codec_ctx, video_stream->codecpar);
        
        if (avcodec_open2(fctx->video_codec_ctx, video_codec, NULL) < 0) {
            player->last_error = PLAYER_ERROR_VIDEO_CODEC;
            avcodec_free_context(&fctx->video_codec_ctx);
            avformat_close_input(&fctx->format_ctx);
            return false;
        }
        
        // Create SDL window and renderer for video output
        sctx->window = SDL_CreateWindow("Switch M3U Player",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      fctx->video_codec_ctx->width,
                                      fctx->video_codec_ctx->height,
                                      SDL_WINDOW_SHOWN);
        
        if (!sctx->window) {
            player->last_error = PLAYER_ERROR_SDL;
            avcodec_free_context(&fctx->video_codec_ctx);
            avformat_close_input(&fctx->format_ctx);
            return false;
        }
        
        sctx->renderer = SDL_CreateRenderer(sctx->window, -1, SDL_RENDERER_ACCELERATED);
        
        if (!sctx->renderer) {
            player->last_error = PLAYER_ERROR_SDL;
            SDL_DestroyWindow(sctx->window);
            avcodec_free_context(&fctx->video_codec_ctx);
            avformat_close_input(&fctx->format_ctx);
            return false;
        }

        sctx->texture = SDL_CreateTexture(sctx->renderer,
                                        SDL_PIXELFORMAT_YV12,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        fctx->video_codec_ctx->width,
                                        fctx->video_codec_ctx->height);
        
        if (!sctx->texture) {
            player->last_error = PLAYER_ERROR_SDL;
            SDL_DestroyRenderer(sctx->renderer);
            SDL_DestroyWindow(sctx->window);
            avcodec_free_context(&fctx->video_codec_ctx);
            avformat_close_input(&fctx->format_ctx);
            return false;
        }
        
        // Initialize scaling context
        fctx->sws_ctx = sws_getContext(fctx->video_codec_ctx->width,
                                      fctx->video_codec_ctx->height,
                                      fctx->video_codec_ctx->pix_fmt,
                                      fctx->video_codec_ctx->width,
                                      fctx->video_codec_ctx->height,
                                      AV_PIX_FMT_YUV420P,
                                      SWS_BILINEAR,
                                      NULL, NULL, NULL);
    }

    if (fctx->audio_stream_idx >= 0) {
        AVStream* audio_stream = fctx->format_ctx->streams[fctx->audio_stream_idx];
        const AVCodec* audio_codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
        
        fctx->audio_codec_ctx = avcodec_alloc_context3(audio_codec);
        avcodec_parameters_to_context(fctx->audio_codec_ctx, audio_stream->codecpar);
        
        if (avcodec_open2(fctx->audio_codec_ctx, audio_codec, NULL) < 0) {
            player->last_error = PLAYER_ERROR_AUDIO_CODEC;
            avcodec_free_context(&fctx->audio_codec_ctx);
            if (fctx->video_codec_ctx) avcodec_free_context(&fctx->video_codec_ctx);
            avformat_close_input(&fctx->format_ctx);
            return false;
        }
        
        // Set up SDL audio
        SDL_AudioSpec wanted_spec;
        wanted_spec.freq = fctx->audio_codec_ctx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
#if LIBAVCODEC_VERSION_MAJOR >= 59
        wanted_spec.channels = fctx->audio_codec_ctx->ch_layout.nb_channels;
#else
        wanted_spec.channels = fctx->audio_codec_ctx->channels;
#endif
        wanted_spec.silence = 0;
        wanted_spec.samples = 4096;
        wanted_spec.callback = NULL;  // Using queue audio mode
        wanted_spec.userdata = NULL;
        
        sctx->audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &sctx->audio_spec, 0);
        
        if (!sctx->audio_dev) {
            player->last_error = PLAYER_ERROR_SDL;
            // Clean up all previously allocated resources
            if (sctx->texture) SDL_DestroyTexture(sctx->texture);
            if (sctx->renderer) SDL_DestroyRenderer(sctx->renderer);
            if (sctx->window) SDL_DestroyWindow(sctx->window);
            if (fctx->video_codec_ctx) avcodec_free_context(&fctx->video_codec_ctx);
            if (fctx->audio_codec_ctx) avcodec_free_context(&fctx->audio_codec_ctx);
            avformat_close_input(&fctx->format_ctx);
            return false;
        }
        
        // Initialize resampler
        fctx->swr_ctx = swr_alloc();
        if (!fctx->swr_ctx) {
            player->last_error = PLAYER_ERROR_MEMORY;
            return false;
        }
        
        av_opt_set_chlayout(fctx->swr_ctx, "in_chlayout", &fctx->audio_codec_ctx->ch_layout, 0);
        av_opt_set_int(fctx->swr_ctx, "in_sample_rate", fctx->audio_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(fctx->swr_ctx, "in_sample_fmt", fctx->audio_codec_ctx->sample_fmt, 0);
        
        av_opt_set_chlayout(fctx->swr_ctx, "out_chlayout", &fctx->audio_codec_ctx->ch_layout, 0);
        av_opt_set_int(fctx->swr_ctx, "out_sample_rate", sctx->audio_spec.freq, 0);
        av_opt_set_sample_fmt(fctx->swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        
        if (swr_init(fctx->swr_ctx) < 0) {
            player->last_error = PLAYER_ERROR_AUDIO_CODEC;
            return false;
        }
    }

    // Allocate frame and packet
    fctx->frame = av_frame_alloc();
    fctx->packet = av_packet_alloc();
    
    player->state = PLAYER_STATE_STOPPED;
    player->current_time = 0.0;

    return true;
}

bool player_play(Player* player) {
    if (!player || player->state == PLAYER_STATE_PLAYING) {
        if (player) player->last_error = PLAYER_ERROR_NONE;
        return false;
    }
    
    mutexLock(&player->state_mutex);
    player->state = PLAYER_STATE_PLAYING;
    
    // Start playback thread
    Result rc = threadCreate(&player->playback_thread, 
                           playback_thread_func, 
                           player, 
                           NULL, 
                           0x4000, // Stack size
                           0x2B,   // Priority
                           -2);    // Core -2 = don't care
    if (R_FAILED(rc)) {
        player->last_error = PLAYER_ERROR_THREAD;
        player->state = PLAYER_STATE_ERROR;
        mutexUnlock(&player->state_mutex);
        return false;
    }
    
    rc = threadStart(&player->playback_thread);
    if (R_FAILED(rc)) {
        player->last_error = PLAYER_ERROR_THREAD;
        player->state = PLAYER_STATE_ERROR;
        threadClose(&player->playback_thread);
        mutexUnlock(&player->state_mutex);
        return false;
    }
    
    mutexUnlock(&player->state_mutex);
    return true;
}

bool player_pause(Player* player) {
    if (!player || player->state != PLAYER_STATE_PLAYING) {
        if (player) player->last_error = PLAYER_ERROR_NONE;
        return false;
    }
    
    mutexLock(&player->state_mutex);
    player->state = PLAYER_STATE_PAUSED;
    SDL_PauseAudioDevice(((struct SDLContext*)player->sdl_ctx)->audio_dev, 1);
    mutexUnlock(&player->state_mutex);
    
    return true;
}

bool player_stop(Player* player) {
    if (!player) {
        if (player) player->last_error = PLAYER_ERROR_NONE;
        return false;
    }
    
    mutexLock(&player->state_mutex);
    player->state = PLAYER_STATE_STOPPED;
    player->current_time = 0.0;
    
    // Stop audio playback
    SDL_PauseAudioDevice(((struct SDLContext*)player->sdl_ctx)->audio_dev, 1);
    SDL_ClearQueuedAudio(((struct SDLContext*)player->sdl_ctx)->audio_dev);
    
    // Wait for playback thread to finish
    Result rc = threadWaitForExit(&player->playback_thread);
    if (R_FAILED(rc)) {
        player->last_error = PLAYER_ERROR_THREAD;
        mutexUnlock(&player->state_mutex);
        return false;
    }
    
    threadClose(&player->playback_thread);
    mutexUnlock(&player->state_mutex);
    
    return true;
}

double player_get_duration(const Player* player) {
    if (!player || !player->ffmpeg_ctx) return 0.0;
    struct FFmpegContext* fctx = (struct FFmpegContext*)player->ffmpeg_ctx;
    if (!fctx->format_ctx) return 0.0;
    return (double)fctx->format_ctx->duration / AV_TIME_BASE;
}

double player_get_position(const Player* player) {
    if (!player) return 0.0;
    return player->current_time;
}

bool player_seek(Player* player, double position) {
    if (!player || !player->ffmpeg_ctx) {
        if (player) player->last_error = PLAYER_ERROR_NONE;
        return false;
    }
    
    mutexLock(&player->state_mutex);
    struct FFmpegContext* fctx = (struct FFmpegContext*)player->ffmpeg_ctx;
    
    int64_t timestamp = position * AV_TIME_BASE;
    if (av_seek_frame(fctx->format_ctx, -1, timestamp, AVSEEK_FLAG_ANY) < 0) {
        player->last_error = PLAYER_ERROR_STREAM_INFO;
        mutexUnlock(&player->state_mutex);
        return false;
    }
    
    player->current_time = position;
    mutexUnlock(&player->state_mutex);
    
    return true;
} 

static void playback_thread_func(void* arg) {
    Player* player = (Player*)arg;
    struct FFmpegContext* fctx = (struct FFmpegContext*)player->ffmpeg_ctx;
    struct SDLContext* sctx = (struct SDLContext*)player->sdl_ctx;
    
    while (player->state == PLAYER_STATE_PLAYING) {
        mutexLock(&player->state_mutex);
        
        // Read packet
        if (av_read_frame(fctx->format_ctx, fctx->packet) >= 0) {
            // Handle errors in video/audio processing
            int ret;
            if (fctx->packet->stream_index == fctx->video_stream_idx) {
                ret = avcodec_send_packet(fctx->video_codec_ctx, fctx->packet);
                if (ret < 0) {
                    player->last_error = PLAYER_ERROR_VIDEO_CODEC;
                    player->state = PLAYER_STATE_ERROR;
                    mutexUnlock(&player->state_mutex);
                    break;
                }
                // Handle video packet
                if (avcodec_receive_frame(fctx->video_codec_ctx, fctx->frame) == 0) {
                    // Convert frame to SDL texture
                    SDL_UpdateTexture(sctx->texture, NULL, 
                                    fctx->frame->data[0], 
                                    fctx->frame->linesize[0]);
                    
                    SDL_RenderClear(sctx->renderer);
                    SDL_RenderCopy(sctx->renderer, sctx->texture, NULL, NULL);
                    SDL_RenderPresent(sctx->renderer);
                    
                    // Update playback time
                    player->current_time = fctx->frame->pts * 
                        av_q2d(fctx->format_ctx->streams[fctx->video_stream_idx]->time_base);
                }
            }
            // Handle audio packet
            else if (fctx->packet->stream_index == fctx->audio_stream_idx) {
                ret = avcodec_send_packet(fctx->audio_codec_ctx, fctx->packet);
                if (ret < 0) {
                    player->last_error = PLAYER_ERROR_AUDIO_CODEC;
                    player->state = PLAYER_STATE_ERROR;
                    mutexUnlock(&player->state_mutex);
                    break;
                }
                while (avcodec_receive_frame(fctx->audio_codec_ctx, fctx->frame) == 0) {
                    // Convert audio format if needed
                    uint8_t* audio_buf;
                    int audio_buf_size;
                    
                    swr_convert(fctx->swr_ctx,
                              &audio_buf,
                              fctx->frame->nb_samples,
                              (const uint8_t**)fctx->frame->data,
                              fctx->frame->nb_samples);
                    
                    audio_buf_size = av_samples_get_buffer_size(NULL,
                                                                fctx->audio_codec_ctx->ch_layout.nb_channels,
                                                                fctx->frame->nb_samples,
                                                                AV_SAMPLE_FMT_S16,
                                                                1);
                    
                    // Queue audio to SDL
                    SDL_QueueAudio(sctx->audio_dev, audio_buf, audio_buf_size);
                }
            }
            
            av_packet_unref(fctx->packet);
        }
        
        mutexUnlock(&player->state_mutex);
        
        // Small sleep to prevent CPU overuse
        svcSleepThread(1000000LL); // 1ms
    }
    
    // Clean up thread resources
    threadExit();
} 

void player_play_item(Player* player, PlaylistItem* item) {
    // Existing play logic...
} 