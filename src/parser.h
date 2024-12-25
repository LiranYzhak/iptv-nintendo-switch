#ifndef PARSER_H
#define PARSER_H

#include "playlist.h"
#include <stdbool.h>

// Parse result structure
typedef struct {
    PlaylistItem* items;
    size_t count;
    char* error;
} ParseResult;

// Parser functions
ParseResult parse_m3u(const char* content);
void parse_result_free(ParseResult* result);

// Helper functions
bool is_url(const char* str);
bool is_extinf_tag(const char* line);

#endif // PARSER_H 