#ifndef NETWORK_H
#define NETWORK_H

#include <switch.h>
#include <stdbool.h>

typedef struct {
    char* data;
    size_t size;
} NetworkBuffer;

// Network management functions
bool network_init(void);
void network_cleanup(void);
bool network_check_connection(void);

// Switch-specific network functions
bool network_has_internet(void);
void network_get_system_proxy(char* proxy_url, size_t max_len);

// Content download functions
NetworkBuffer* network_download(const char* url);
void network_buffer_free(NetworkBuffer* buffer);
void fetch_playlist(const char* url);

// Error handling functions
const char* network_get_last_error(void);

#endif // NETWORK_H 