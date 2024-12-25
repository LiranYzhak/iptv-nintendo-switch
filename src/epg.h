#ifndef EPG_H
#define EPG_H

#include <time.h>
#include <stdbool.h>

// EPG Program structure
typedef struct {
    char* title;
    char* description;
    time_t start_time;
    time_t end_time;
    char* channel_id;
} EPGProgram;

// EPG Program List structure
typedef struct {
    EPGProgram* programs;
    size_t program_count;
    size_t capacity;
} EPGProgramList;

// EPG Data structure
typedef struct {
    EPGProgramList* channels;
    size_t channel_count;
    time_t last_update;
} EPGData;

// EPG Functions
EPGData* epg_create(void);
void epg_free(EPGData* epg);
bool epg_load_xmltv(EPGData* epg, const char* filename);
bool epg_save_cache(const EPGData* epg, const char* filename);
bool epg_load_cache(EPGData* epg, const char* filename);

// Program list functions
EPGProgramList* epg_program_list_create(void);
void epg_program_list_free(EPGProgramList* list);
bool epg_program_list_add(EPGProgramList* list, const EPGProgram* program);
EPGProgram* epg_program_list_find(const EPGProgramList* list, time_t time);
void epg_program_list_sort(EPGProgramList* list);

// Program functions
EPGProgram* epg_program_create(void);
void epg_program_free(EPGProgram* program);
EPGProgram* epg_program_copy(const EPGProgram* program);

#endif // EPG_H 