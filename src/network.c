#include "network.h"
#include <curl/curl.h>
#include <switch.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static char last_error[256] = {0};

bool network_has_internet(void) {
    NifmInternetConnectionType connection_type;
    NifmInternetConnectionStatus connection_status;
    u32 wifiStrength;
    Result rc;

    // Initialize nifm
    if (R_FAILED(nifmInitialize(NifmServiceType_User))) {
        return false;
    }

    // Get connection status
    rc = nifmGetInternetConnectionStatus(&connection_type, &wifiStrength, &connection_status);

    // Cleanup
    nifmExit();

    return R_SUCCEEDED(rc) && connection_status == NifmInternetConnectionStatus_Connected;
}

void network_get_system_proxy(char* proxy_url, size_t max_len) {
    proxy_url[0] = '\0';
}

// Add curl configuration for Switch
static CURL* curl_init_switch(void) {
    CURL* curl = curl_easy_init();
    if (curl) {
        // Configure curl for Switch network environment
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    }
    return curl;
}

// פונקציית Callback עבור CURL
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    NetworkBuffer* buffer = (NetworkBuffer*)userp;

    char* ptr = realloc(buffer->data, buffer->size + realsize + 1);
    if (!ptr) return 0;

    buffer->data = ptr;
    memcpy(&(buffer->data[buffer->size]), contents, realsize);
    buffer->size += realsize;
    buffer->data[buffer->size] = 0;

    return realsize;
}

bool network_init(void) {
    // Initialize network services
    Result rc = socketInitializeDefault();
    if (R_FAILED(rc)) {
        strncpy(last_error, "Failed to initialize network services", sizeof(last_error) - 1);
        return false;
    }

    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        socketExit();
        strncpy(last_error, "Failed to initialize CURL", sizeof(last_error) - 1);
        return false;
    }

    if (network_has_internet()) {
        // Network is available, continue with initialization
        return true;
    }

    strncpy(last_error, "No internet connection available", sizeof(last_error) - 1);
    return false;
}

void network_cleanup(void) {
    curl_global_cleanup();
    socketExit();
}

bool network_check_connection(void) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

NetworkBuffer* network_download(const char* url) {
    NetworkBuffer* buffer = (NetworkBuffer*)malloc(sizeof(NetworkBuffer));
    if (!buffer) {
        strncpy(last_error, "Memory allocation failed", sizeof(last_error) - 1);
        return NULL;
    }

    buffer->data = malloc(1);
    buffer->size = 0;

    CURL* curl = curl_init_switch();  // Use our Switch-specific initialization
    if (!curl) {
        free(buffer->data);
        free(buffer);
        strncpy(last_error, "Failed to initialize CURL", sizeof(last_error) - 1);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        strncpy(last_error, curl_easy_strerror(res), sizeof(last_error) - 1);
        free(buffer->data);
        free(buffer);
        curl_easy_cleanup(curl);
        return NULL;
    }

    curl_easy_cleanup(curl);
    return buffer;
}

void network_buffer_free(NetworkBuffer* buffer) {
    if (buffer) {
        free(buffer->data);
        free(buffer);
    }
}

const char* network_get_last_error(void) {
    return last_error;
} 