#include "epg.h"
#include <stdlib.h>
#include <string.h>

// Forward declaration of static function
static int compare_programs(const void* a, const void* b);

EPGData* epg_create(void) {
    EPGData* epg = malloc(sizeof(EPGData));
    if (!epg) return NULL;
    
    epg->channels = NULL;
    epg->channel_count = 0;
    epg->last_update = 0;
    
    return epg;
}

void epg_free(EPGData* epg) {
    if (!epg) return;
    
    for (size_t i = 0; i < epg->channel_count; i++) {
        epg_program_list_free(&epg->channels[i]);
    }
    free(epg->channels);
    free(epg);
}

EPGProgramList* epg_program_list_create(void) {
    EPGProgramList* list = malloc(sizeof(EPGProgramList));
    if (!list) return NULL;
    
    list->programs = NULL;
    list->program_count = 0;
    list->capacity = 0;
    
    return list;
}

void epg_program_list_free(EPGProgramList* list) {
    if (!list) return;
    
    for (size_t i = 0; i < list->program_count; i++) {
        epg_program_free(&list->programs[i]);
    }
    free(list->programs);
    free(list);
}

bool epg_program_list_add(EPGProgramList* list, const EPGProgram* program) {
    if (!list || !program) return false;
    
    if (list->program_count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        EPGProgram* new_programs = realloc(list->programs, 
                                         new_capacity * sizeof(EPGProgram));
        if (!new_programs) return false;
        
        list->programs = new_programs;
        list->capacity = new_capacity;
    }
    
    list->programs[list->program_count] = *program;
    list->program_count++;
    
    return true;
}

EPGProgram* epg_program_create(void) {
    EPGProgram* program = malloc(sizeof(EPGProgram));
    if (!program) return NULL;
    
    program->title = NULL;
    program->description = NULL;
    program->channel_id = NULL;
    program->start_time = 0;
    program->end_time = 0;
    
    return program;
}

void epg_program_free(EPGProgram* program) {
    if (!program) return;
    
    free(program->title);
    free(program->description);
    free(program->channel_id);
}

EPGProgram* epg_program_copy(const EPGProgram* program) {
    if (!program) return NULL;
    
    EPGProgram* copy = epg_program_create();
    if (!copy) return NULL;
    
    copy->title = program->title ? strdup(program->title) : NULL;
    copy->description = program->description ? strdup(program->description) : NULL;
    copy->channel_id = program->channel_id ? strdup(program->channel_id) : NULL;
    copy->start_time = program->start_time;
    copy->end_time = program->end_time;
    
    return copy;
}

void epg_program_list_sort(EPGProgramList* list) {
    if (!list || list->program_count < 2) return;
    qsort(list->programs, list->program_count, sizeof(EPGProgram), compare_programs);
}

static int compare_programs(const void* a, const void* b) {
    const EPGProgram* p1 = (const EPGProgram*)a;
    const EPGProgram* p2 = (const EPGProgram*)b;
    
    if (p1->start_time < p2->start_time) return -1;
    if (p1->start_time > p2->start_time) return 1;
    return 0;
} 