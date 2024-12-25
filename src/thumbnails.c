#include <sys/stat.h>
#include "ui.h"
#include <SDL2/SDL_image.h>
#include <curl/curl.h>

#define THUMBNAIL_CACHE_DIR "cache/thumbnails"
#define THUMBNAIL_MAX_AGE (7 * 24 * 3600)  // 1 week

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

static char* get_cache_path(const char* url) {
    char* filename = strrchr(url, '/');
    if (!filename) filename = (char*)url;
    else filename++;
    
    char* path = malloc(strlen(THUMBNAIL_CACHE_DIR) + strlen(filename) + 2);
    sprintf(path, "%s/%s", THUMBNAIL_CACHE_DIR, filename);
    return path;
}

SDL_Texture* ui_get_thumbnail(UI* ui, const char* url) {
    // Check cache first
    for (size_t i = 0; i < ui->thumbnail_count; i++) {
        if (strcmp(ui->thumbnails[i].url, url) == 0) {
            ui->thumbnails[i].last_access = time(NULL);
            return ui->thumbnails[i].texture;
        }
    }
    
    // Load thumbnail
    ui_load_thumbnail(ui, url);
    return NULL;  // Will be available next frame
}

void ui_load_thumbnail(UI* ui, const char* url) {
    // Create cache directory if needed
    mkdir(THUMBNAIL_CACHE_DIR, 0755);
    
    char* cache_path = get_cache_path(url);
    bool needs_download = true;
    
    // Check if cached file exists and is fresh
    struct stat st;
    if (stat(cache_path, &st) == 0) {
        if (time(NULL) - st.st_mtime < THUMBNAIL_MAX_AGE) {
            needs_download = false;
        }
    }
    
    if (needs_download) {
        CURL* curl = curl_easy_init();
        if (curl) {
            FILE* fp = fopen(cache_path, "wb");
            if (fp) {
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                curl_easy_perform(curl);
                fclose(fp);
            }
            curl_easy_cleanup(curl);
        }
    }
    
    // Load image
    SDL_Surface* surface = IMG_Load(cache_path);
    free(cache_path);
    
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
        SDL_FreeSurface(surface);
        
        if (texture) {
            // Add to cache
            if (ui->thumbnail_count >= MAX_THUMBNAILS) {
                // Remove least recently used thumbnail
                time_t oldest = time(NULL);
                size_t oldest_idx = 0;
                
                for (size_t i = 0; i < ui->thumbnail_count; i++) {
                    if (ui->thumbnails[i].last_access < oldest) {
                        oldest = ui->thumbnails[i].last_access;
                        oldest_idx = i;
                    }
                }
                
                free(ui->thumbnails[oldest_idx].url);
                SDL_DestroyTexture(ui->thumbnails[oldest_idx].texture);
                
                // Move last item to removed position
                if (oldest_idx < ui->thumbnail_count - 1) {
                    ui->thumbnails[oldest_idx] = ui->thumbnails[ui->thumbnail_count - 1];
                }
                ui->thumbnail_count--;
            }
            
            // Add new thumbnail
            ui->thumbnails[ui->thumbnail_count].url = strdup(url);
            ui->thumbnails[ui->thumbnail_count].texture = texture;
            ui->thumbnails[ui->thumbnail_count].last_access = time(NULL);
            ui->thumbnail_count++;
        }
    }
}

void ui_cleanup_thumbnails(UI* ui) {
    for (size_t i = 0; i < ui->thumbnail_count; i++) {
        free(ui->thumbnails[i].url);
        SDL_DestroyTexture(ui->thumbnails[i].texture);
    }
    ui->thumbnail_count = 0;
} 