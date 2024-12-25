cmake_minimum_required(VERSION 3.13)

# Include Switch toolchain file
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/switch.cmake)

project(iptv_player)

# Switch toolchain and libraries paths
set(DEVKITPRO $ENV{DEVKITPRO})
set(LIBNX ${DEVKITPRO}/libnx)
set(PORTLIBS ${DEVKITPRO}/portlibs/switch)

# Include directories
include_directories(
    ${LIBNX}/include
    ${PORTLIBS}/include
    ${PORTLIBS}/include/SDL2
    ${PORTLIBS}/include/ffmpeg
    ${PORTLIBS}/include/SDL2_ttf
)

# Library directories
link_directories(
    ${LIBNX}/lib
    ${PORTLIBS}/lib
)

# Source files
set(SOURCES
    src/main.c
    src/network.c
    src/player.c
    src/playlist.c
    src/ui.c
    src/ui_input.c
    src/ui_draw.c
    src/epg.c
    src/categories.c
    src/animations.c
    src/category_filter.c
    src/drawing.c
    src/parser.c
    src/keyboard.c
    src/search.c
)

# Add Switch-specific sources
if(SWITCH)
    list(APPEND SOURCES src/nxgamepad.c)
endif()

# Add compiler definitions
add_definitions(
    -D__STDC_CONSTANT_MACROS
    -D__SWITCH__
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
if(SWITCH)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        -L${DEVKITPRO}/portlibs/switch/lib
        -L${DEVKITPRO}/libnx/lib
        -specs=${DEVKITPRO}/libnx/switch.specs
        -Wl,--start-group
        
        # ספריית בסיס של הסוויץ'
        -lnx
        
        # ספריות SDL
        -lSDL2
        -lSDL2_ttf
        -lSDL2_image
        
        # ספריות טקסט
        -lfreetype
        -lharfbuzz
        
        # ספריות מולטימדיה
        -lavformat
        -lavcodec
        -lavutil
        -lswscale
        -lswresample
        
        # ספריות תרפיקה
        -lEGL
        -lGLESv2
        -lglapi
        -ldrm_nouveau
        
        # ספריות תמונה
        -lpng
        -ljpeg
        -lwebp
        
        # ספריות דחיסה ורשת
        -lz
        -lbz2
        -lminizip
        -lcurl
        
        # ספריות מערכת בסיסיות
        -lm
        -lstdc++
        -lc
        -lgcc
        -Wl,--end-group
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL2
        SDL2_ttf
    )
endif()

# Application metadata
set(APP_TITLE "IPTV Player")
set(APP_AUTHOR "Komi77")
set(APP_VERSION "1.0.0")

# Generate .nacp file
add_custom_target(${PROJECT_NAME}.nacp
    COMMAND nacptool --create "${APP_TITLE}" "${APP_AUTHOR}" "${APP_VERSION}" ${PROJECT_NAME}.nacp
    DEPENDS ${PROJECT_NAME}
)

# Generate .nro file with icon
add_custom_target(${PROJECT_NAME}.nro ALL
    COMMAND elf2nro $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.nro --icon=${CMAKE_SOURCE_DIR}/resources/icon.jpg --nacp=${PROJECT_NAME}.nacp
    DEPENDS ${PROJECT_NAME} ${PROJECT_NAME}.nacp
)

# Add compilation definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    __SWITCH__
    EGL_NO_X11
    MESA_EGL_NO_X11_HEADERS
    CURL_STATICLIB
    _POSIX_SOURCE
)

# Add compilation flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -D_GNU_SOURCE
    -O2
    -ffunction-sections
    -fdata-sections
    -fPIE
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src
) 