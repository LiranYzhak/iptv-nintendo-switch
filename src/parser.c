#include "playlist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to parse duration string (HH:MM:SS)
static int parse_duration(const char* duration_str) {
    if (!duration_str) return 0;
    
    int hours = 0, minutes = 0, seconds = 0;
    sscanf(duration_str, "%d:%d:%d", &hours, &minutes, &seconds);
    
    return hours * 3600 + minutes * 60 + seconds;
}

// Helper function to parse attributes from EXTINF line
static void parse_attributes(const char* line, PlaylistItem* item) {
    char* duration_str = strchr(line, ':');
    if (!duration_str) return;
    duration_str++;
    
    item->duration = parse_duration(duration_str);
    
    // Parse optional attributes
    char* attr_start = strchr(duration_str, ' ');
    if (!attr_start) return;
    
    char* attr_str = strdup(attr_start + 1);  // Skip space
    char* token = strtok(attr_str, " ");
    
    while (token) {
        char* value = strchr(token, '=');
        if (value) {
            *value = '\0';  // Split at equals sign
            value++;
            
            // Remove quotes if present
            if (*value == '"') {
                value++;
                char* end = strchr(value, '"');
                if (end) *end = '\0';
            }
            
            if (strcmp(token, "tvg-id") == 0) {
                item->tvg_id = strdup(value);
            }
            else if (strcmp(token, "tvg-name") == 0) {
                item->tvg_name = strdup(value);
            }
            else if (strcmp(token, "tvg-logo") == 0) {
                item->tvg_logo = strdup(value);
            }
            else if (strcmp(token, "group-title") == 0) {
                item->group = strdup(value);
            }
            else if (strcmp(token, "language") == 0) {
                item->language = strdup(value);
            }
        }
        token = strtok(NULL, " ");
    }
    
    free(attr_str);
    
    // Get title (everything after the last comma)
    char* title = strrchr(attr_start, ',');
    if (title) {
        title++;  // Skip comma
        while (*title == ' ') title++;  // Skip spaces
        item->title = strdup(title);
    }
}

bool playlist_load_m3u(Playlist* playlist, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;
    
    char line[1024];
    PlaylistItem current_item = {0};
    bool in_item = false;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (strncmp(line, "#EXTINF:", 8) == 0) {
            if (in_item) {
                // Previous item wasn't completed
                free(current_item.title);
                free(current_item.tvg_id);
                free(current_item.tvg_name);
                free(current_item.tvg_logo);
                free(current_item.group);
                free(current_item.language);
            }
            
            memset(&current_item, 0, sizeof(current_item));
            parse_attributes(line, &current_item);
            in_item = true;
        }
        else if (line[0] != '#' && in_item) {
            current_item.url = strdup(line);
            playlist_add_item(playlist, &current_item);
            
            // Clear current item
            free(current_item.title);
            free(current_item.tvg_id);
            free(current_item.tvg_name);
            free(current_item.tvg_logo);
            free(current_item.group);
            free(current_item.language);
            free(current_item.url);
            
            memset(&current_item, 0, sizeof(current_item));
            in_item = false;
        }
    }
    
    if (in_item) {
        // Clean up last item if incomplete
        free(current_item.title);
        free(current_item.tvg_id);
        free(current_item.tvg_name);
        free(current_item.tvg_logo);
        free(current_item.group);
        free(current_item.language);
    }
    
    fclose(file);
    return true;
} 